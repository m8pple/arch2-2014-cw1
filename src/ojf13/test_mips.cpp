//
//  test_mips.cpp
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "mips_test.h"
#include "include/test_mips.h"
#include "include/mips_instr.h"

#include <random>

int main(){
    mips_mem_h mem = mips_mem_create_ram(1<<5, 4);
    mips_cpu_h cpu = mips_cpu_create(mem);
    
    mips_test_begin_suite();

	runTest(registerReset, cpu, mem);
	runTest(memoryIO, cpu, mem);
    runTest(rTypeAnd, cpu, mem);
	
    
    mips_test_end_suite();
	
    mips_cpu_free(cpu);
    mips_mem_free(mem);
	return 0;
}
                       
void runTest(testResult(*test)(mips_cpu_h, mips_mem_h),
             mips_cpu_h cpu, mips_mem_h mem){

    testResult result = test(cpu, mem);
    mips_test_end_test(
                       mips_test_begin_test(result.instr),
                       result.passed,
                       result.desc);
    
}

testResult registerReset(mips_cpu_h cpu, mips_mem_h mem){
    mips_cpu_reset(cpu);
    uint32_t got;
    int correct;
    
    try{
        mips_cpu_get_pc(cpu, &got);
        correct = (got==0) ? 1 : 0;
        
        for(int i=0; i<32 && correct; ++i){
            mips_cpu_get_register(cpu, i, &got);
            correct |= (got==0);
        }
    } catch(mips_error) {
        correct = 0;
    }
    
    return {"<INTERNAL>", "Check all registers zeroed after reset.", correct};
}

testResult memoryIO(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	srand(764835875);
	
	for(int r=0; r<1024; r++){
		uint8_t ibuf[4] = {
			(uint8_t)(rand()%(1<<31)),
			(uint8_t)(rand()%(1<<31)),
			(uint8_t)(rand()%(1<<31)),
			(uint8_t)(rand()%(1<<31))
		};
		mips_mem_write(mem, 0x0, 4, ibuf);
		
		uint8_t obuf[4];
		mips_mem_read(mem, 0x0, 4, obuf);
		
		for(int i=0; i<4; ++i)
			correct &= obuf[i]==ibuf[i] ? 1 : 0;
	}
	
	return {"<INTERNAL>", "Check can read same value back after write.", correct};
}

testResult rTypeAnd(mips_cpu_h cpu, mips_mem_h mem){
	mips_cpu_set_register(cpu, 1, 0x00FF00F0);
	mips_cpu_set_register(cpu, 2, 0x000F0F0F);
	mips_mem_write(mem, 0x0, 4, Instruction(AND, 1, 2, 3, 0).bufferedVal());
	
	mips_cpu_set_pc(cpu, 0);
	mips_cpu_step(cpu);
	
	uint32_t r1,r2,r3;
	mips_cpu_get_register(cpu, 1, &r1);
	mips_cpu_get_register(cpu, 2, &r2);
	mips_cpu_get_register(cpu, 3, &r3);
	int correct = 1 && r1==0x00FF00F0 && r2==0x000F0F0F && r3==0x000F0000;
    return {"AND", "Check registers after AND-ing.", correct};
}
