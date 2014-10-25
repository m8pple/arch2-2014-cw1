//
//  test_mips.cpp
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "mips_test.h"
#include "include/test_mips.h"

#include <random>
#include <string>

int main(){
    mips_mem_h mem = mips_mem_create_ram(1<<5, 4);
    mips_cpu_h cpu = mips_cpu_create(mem);
	
	srand((unsigned)time(NULL));
    mips_test_begin_suite();

	for(int i=0; i<NUM_TESTS; ++i)
		runTest(tests[i], cpu, mem);
	
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
	
	try{
		for(int r=0; r<1024 && correct; ++r){
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
	} catch(mips_error) {
		correct = 0;
	}
	return {"<INTERNAL>", "Check can read same value back after write.", correct};
}

testResult RTypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncR verfunc){
	int correct = 1;
	
	try{
		for(int r=0; r<1024 && correct; ++r){
			uint32_t shift = rand();
			uint32_t r1 = rand();
			uint32_t r2 = rand();
			
			mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, 3, shift).bufferedVal());
			
			mips_cpu_set_register(cpu, 1, r1);
			mips_cpu_set_register(cpu, 2, r2);
			
			mips_cpu_set_pc(cpu, 0);
			mips_cpu_step(cpu);
			
			uint32_t r3;
			mips_cpu_get_register(cpu, 3, &r3);
			correct = r3 == verfunc(r1, r2, shift);
		}
	} catch(mips_error) {
		correct = 0;
	}
	std::string desc ="Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}

testResult ITypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncI verfunc){
	int correct = 1;
	
	try{
		for(int r=0; r<1024 && correct; ++r){
			uint32_t r1 = rand();
			uint32_t imm= rand();
			
			mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, imm).bufferedVal());
			
			mips_cpu_set_register(cpu, 1, r1);
			
			mips_cpu_set_pc(cpu, 0);
			mips_cpu_step(cpu);
			
			uint32_t r2;
			mips_cpu_get_register(cpu, 2, &r2);
			correct = r2 == verfunc(r1, imm);
		}
	} catch(mips_error) {
		correct = 0;
	}
	std::string desc ="Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}

uint32_t ADDverify(uint32_t r1, uint32_t r2, uint32_t shift){
	return r1+(r2<<shift);
}
testResult ADDResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, ADD, (verifyFuncR)ADDverify);
}

uint32_t ADDIverify(uint32_t r1, uint32_t i){
	return r1+i;
}
testResult ADDIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, ADDI, (verifyFuncI)ADDIverify);
}

uint32_t ANDverify(uint32_t r1, uint32_t r2, uint32_t shift){
	return r1&(r2<<shift);
}
testResult ANDResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, AND, (verifyFuncR)ANDverify);
}

testResult ANDInputs(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	mips_mem_write(mem, 0x0, 4, Instruction(AND, 1, 2, 3, 0).bufferedVal());
	
	try{
		for(int r=0; r<1024 && correct; ++r){
			uint32_t ir1 = rand();
			uint32_t ir2 = rand();
			mips_cpu_set_register(cpu, 1, ir1);
			mips_cpu_set_register(cpu, 2, ir2);
			
			mips_cpu_set_pc(cpu, 0);
			mips_cpu_step(cpu);
		
			mips_cpu_set_pc(cpu, 0);
			mips_cpu_step(cpu);
		
			uint32_t or1,or2;
			mips_cpu_get_register(cpu, 1, &or1);
			mips_cpu_get_register(cpu, 2, &or2);
			correct = (or1==ir1) && (or2==ir2);
		}
	} catch(mips_error){
		correct = 0;
	}
	return {"AND", "Check input registers unchanged after AND-ing.", correct};
}

