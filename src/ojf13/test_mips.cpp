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
#include <climits>

#define MEM_SIZE 0x100000

int main(){
    mips_mem_h mem = mips_mem_create_ram(MEM_SIZE, 4);
	mips_cpu_h cpu;
	
	if(mem)
		cpu = mips_cpu_create(mem);
	else{
		std::cout << "Insufficient memory." << std::endl;
		return -1;
	}
	
	srand((unsigned)time(NULL));
	std::cout << std::hex;
	
	mips_cpu_set_debug_level(cpu, 3, stderr);
    mips_test_begin_suite();
	
	runTest(initialPC, cpu, mem, 1);		//
	runTest(constInputs, cpu, mem, 1);		// It's hard to imagine that the correctness
	runTest(registerReset, cpu, mem, 1);	//	of these would be input-dependent without
	runTest(memoryIO, cpu, mem, 1);			//	everything else failing hard, so only do once.
	runTest(noOperation, cpu, mem, 1);		//
	
	runTest(isPrime, cpu, mem, 5);
	
#if MEM_SIZE<UINTMAX	//Otherwise there's nothing to test
	runTest(outOfRange, cpu, mem, 1);
#endif

	for(unsigned i=0; i<NUM_OP_TESTS; ++i)
		runTest(opTests[i], cpu, mem, 1024);
	
    mips_test_end_suite();
	
    mips_cpu_free(cpu);
    mips_mem_free(mem);
	return 0;
}

//Uniformly choose between a few edge cases, and random numbers
uint32_t chooseValue32(void){
	switch( rand()&0x7 ){
		case 1:
			return 0x0;
		case 2:
			return 0xFFFFFFFF;
		case 3:
			return 0x1;
		case 4:
			return 0xFFFFFFFE;
		case 5:
			return INT_MIN+1;
		case 6:
			return INT_MAX;
		case 7:
			return 0x76543210;
		default:
			return rand()&0xFFFFFFFF;
	}
}
uint32_t chooseValue16(void){
	switch( rand()&0x7 ){
		case 1:
			return 0x0;
		case 2:
			return 0xFFFF;
		case 3:
			return 0x1;
		case 4:
			return 0xFFFE;
		case 5:
			return 0x8000;
		case 6:
			return 0x0123;
		case 7:
			return 0x7654;
		default:
			return rand()&0xFFFF;
	}
}
uint32_t chooseValue8(void){
	switch( rand()&0x7 ){
		case 1:
			return 0x0;
		case 2:
			return 0xFF;
		case 3:
			return 0x1;
		case 4:
			return 0xFE;
		case 5:
			return 0x80;
		case 6:
			return 0x01;
		case 7:
			return 0x76;
		default:
			return rand()&0xFF;
	}
}

void runTest(testResult(*test)(mips_cpu_h, mips_mem_h),
             mips_cpu_h cpu, mips_mem_h mem, unsigned runs){

	testResult result;
 
	for(unsigned i=0; i<runs; ++i){
		result = test(cpu, mem);
		mips_test_end_test(mips_test_begin_test(result.instr),
						   result.passed, result.desc);
	}
	
}

testResult initReturn(int correct=0){
	return {"<INTERNAL>", "Check PC is initially zero.", correct};
}
testResult initialPC(mips_cpu_h cpu, mips_mem_h mem){
	uint32_t got1, got2;
	mips_error e = mips_Success;
	uint8_t zero[12] = {0};
	
	try{
		e = mips_mem_write(mem, 0x0, 12, zero);
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return initReturn();
		}
		
		e = mips_cpu_get_pc(cpu, &got1);
		
		if(e){
			std::cout << "Error " << e << " getting PC." << std::endl;
			return initReturn();
		}
		
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error: Unexpected exception " << e << " performing NOP." << std::endl;
			return initReturn();
		}
		
		e = mips_cpu_get_pc(cpu, &got2);
		
		if(e){
			std::cout << "Error " << e << " accessing registers." << std::endl;
			return initReturn();
		}
	}
	catch(mips_error e){
		std::cout << "Error: Uncaught exception " << e << std::endl;
		return initReturn();
	}
	
	return initReturn(got1==0 && got2==4);
}
testResult registerReturn(int correct=0){
	return {"<INTERNAL>", "Check all registers zeroed after reset.", correct};
}
testResult registerReset(mips_cpu_h cpu, mips_mem_h){
    uint32_t got;
    int correct, i=0;
	mips_error e = mips_Success;
    
    try{
		for(unsigned i=0; i<MIPS_NUM_REG && !e; ++i)
			e = mips_cpu_set_register(cpu, i, chooseValue32());
		
		if(e){
			std::cout << "Error " << e << " accessing registers." << std::endl;
			return registerReturn();
		}
		
		e = mips_cpu_set_pc(cpu, chooseValue32());
		
		if(e){
			std::cout << "Error " << e << " accessing registers." << std::endl;
			return registerReturn();
		}
		
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " accessing registers." << std::endl;
			return registerReturn();
		}

		e = mips_cpu_get_pc(cpu, &got);
		
		if(e){
			std::cout << "Error " << e << " accessing registers." << std::endl;
			return registerReturn();
		}
		
        correct = (got==0) ? 1 : 0;
		
        for(i=1; i<32 && correct; i++){
            e = mips_cpu_get_register(cpu, i-1, &got);
            correct |= (got==0) && !e;
        }
		
		if(e){
			std::cout << "Error " << e << " accessing registers." << std::endl;
			return registerReturn();
		}

	} catch(...) {
        correct = 0;
		std::cout << "Error: Uncaught exception accessing registers." << std::endl;
    }

	if(!correct)
		std::cout << ((i==0)? "PC" : "GP reg") << " not zeroed" << std::endl;
	
	return registerReturn(correct);
}

testResult memoryReturn(int correct=0){
	return {"<INTERNAL>", "Check can read same value back after write.", correct};
}
testResult memoryIO(mips_cpu_h, mips_mem_h mem){
	int correct = 1;
	mips_error e = mips_Success;
	
	try{
		uint8_t ibuf[4] = {
			(uint8_t)chooseValue8(),
			(uint8_t)chooseValue8(),
			(uint8_t)chooseValue8(),
			(uint8_t)chooseValue8()
		};
		e = mips_mem_write(mem, 0x0, 4, ibuf);
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return memoryReturn();
		}
		
		uint8_t obuf[4];
		e = mips_mem_read(mem, 0x0, 4, obuf);
		
		if(e){
			std::cout << "Error " << e << " reading from memory." << std::endl;
			return memoryReturn();
		}
		
		for(unsigned i=0; i<4; ++i)
			correct &= obuf[i]==ibuf[i] ? 1 : 0;
	
	} catch(mips_error e) {
		correct = 0;
		std::cout << "Error: uncaught exception reading/writing memory." << std::endl;
	}
	
	return memoryReturn(correct);
}

testResult oorReturn(int correct=0){
	return {"<INTERNAL>", "Check exception on stepping PC out of range.", correct};
}
testResult outOfRange(mips_cpu_h cpu, mips_mem_h){
	mips_error e = mips_Success, gotE;
	
	try{
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU" << std::endl;
			return oorReturn();
		}
		
		e = mips_cpu_set_pc(cpu, MEM_SIZE-(MEM_SIZE%4)+4);
		
		if(e){
			std::cout << "Error " << e << " setting PC" << std::endl;
			return oorReturn();
		}
		
		gotE = mips_cpu_step(cpu);
		
		if( !gotE ){
			std::cout << "Error: Stepping PC out of range did not trap." << std::endl;
			return oorReturn();
		}
		else if( gotE  != mips_ExceptionAccessViolation ){
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << gotE << std::endl;
			std::cout << "--Expected: Exception " << mips_ExceptionAccessViolation << std::endl;
			return oorReturn();
		}
		else
			return oorReturn(1);
	} catch(...){
		std::cout << "Error: uncaught exception performing NOP." << std::endl;
		return oorReturn();
	}
}

testResult constReturn(int correct=0){
	return {"<INTERNAL>", "Check input registers unchanged after AND-ing.", correct};
}
testResult constInputs(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	mips_error e = mips_Success;
	
	try{
		e = mips_mem_write(mem, 0x0, 4, Instruction(AND, 1, 2, 3, 0).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return constReturn();
		}
		
		uint32_t ir1 = chooseValue32();
		uint32_t ir2 = chooseValue32();
		
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU" << std::endl;
			return constReturn();
		}
		
		e = mips_cpu_set_register(cpu, 1, ir1);
		
		if(e){
			std::cout << "Error " << e << " setting register" << std::endl;
			return constReturn();
		}
		
		e = mips_cpu_set_register(cpu, 2, ir2);
		
		if(e){
			std::cout << "Error " << e << " setting register" << std::endl;
			return constReturn();
		}
		
		e = mips_cpu_step(cpu);
		
		
		if(e){
			std::cout << "Error " << e << " stepping CPU" << std::endl;
			return constReturn();
		}
		
		uint32_t or1,or2;
		e = mips_cpu_get_register(cpu, 1, &or1);
		
		if(e){
			std::cout << "Error " << e << " getting register" << std::endl;
			return constReturn();
		}
		
		e = mips_cpu_get_register(cpu, 2, &or2);
		
		if(e){
			std::cout << "Error " << e << " getting register" << std::endl;
			return constReturn();
		}
		
		correct = (or1==ir1) && (or2==ir2);
		
		if(!correct){
			std::cout << "Registers changed from: 0x" << ir1 << ", 0x" << ir2 << std::endl;
			std::cout << "--------------------To: 0x" << or1 << ", 0x" << or2 << std::endl;
		}
		
	} catch(...){
		correct = 0;
		std::cout << "Error: uncaught exception performing AND." << std::endl;
	}
	
	return constReturn(correct);
}

testResult nopReturn(int correct=0){
	return {"<INTERNAL>", "Check only PC changes after NOP.", correct};
}
testResult noOperation(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	mips_error e = mips_Success;
	
	try{
		e = mips_mem_write(mem, 0x0, 4, Instruction(SLL, 0, 0, 0, 0).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return nopReturn();
		}
		
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU." << std::endl;
			return nopReturn();
		}
		
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error " << e << " stepping CPU." << std::endl;
			return nopReturn();
		}
		
		uint32_t r[MIPS_NUM_REG];
		unsigned i;
		for(i=0; (i<MIPS_NUM_REG) && correct; ++i){
			e = mips_cpu_get_register(cpu, i, &r[i]);
			correct &= !e ? ~r[i] : 0;
		}
		
		if(e){
			std::cout << "Error " << e << " getting register." << std::endl;
			return nopReturn();
		}
		
		if(!correct)
			std::cout << "GP reg " << i << " got set during NOP." << std::endl;
		
	} catch(...){
		correct = 0;
		std::cout << "Error: uncaught exception performing NOP (SLL 0)." << std::endl;
	}
	
	return nopReturn(correct);
}

bool isPrimeC(unsigned n){
	for(unsigned i=2; i<n; ++i)
		if( n%i == 0 )
			return false;
	return true;
}
testResult isPrime(mips_cpu_h cpu, mips_mem_h mem){
	uint32_t n = rand()%1000; //let's.. keep it sensible(ish)
#define PRIME_POS  0x30
#define NPRIME_POS 0x40
	
	mips_cpu_reset(cpu);
	
	mips_cpu_set_register(cpu, 1, n);
	mips_mem_write(mem, 0x0, 4, Instruction(ADDI, 1, 2, -1).bufferedVal());
	mips_mem_write(mem, 0x4, 4, Instruction(SLTI, 2, 3, 2).bufferedVal());
	mips_mem_write(mem, 0x8, 4, Instruction(BGTZ, 3, (PRIME_POS-0x8-4)>>2).bufferedVal());
	mips_mem_write(mem, 0xc, 4, Instruction(DIV, 1, 2, 0, 0).bufferedVal());
	mips_mem_write(mem, 0x10, 4, Instruction(MFHI, 0, 0, 3, 0).bufferedVal());
	mips_mem_write(mem, 0x14, 4, Instruction(BEQ, 3, 0, (NPRIME_POS-0x14-4)>>2).bufferedVal());
	mips_mem_write(mem, 0x18, 4, Instruction(ADDI, 2, 2, -1).bufferedVal());
	mips_mem_write(mem, 0x1c, 4, Instruction(J, 0x4>>2).bufferedVal());
	//NOP
	mips_mem_write(mem, 0x20, 4, Instruction(SLL, 0, 0, 0, 0).bufferedVal());
	
	uint32_t pc=0;
	while( pc <= 0x20 ){
		mips_cpu_step(cpu);
		mips_cpu_get_pc(cpu, &pc);
	}
	
	bool correct;
	
	if( pc == PRIME_POS )
		correct = isPrimeC(n)==true;
	else if( pc == NPRIME_POS )
		correct = isPrimeC(n)==false;
	else
		correct = false;
	
	return {"<INTERNAL>", "Test primality.", correct};
}

testResult result(mips_asm mnemonic, int correct=0){
	std::string desc = "Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}
testResult RTypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncR verfunc){
	
	mips_error e = mips_Success, gotE;
	uint32_t r1 = chooseValue32(), r2 = chooseValue32(), r3, exp;
	
	try{
		e = mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, 3, 0).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_set_register(cpu, 1, r1);
		
		if(e){
			std::cout << "Error " << e << " setting register." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_set_register(cpu, 2, r2);
		
		if(e){
			std::cout << "Error " << e << " setting register." << std::endl;
			return result(mnemonic);
		}

		gotE = mips_cpu_step(cpu);
		
		e = mips_cpu_get_register(cpu, 3, &r3);
		
		if(e){
			std::cout << "Error " << e << " getting register." << std::endl;
			return result(mnemonic);
		}
		
	} catch(...) {
		std::cout << "Error: uncaught exception performing " << mipsInstruction[mnemonic].mnem << std::endl;
		return result(mnemonic);
	}
	
	try{
		exp = verfunc(r1, r2, 0);
		if( gotE ){
			//Simulator threw exception, shouldn't have.
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << gotE << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
			return result(mnemonic);
		}
		else if( exp == r3 ){
			//Correct result
			return result(mnemonic, 1);
		}
		else{
			//Incorrect result
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << r3 << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
			return result(mnemonic);
		}
	} catch(mips_error expE){
		if( expE == gotE ){
			//Correct exception
			return result(mnemonic, 1);
		}
		else if( gotE ){
			//Exception, but the wrong one!
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << gotE << std::endl;
			std::cout << "--Expected: Exception " << expE << std::endl;
			return result(mnemonic);
		}
		else{
			//No exception, should have
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << r3 << std::endl;
			std::cout << "--Expected: Exception " << expE << std::endl;
			return result(mnemonic);
		}
	}
}

testResult ITypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncI verfunc){
	
	mips_error e = mips_Success, gotE;
	uint32_t r1, r2, exp;
	uint16_t imm;
	
	try{
		r1 = mnemonic==LUI ? 0 : chooseValue32();
		imm= chooseValue16();
		
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, imm).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_set_register(cpu, 1, r1);
		
		if(e){
			std::cout << "Error " << e << " setting register." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_set_pc(cpu, 0);
		
		if(e){
			std::cout << "Error " << e << " setting PC." << std::endl;
			return result(mnemonic);
		}
		
		gotE = mips_cpu_step(cpu);
		
		e = mips_cpu_get_register(cpu, 2, &r2);
		
		if(e){
			std::cout << "Error " << e << " getting register." << std::endl;
			return result(mnemonic);
		}

	} catch(...) {
		std::cout << "Error: uncaught exception performing " << mipsInstruction[mnemonic].mnem << std::endl;
		return result(mnemonic);
	}
	
	try{
		exp = verfunc(r1, imm);
		if( gotE ){
			//Simulator threw exception, shouldn't have.
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << gotE << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
			return result(mnemonic);
		}
		else if( exp == r2 ){
			//Correct result
			return result(mnemonic, 1);
		}
		else{
			//Incorrect result
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << r2 << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
			return result(mnemonic);
		}
	} catch(mips_error expE){
		if( expE == gotE ){
			//Correct exception
			return result(mnemonic, 1);
		}
		else if( gotE ){
			//Exception, but the wrong one!
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << gotE << std::endl;
			std::cout << "--Expected: Exception " << expE << std::endl;
			return result(mnemonic);
		}
		else{
			//No exception, should have
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << r2 << std::endl;
			std::cout << "--Expected: Exception " << expE << std::endl;
			return result(mnemonic);
		}
	}
}

/* Includes JR, which is RType, but easier to test with J,JAL */
testResult JTypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncJ verfunc){
	
	int correct;
	mips_error e = mips_Success, gotE;
	uint32_t pcInit= (rand()%MEM_SIZE)&0xFFFFFFFC;
	uint32_t index = rand()&0x03FFFFFC;	//26 bits, aligned

	try{		
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU." << std::endl;
			return result(mnemonic);
		}

		if(mnemonic == JR){
			e = mips_cpu_set_register(cpu, 1, index);
			
			if(e){
				std::cout << "Error " << e << " setting register." << std::endl;
				return result(mnemonic);
			}
			
			e = mips_mem_write(mem, pcInit, 4, Instruction(mnemonic, 1, 0,0,0).bufferedVal());
			
			if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}
		}
		else{
			e = mips_mem_write(mem, pcInit, 4, Instruction(mnemonic, index).bufferedVal());
			
			if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}
		}
		
		//Make sure not a J/B instruction in the delay slot (UB)
		uint8_t tmp[4]={0};
		e = mips_mem_write(mem, pcInit+4, 4, tmp);
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}

		e = mips_cpu_set_pc(cpu, pcInit);
		
		if(e){
			std::cout << "Error " << e << " setting PC." << std::endl;
			return result(mnemonic);
		}
		
		uint32_t after1, after2;
		gotE = mips_cpu_step(cpu);
		
		e = mips_cpu_get_pc(cpu, &after1);
		
		if(e){
			std::cout << "Error " << e << " getting PC." << std::endl;
			return result(mnemonic);
		}
		
		correct = (after1 == pcInit+4);
		
		if( !correct ){
			std::cout << "Error: Incorrect PC." << std::endl;
			std::cout << "---PC at: 0x" << after1 << " during branch delay slot;" << std::endl;
			std::cout << "Expected: 0x" << pcInit+4 << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error: Unexpected exception " << e << " performing branch." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_get_pc(cpu, &after2);
		
		if(e){
			std::cout << "Error " << e << " getting PC." << std::endl;
			return result(mnemonic);
		}
		
		uint32_t exp = verfunc(pcInit, index);
		correct = (after2 == exp);
		
		if( !correct ){
			std::cout << "Error: Incorrect jump." << std::endl;
			std::cout << "Result was: 0x" << after2 << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
		}
		else if( mnemonic == JAL ){
			uint32_t r31;
			e = mips_cpu_get_register(cpu, 31, &r31);
			
			if(e){
				std::cout << "Error " << e << " getting register." << std::endl;
				return result(mnemonic);
			}
			
			correct = (r31 == pcInit+8);
			
			if( !correct ){
				std::cout << "Error: Incorrect link." << std::endl;
				std::cout << "Link register was: 0x" << r31 << std::endl;
				std::cout << "---------Expected: 0x" << pcInit+8 << std::endl;
			}
		}
	}
	catch(...){
		std::cout << "Error: Uncaught exception performing branch: " << e << std::endl;
		return result(mnemonic);
	}
	return result(mnemonic, correct);
}

testResult branchResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncB verfunc){
	
	mips_error e = mips_Success;
	int correct = -1;
	
	try{
		int16_t trgtOffset = chooseValue16();
		uint32_t r1 = chooseValue32(), r2 = rand()%2 ? r1 : chooseValue32();
		
		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU." << std::endl;
			return result(mnemonic);
		}
		
		/* Write instructions in appropriate format */
		if(mnemonic == BEQ || mnemonic == BNE){
			e = mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, trgtOffset).bufferedVal());
			
			if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}
			
			//Make sure not a J/B instruction in the delay slot (UB)
			uint8_t tmp[4]={0};
			e = mips_mem_write(mem, 0x4, 4, tmp);
			
			if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}
		}
		else{
			e = mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, trgtOffset).bufferedVal());
			
			if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}
			
			//Make sure not a J/B instruction in the delay slot (UB)
			uint8_t tmp[4]={0};
			e = mips_mem_write(mem, 0x4, 4, tmp);
			
			if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}
		}
		e = mips_cpu_set_register(cpu, 1, r1);
		
		if(e){
			std::cout << "Error " << e << " setting register." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_set_register(cpu, 2, r2);
		
		if(e){
			std::cout << "Error " << e << " setting register." << std::endl;
			return result(mnemonic);
		}
		
		uint32_t after1, after2;
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error: Unexpected exception " << e << " performing branch." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_get_pc(cpu, &after1);
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}
		
		correct = (after1 == 0x4);
		
		if( !correct ){
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "---PC at: 0x" << after1 << " during branch delay slot;" << std::endl;
			std::cout << "Expected: 0x" << 4 << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error: Unexpected exception " << e << " performing branch." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_get_pc(cpu, &after2);
		
		if(e){
			std::cout << "Error " << e << " getting PC." << std::endl;
			return result(mnemonic);
		}
		
		uint32_t exp = verfunc(r1, r2, trgtOffset);
		correct = (after1 == 0x4) && (after2 == exp);
		
		if( !correct ){
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Branched to: 0x" << after2 << std::endl;
			std::cout << "---Expected: 0x" << exp << std::endl;
		}
		/* Test linking */
		else if( (mnemonic == BGEZAL || mnemonic == BLTZAL) && exp != 0x8 ){
			uint32_t r31;
			e = mips_cpu_get_register(cpu, 31, &r31);
			
			if(e){
				std::cout << "Error " << e << " getting register." << std::endl;
				return result(mnemonic);
			}
			
			correct = (r31 == 0x8);
			
			if( !correct ){
				std::cout << "Error: Incorrect result." << std::endl;
				std::cout << "Link register was: 0x" << r31 << std::endl;
				std::cout << "---------Expected: 0x" << 0x8 << std::endl;
			}
		}
	}
	catch(mips_error e){
		std::cout << "Error: Uncaught exception " << e << " performing branch." << std::endl;
		return result(mnemonic);
	}
	return result(mnemonic, correct);
}

testResult hiloResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncHL verfunc){
	
	mips_error e = mips_Success;
	int correct;
	hilo exp;
	try{
		uint32_t r1 = chooseValue32(), r2 = chooseValue32();

		e = mips_cpu_reset(cpu);
		
		if(e){
			std::cout << "Error " << e << " resetting CPU." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, 0, 0).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_mem_write(mem, 0x4, 4, Instruction(MFLO, 0, 0, 3, 0).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_mem_write(mem, 0x8, 4, Instruction(MFHI, 0, 0, 4, 0).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_set_register(cpu, 1, r1);
		
		if(e){
			std::cout << "Error " << e << " setting register." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_set_register(cpu, 2, r2);
		
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error: Unexpected exception " << e << " executing instruction." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error: Unexpected exception " << e << " executing instruction." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_step(cpu);
		
		if(e){
			std::cout << "Error: Unexpected exception " << e << " executing instruction." << std::endl;
			return result(mnemonic);
		}
		
		hilo got;
		e = mips_cpu_get_register(cpu, 3, &(got.lo));
		
		if(e){
			std::cout << "Error " << e << " getting register." << std::endl;
			return result(mnemonic);
		}
		
		e = mips_cpu_get_register(cpu, 4, &(got.hi));
		
		if(e){
			std::cout << "Error " << e << " getting register." << std::endl;
			return result(mnemonic);
		}
		
		try{
			exp = verfunc(r1, r2);
			correct = (got.lo == exp.lo) && (got.hi == exp.hi);
		}
		catch(mips_error){
			//Result is undefined, so whatever happened was 'correct'
			correct = 1;
		}

		if(!correct){
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: [0x" << got.hi << ", 0x" << got.lo << "]" << std::endl;
			std::cout << "--Expected: [0x" << exp.hi << ", 0x" << exp.lo << "]" << std::endl;
		}
		
	} catch(mips_error e){
		correct = 0;
		std::cout << "Error: Uncaught exception: " << e << std::endl;
	}
	return result(mnemonic, correct);
}

testResult loadstoreResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncLS verfunc){
	
	mips_error e = mips_Success, gotE;
	
	uint32_t rtInit = 0x00010203;
	uint8_t idata[4] = {
		(uint8_t)chooseValue8(),
		(uint8_t)chooseValue8(),
		(uint8_t)chooseValue8(),
		(uint8_t)chooseValue8()
	};
	uint8_t zero[4] = {0};
	
	int16_t offset	= rand()<<2;	//align
	uint32_t addr	= 0x100;		//avoid program memory
	uint8_t align = 0;
	
	uint32_t odata;
	
	uint32_t exp;
	
	try{
		
		if(   mnemonic == LB || mnemonic == LBU || mnemonic == LWL || mnemonic == LWR
		   || mnemonic == SH || mnemonic == SB ){
			
			align+= rand()%4;
			addr += align;				//mix up the alignment
		}
		else if( mnemonic == LW || mnemonic == SW ){
			
			align+= rand()%2 ? 1 : 0;	//50% chance of being misaligned (test exception)
			addr += align;
		}
		
		e = mips_cpu_reset(cpu);

		if(e){
			std::cout << "Error " << e << " resetting CPU." << std::endl;
			return result(mnemonic);
		}

		e = mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, offset).bufferedVal());
		
		if(e){
			std::cout << "Error " << e << " writing to memory." << std::endl;
			return result(mnemonic);
		}

		e = mips_cpu_set_register(cpu, 1, addr);

		if(e){
			std::cout << "Error " << e << " setting register." << std::endl;
			return result(mnemonic);
		}

		
		if(mnemonic == LB || mnemonic == LBU ||
		   mnemonic == LW || mnemonic == LWL || mnemonic == LWR){
			//Initial value to test LWL/LWR leave half, and others clear
			e = mips_cpu_set_register(cpu, 2, rtInit);

			if(e){
				std::cout << "Error " << e << " setting register." << std::endl;
				return result(mnemonic);
			}
			
			//Write a word, even if only testing a byte/half
			e = mips_mem_write(mem, addr+offset-align, 4, idata);

			//If there was an error writing to here, we're testing we get the error on load
			/*if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}*/

			//Zero around it
			e = mips_mem_write(mem, addr+offset-align-4, 4, zero);
			
			/*if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}*/

			e = mips_mem_write(mem, addr+offset-align+4, 4, zero);

			/*if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}*/

			gotE = mips_cpu_step(cpu);

			e = mips_cpu_get_register(cpu, 2, &odata);

			if(e){
				std::cout << "Error " << e << " getting register." << std::endl;
				return result(mnemonic);
			}
		}
		else{
			//Zero the address we want to test writing to
			e = mips_mem_write(mem, addr+offset-align, 4, zero);

			/*if(e){
				std::cout << "Error " << e << " writing to memory." << std::endl;
				return result(mnemonic);
			}*/

			e = mips_cpu_set_register(cpu, 2, (idata[0]<<24|idata[1]<<16|idata[2]<<8|idata[3]));

			if(e){
				std::cout << "Error " << e << " setting register." << std::endl;
				return result(mnemonic);
			}

			gotE = mips_cpu_step(cpu);
			
			uint8_t t[4];
			e = mips_mem_read(mem, addr+offset-align, 4, t);

			/*if(e){
				std::cout << "Error " << e << " accessing memory." << std::endl;
				return result(mnemonic);
			}*/

			odata = t[0]<<24|t[1]<<16|t[2]<<8|t[3];
		}
	} catch(...) {
		std::cout << "Error: Uncaught exception performing " << mipsInstruction[mnemonic].mnem << std::endl;
		return result(mnemonic);
	}

	try{
		exp = verfunc( idata[0]<<24|idata[1]<<16|idata[2]<<8|idata[3], addr+offset, rtInit );
		if( gotE ){
			//Simulator threw exception, shouldn't have.
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << gotE << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
			return result(mnemonic);
		}
		else if( exp == odata ){
			//Correct result
			return result(mnemonic, 1);
		}
		else{
			//Incorrect result
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << odata << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
			return result(mnemonic);
		}
	} catch(mips_error expE){
		if( expE == gotE ){
			//Correct exception
			return result(mnemonic, 1);
		}
		else if( gotE ){
			//Exception, but the wrong one!
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: Exception " << gotE << std::endl;
			std::cout << "--Expected: Exception " << expE << std::endl;
			return result(mnemonic);
		}
		else{
			//No exception, should have
			std::cout << "Error: Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << odata << std::endl;
			std::cout << "--Expected: Exception " << expE << std::endl;
			return result(mnemonic);
		}
	}
}

uint32_t ADDverify(uint32_t r1, uint32_t r2, uint8_t){
	if(((signed)r2>0 && (signed)r1>INT32_MAX-(signed)r2) ||
	   ((signed)r2<0 && (signed)r1<INT32_MIN-(signed)r2))
		throw mips_ExceptionArithmeticOverflow;
	else
		return (signed)r1+(signed)r2;
}
testResult ADDResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, ADD, (verifyFuncR)ADDverify);
}

uint32_t ADDIverify(uint32_t r1, uint16_t i){
	int32_t imm = i&0x8000 ? (MASK_16b<<16)|i : i&MASK_16b;
	if((imm>0 && (signed)r1>INT32_MAX-imm) ||
	   (imm<0 && (signed)r1<INT32_MIN-imm))
		throw mips_ExceptionArithmeticOverflow;
	else{
		int32_t p = (signed)r1+imm;
		return p;
	}
}
testResult ADDIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, ADDI, (verifyFuncI)ADDIverify);
}

uint32_t ADDIUverify(uint32_t r1, uint16_t i){
	return i&0x8000 ? ((MASK_16b<<16)|i)+r1 : (i&MASK_16b)+r1;
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

uint32_t ANDIverify(uint32_t r1, uint16_t i){
	return r1&(0x00000000|i);
}
testResult ANDIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, ANDI, (verifyFuncI)ANDIverify);
}

//Branch tests need not check for access violation if address out of range
//	since the PC is checked, but that address in memory never loaded.
//outOfRange test checks this behaviour is properly handled.
uint32_t BEQverify(uint32_t r1, uint32_t r2, uint16_t offset){
	return ((signed)r1==(signed)r2) ? 4+(offset<<2): 8;
}
testResult BEQResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BEQ, (verifyFuncB)BEQverify);
}

uint32_t BGEZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1>=0) ? 4+(offset<<2): 8;
}
testResult BGEZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BGEZ, (verifyFuncB)BGEZverify);
}

uint32_t BGEZALverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1>=0) ? 4+(offset<<2): 8;
}
testResult BGEZALResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BGEZAL, (verifyFuncB)BGEZALverify);
}

uint32_t BGTZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1>0) ? 4+(offset<<2): 8;
}
testResult BGTZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BGTZ, (verifyFuncB)BGTZverify);
}

uint32_t BLEZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1<=0) ? 4+(offset<<2): 8;
}
testResult BLEZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BLEZ, (verifyFuncB)BLEZverify);
}

uint32_t BLTZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1<0) ? 4+(offset<<2): 8;
}
testResult BLTZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BLTZ, (verifyFuncB)BLTZverify);
}

uint32_t BLTZALverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1<0) ? 4+(offset<<2): 8;
}
testResult BLTZALResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BLTZAL, (verifyFuncB)BLTZALverify);
}

uint32_t BNEverify(uint32_t r1, uint32_t r2, uint16_t offset){
	return ((signed)r1!=(signed)r2) ? 4+(offset<<2): 8;
}
testResult BNEResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BNE, (verifyFuncB)BNEverify);
}

/* (DIV|MULT)U? tests also implicitly test MFHI, MFLO */
hilo DIVverify(uint32_t r1, uint32_t r2){
	if(r2 == 0)
		//Result is undefined
		throw mips_InternalError;
	else
		return {
			(uint32_t)((signed)r1/(signed)r2),
			(uint32_t)((signed)r1%(signed)r2)
		};
}
testResult DIVResult(mips_cpu_h cpu, mips_mem_h mem){
	return hiloResult(cpu, mem, DIV, DIVverify);
}

hilo DIVUverify(uint32_t r1, uint32_t r2){
	if(r2 == 0)
		throw mips_InternalError;
	else
		return {
			r1/r2,
			r1%r2
		};
}
testResult DIVUResult(mips_cpu_h cpu, mips_mem_h mem){
	return hiloResult(cpu, mem, DIVU, DIVUverify);
}

//J,JR,JAL don't check for an access violation exception
//	since the test only checks PC, so an out of range address
//	is never (attempted to be) loaded.
uint32_t Jverify(uint32_t pcPrior, uint32_t idx){
	return (pcPrior&0xF0000000)|(idx<<2);
}
testResult JResult(mips_cpu_h cpu, mips_mem_h mem){
	return JTypeResult(cpu, mem, J, Jverify);
}

uint32_t JALverify(uint32_t pcPrior, uint32_t idx){
	return (pcPrior&0xF0000000)|(idx<<2);
}
testResult JALResult(mips_cpu_h cpu, mips_mem_h mem){
	return JTypeResult(cpu, mem, JAL, JALverify);
}

uint32_t JRverify(uint32_t, uint32_t r1){
	return r1;
}
testResult JRResult(mips_cpu_h cpu, mips_mem_h mem){
	return JTypeResult(cpu, mem, JR, JRverify);
}


uint32_t LBverify(uint32_t data, uint32_t effaddr, uint32_t){
	uint8_t align = effaddr%4;
	uint32_t ret = (data>>(24-align*8))&0x000000FF;

	if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	return ret&0x00000080 ? ret|0xFFFFFF00 : ret;
}
testResult LBResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LB, LBverify);
}

uint32_t LBUverify(uint32_t data, uint32_t effaddr, uint32_t){
	uint8_t align = effaddr%4;
	
	if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	return (data>>(24-align*8))&0x000000FF;
}
testResult LBUResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LBU, LBUverify);
}

uint32_t LUIverify(uint32_t, uint16_t imm){
	return imm<<16;
}
testResult LUIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, LUI, (verifyFuncI)LUIverify);
}

uint32_t LWverify(uint32_t data, uint32_t effaddr, uint32_t){
	uint8_t align = effaddr%4;
	
	if( align )
		throw mips_ExceptionInvalidAlignment;
	else if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	else
		return data;
}
testResult LWResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LW, LWverify);
}

uint32_t LWLverify(uint32_t data, uint32_t effaddr, uint32_t rt){
	uint8_t align = effaddr%4;
	
	if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	return (data<<(align*8) & (MASK_16b<<16)) | (rt & MASK_16b);
}
testResult LWLResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LWL, LWLverify);
}

uint32_t LWRverify(uint32_t data, uint32_t effaddr, uint32_t rt){
	uint8_t align = effaddr%4;
	
	if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	return (data>>(24-align*8) & MASK_16b) | (rt & MASK_16b<<16);
}
testResult LWRResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LWR, LWRverify);
}

hilo MULTverify(uint32_t r1, uint32_t r2){
	int64_t p = (signed)r1*(signed)r2;
	return {
		(uint32_t)((p&0xFFFFFFFF00000000)>>32),
		(uint32_t)((p&0x00000000FFFFFFFF))
	};
}
testResult MULTResult(mips_cpu_h cpu, mips_mem_h mem){
	return hiloResult(cpu, mem, MULT, MULTverify);
}

hilo MULTUverify(uint32_t r1, uint32_t r2){
	uint64_t p = r1*r2;
	return {
		(uint32_t)((p&0xFFFFFFFF00000000)>>32),
		(uint32_t)((p&0x00000000FFFFFFFF))
	};
}
testResult MULTUResult(mips_cpu_h cpu, mips_mem_h mem){
	return hiloResult(cpu, mem, MULTU, MULTUverify);
}

uint32_t ORverify(uint32_t r1, uint32_t r2, uint8_t){
	return r1|r2;
}
testResult ORResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, OR, (verifyFuncR)ORverify);
}

uint32_t ORIverify(uint32_t r1, uint16_t i){
	return r1|(0x00000000|i);
}
testResult ORIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, ORI, (verifyFuncI)ORIverify);
}

uint32_t SBverify(uint32_t data, uint32_t effaddr, uint32_t){
	uint8_t align = effaddr%4;
	
	if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	return (data&MASK_08b)<<(3-align)*8;
}
testResult SBResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, SB, SBverify);
}

uint32_t SHverify(uint32_t data, uint32_t effaddr, uint32_t){
	uint8_t align = effaddr%4;
	
	if( align%2 )
		throw mips_ExceptionInvalidAlignment;
	else if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	else
		return align ? data&MASK_16b : (data&MASK_16b)<<16;
}
testResult SHResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, SH, SHverify);
}

uint32_t SUBverify(uint32_t r1, uint32_t r2, uint8_t){
	if(((signed)r2>0 && (signed)r1<INT32_MIN+(signed)r2) ||
	   ((signed)r2<0 && (signed)r1>INT32_MAX-(signed)r2))
		throw mips_ExceptionArithmeticOverflow;
	else
		return (uint32_t)((signed)r1-(signed)r2);
}
testResult SUBResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SUB, (verifyFuncR)SUBverify);
}

uint32_t SUBUverify(uint32_t r1, uint32_t r2, uint8_t){
	return r1-r2;
}
testResult SUBUResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SUBU, (verifyFuncR)SUBUverify);
}

uint32_t SLLverify(uint32_t, uint32_t r1, uint8_t shift){
	return r1<<shift;
}
testResult SLLResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SLL, (verifyFuncR)SLLverify);
}

uint32_t SLLVverify(uint32_t r1, uint32_t r2, uint8_t){
	return r2<<r1;
}
testResult SLLVResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SLLV, (verifyFuncR)SLLVverify);
}

uint32_t SLTverify(uint32_t r1, uint32_t r2, uint8_t){
	return ((signed)r1<(signed)r2) ? 1 : 0;
}
testResult SLTResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SLT, (verifyFuncR)SLTverify);
}

uint32_t SLTIverify(uint32_t r1, uint16_t imm){
	return imm&0x8000
			? (signed)r1<(signed)( imm|0xFFFF0000 ) ? 1 : 0
			: (signed)r1<(signed)( imm|0x00000000 ) ? 1 : 0;
}
testResult SLTIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, SLTI, (verifyFuncI)SLTIverify);
}

uint32_t SLTIUverify(uint32_t r1, uint16_t imm){
	//Immediate is sign extended, but compared as unsigned
	return imm&0x8000
			? r1<( imm|0xFFFF0000 ) ? 1 : 0
			: r1<( imm|0x00000000 ) ? 1 : 0;
}
testResult SLTIUResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, SLTIU, (verifyFuncI)SLTIUverify);
}

uint32_t SLTUverify(uint32_t r1, uint32_t r2, uint8_t){
	return (r1<r2) ? 1 : 0;
}
testResult SLTUResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SLTU, (verifyFuncR)SLTUverify);
}

uint32_t SRAverify(uint32_t, uint32_t r1, uint8_t shift){
	return (signed)r1>>shift;
}
testResult SRAResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SRA, (verifyFuncR)SRAverify);
}

uint32_t SRLverify(uint32_t, uint32_t r1, uint8_t shift){
	return r1>>shift;
}
testResult SRLResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SRL, (verifyFuncR)SRLverify);
}

uint32_t SRLVverify(uint32_t r1, uint32_t r2, uint8_t){
	return r2>>r1;
}
testResult SRLVResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, SRLV, (verifyFuncR)SRLVverify);
}

uint32_t SWverify(uint32_t data, uint32_t effaddr, uint32_t){
	uint8_t align = effaddr%4;
	
	if( align )
		throw mips_ExceptionInvalidAlignment;
	else if( effaddr > MEM_SIZE )
		throw mips_ExceptionInvalidAddress;
	else
		return data;
}
testResult SWResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, SW, SWverify);
}

uint32_t XORverify(uint32_t r1, uint32_t r2, uint8_t){
	return r1^r2;
}
testResult XORResult(mips_cpu_h cpu, mips_mem_h mem){
	return RTypeResult(cpu, mem, XOR, (verifyFuncR)XORverify);
}

uint32_t XORIverify(uint32_t r1, uint16_t i){
	return r1^(0x00000000|i);
}
testResult XORIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, XORI, (verifyFuncI)XORIverify);
}
