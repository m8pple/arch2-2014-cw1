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
#include <iostream>

int main(){
    mips_mem_h mem = mips_mem_create_ram(1<<5, 4);
    mips_cpu_h cpu = mips_cpu_create(mem);
	
	srand((unsigned)time(NULL));
    mips_test_begin_suite();

	for(int i=0; i<NUM_TESTS; ++i)
		runTest(tests[i], cpu, mem, 1024);
	
    mips_test_end_suite();
	
    mips_cpu_free(cpu);
    mips_mem_free(mem);
	return 0;
}
                       
void runTest(testResult(*test)(mips_cpu_h, mips_mem_h),
             mips_cpu_h cpu, mips_mem_h mem, unsigned runs){

	testResult result;
 
	for(int i=0; i<runs; ++i){
		result = test(cpu, mem);
		mips_test_end_test(mips_test_begin_test(result.instr),
						   result.passed, result.desc);
	}
	
}

testResult registerReset(mips_cpu_h cpu, mips_mem_h mem){
    uint32_t got;
    int correct, i=0;
    
    try{
		for(int i=0; i<MIPS_NUM_REG; ++i)
			mips_cpu_set_register(cpu, i, rand());
		mips_cpu_set_pc(cpu, rand());
		
		mips_cpu_reset(cpu);

		mips_cpu_get_pc(cpu, &got);
        correct = (got==0) ? 1 : 0;
		
		i=1;
        for(; i<32 && correct; i++){
            mips_cpu_get_register(cpu, i-1, &got);
            correct |= (got==0);
        }
    } catch(mips_error) {
        correct = 0;
    }
	
	if(!correct)
		std::cout << ((i==0)? "PC" : "GP reg") << " not zeroed" << std::endl;
    
    return {"<INTERNAL>", "Check all registers zeroed after reset.", correct};
}

testResult memoryIO(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	
	try{
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
	
	} catch(mips_error) {
		correct = 0;
	}
	return {"<INTERNAL>", "Check can read same value back after write.", correct};
}

testResult RTypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncR verfunc){
	int correct = -1;
	uint32_t r1, r2, r3, exp;
	uint8_t shift;
	
	try{
		uint8_t	shift = rand()&(MASK_SHFT>>POS_SHFT);
		r1 = rand();
		r2 = rand();
		
		mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, 3, shift).bufferedVal());
		
		mips_cpu_set_register(cpu, 1, r1);
		mips_cpu_set_register(cpu, 2, r2);
		
		mips_cpu_set_pc(cpu, 0);
		mips_cpu_step(cpu);
		
		mips_cpu_get_register(cpu, 3, &r3);
		
	} catch(mips_error e) {
		try{
			exp = verfunc(r1, r2, shift);
			correct = 0;		//Simulator threw exception, shouldn't have.
			std::cout << "Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << e << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
		} catch(mips_error v){
			if( v == e )
				correct = 1;	//Correct exception
			else{
				correct = 0;	//Exception, but the wrong one!
				std::cout << "Incorrect result." << std::endl;
				std::cout << "Result was: Exception " << e << std::endl;
				std::cout << "--Expected: Exception " << v << std::endl;
			}
		}
	}
	if(correct == -1){
		try{
			exp = verfunc(r1, r2, shift);
			if( r3 == exp )
				correct = 1;	//No exception, correct result
			else{
				correct = 0;	//Incorrect result.
				std::cout << "Incorrect result." << std::endl;
				std::cout << "Result was: 0x" << r3 << std::endl;
				std::cout << "--Expected: 0x" << exp << std::endl;
			}
		} catch(mips_error v){
			correct = 0;		//No exception, should have
			std::cout << "Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << r3 << std::endl;
			std::cout << "--Expected: Exception " << v << std::endl;
		}
	}
	else;
	
	std::string desc ="Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}

testResult ITypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncI verfunc){
	int correct = 1;
	uint32_t r1, r2, exp;
	uint16_t imm;
	
	try{
		r1 = rand();
		imm= rand();
		
		mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, imm).bufferedVal());
		
		mips_cpu_set_register(cpu, 1, r1);
		
		mips_cpu_set_pc(cpu, 0);
		mips_cpu_step(cpu);
		
		mips_cpu_get_register(cpu, 2, &r2);
		correct = r2 == verfunc(r1, imm);
		
		
	
	} catch(mips_error e) {
		try{
			exp = verfunc(r1, imm);
			correct = 0;		//Simulator threw exception, shouldn't have.
			std::cout << "Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << e << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
		} catch(mips_error v){
			if( v == e )
				correct = 1;	//Correct exception
			else{
				correct = 0;	//Exception, but the wrong one!
				std::cout << "Incorrect result." << std::endl;
				std::cout << "Result was: Exception " << e << std::endl;
				std::cout << "--Expected: Exception " << v << std::endl;
			}
		}
	}
	if(correct == -1){
		try{
			exp = verfunc(r1, imm);
			if( r2 == exp )
				correct = 1;	//No exception, correct result
			else{
				correct = 0;	//Incorrect result.
				std::cout << "Incorrect result." << std::endl;
				std::cout << "Result was: 0x" << r2 << std::endl;
				std::cout << "--Expected: 0x" << exp << std::endl;
			}
		} catch(mips_error v){
			correct = 0;		//No exception, should have
			std::cout << "Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << r2 << std::endl;
			std::cout << "--Expected: Exception " << v << std::endl;
		}
	}
	else;
	
	std::string desc ="Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}

uint32_t ADDverify(uint32_t r1, uint32_t r2, uint8_t){
	int64_t p = (signed)r1+(signed)r2;
	if((~(unsigned)p)&0xFFFFFFFF00000000)
		throw mips_ExceptionArithmeticOverflow;
	else
		return p&0xFFFFFFFF;
}
testResult ADDResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, ADD, (verifyFuncR)ADDverify);
}

uint32_t ADDIverify(uint32_t r1, uint16_t i){
	int64_t p = (signed)r1+(i&0x8000 ? 0xFFFF0000|i : i);
	if((~p)&0xFFFFFFFF00000000)
		throw mips_ExceptionArithmeticOverflow;
	else
		return p&0xFFFFFFFF;
}
testResult ADDIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, ADDI, (verifyFuncI)ADDIverify);
}

uint32_t ADDIUverify(uint32_t r1, uint16_t i){
	return r1+i;
}
testResult ADDIUResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, ADDIU, (verifyFuncI)ADDIUverify);
}

uint32_t ADDUverify(uint32_t r1, uint32_t r2, uint8_t){
	return r1+r2;
}
testResult ADDUResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, ADDU, (verifyFuncR)ADDUverify);
}

uint32_t ANDverify(uint32_t r1, uint32_t r2, uint8_t){
	return r1&r2;
}
testResult ANDResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, AND, (verifyFuncR)ANDverify);
}

testResult ANDInputs(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	mips_mem_write(mem, 0x0, 4, Instruction(AND, 1, 2, 3, 0).bufferedVal());
	
	try{
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
		
		if(!correct){
			std::cout << "Registers changed from: 0x" << ir1 << ", 0x" << ir2 << std::endl;
			std::cout << "--------------------To: 0x" << or1 << ", 0x" << or2 << std::endl;
		}

	} catch(mips_error){
		correct = 0;
	}
	return {"AND", "Check input registers unchanged after AND-ing.", correct};
}

