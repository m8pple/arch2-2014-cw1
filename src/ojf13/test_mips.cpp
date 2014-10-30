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
    mips_mem_h mem = mips_mem_create_ram(0xFFFFFFFF, 4);
    mips_cpu_h cpu = mips_cpu_create(mem);
	srand((unsigned)time(NULL));
	std::cout << std::hex;
	
    mips_test_begin_suite();

	for(unsigned i=0; i<NUM_OP_TESTS; ++i)
		runTest(opTests[i], cpu, mem, 128);
	
	runTest(constInputs, cpu, mem, 1);		//
	runTest(registerReset, cpu, mem, 1);	// It's hard to imagine that the correctness
	runTest(memoryIO, cpu, mem, 1);			//	of these would be input-dependent without
	runTest(noOperation, cpu, mem, 1);		//	everything else failing hard, so only do once.
											//
	
    mips_test_end_suite();
	
    mips_cpu_free(cpu);
    mips_mem_free(mem);
	return 0;
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

testResult registerReset(mips_cpu_h cpu, mips_mem_h){
    uint32_t got;
    int correct, i=0;
    
    try{
		for(unsigned i=0; i<MIPS_NUM_REG; ++i)
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
    } catch(mips_error e) {
        correct = 0;
		std::cout << "Error " << e << " accessing registers." << std::endl;
    }
	
	if(!correct)
		std::cout << ((i==0)? "PC" : "GP reg") << " not zeroed" << std::endl;
    
    return {"<INTERNAL>", "Check all registers zeroed after reset.", correct};
}

testResult memoryIO(mips_cpu_h, mips_mem_h mem){
	int correct = 1;
	
	try{
		uint8_t ibuf[4] = {
			(uint8_t)rand(),
			(uint8_t)rand(),
			(uint8_t)rand(),
			(uint8_t)rand()
		};
		mips_mem_write(mem, 0x0, 4, ibuf);
		
		uint8_t obuf[4];
		mips_mem_read(mem, 0x0, 4, obuf);
		
		for(unsigned i=0; i<4; ++i)
			correct &= obuf[i]==ibuf[i] ? 1 : 0;
	
	} catch(mips_error e) {
		correct = 0;
		std::cout << "Error " << e << "reading from/writing to memory." << std::endl;
	}
	return {"<INTERNAL>", "Check can read same value back after write.", correct};
}

testResult constInputs(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	mips_mem_write(mem, 0x0, 4, Instruction(AND, 1, 2, 3, 0).bufferedVal());
	
	try{
		uint32_t ir1 = rand();
		uint32_t ir2 = rand();
		mips_cpu_reset(cpu);
		mips_cpu_set_register(cpu, 1, ir1);
		mips_cpu_set_register(cpu, 2, ir2);

		mips_cpu_step(cpu);
		
		uint32_t or1,or2;
		mips_cpu_get_register(cpu, 1, &or1);
		mips_cpu_get_register(cpu, 2, &or2);
		correct = (or1==ir1) && (or2==ir2);
		
		if(!correct){
			std::cout << "Registers changed from: 0x" << ir1 << ", 0x" << ir2 << std::endl;
			std::cout << "--------------------To: 0x" << or1 << ", 0x" << or2 << std::endl;
		}
		
	} catch(mips_error e){
		correct = 0;
		std::cout << "Error (" << e << ") performing AND." << std::endl;
	}
	return {"<INTERNAL>", "Check input registers unchanged after AND-ing.", correct};
}

testResult noOperation(mips_cpu_h cpu, mips_mem_h mem){
	int correct = 1;
	mips_mem_write(mem, 0x0, 4, Instruction(SLL, 0, 0, 0, 0).bufferedVal());
	
	try{
		mips_cpu_reset(cpu);
		mips_cpu_step(cpu);
		
		uint32_t r[MIPS_NUM_REG];
		unsigned i;
		for(i=0; i<MIPS_NUM_REG; ++i){
			mips_cpu_get_register(cpu, i, &r[i]);
			correct &= ~r[i];
			if(!correct)
				break;	//to keep idx of non-zero reg
		}
		
		if(!correct)
			std::cout << "GP reg " << i << " got set during NOP." << std::endl;
		
	} catch(mips_error e){
		correct = 0;
		std::cout << "Error (" << e << ") performing NOP (SLL 0)." << std::endl;
	}
	return {"<INTERNAL>", "Check only PC changes after NOP.", correct};
}

testResult RTypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncR verfunc){
	int correct = -1;
	uint32_t r1= rand(), r2=rand(), r3, exp;
	uint8_t shift= rand()&(MASK_SHFT>>POS_SHFT);
	
	try{
		
		mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, 3, shift).bufferedVal());
		
		mips_cpu_reset(cpu);
		mips_cpu_set_register(cpu, 1, r1);
		mips_cpu_set_register(cpu, 2, r2);

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
	
	std::string desc = "Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}

testResult ITypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncI verfunc){
	int correct = 1;
	uint32_t r1, r2, exp;
	uint16_t imm;
	
	try{
		r1 = mnemonic==LUI ? 0 : rand();
		imm= rand();
		
		mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, imm).bufferedVal());
		
		mips_cpu_set_register(cpu, 1, r1);
		
		mips_cpu_set_pc(cpu, 0);
		mips_cpu_step(cpu);
		
		mips_cpu_get_register(cpu, 2, &r2);
	
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
	
	std::string desc = "Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}
/* Includes JR, which is RType, but easier to test with J,JAL */
testResult JTypeResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncJ verfunc){
	int correct = -1;
	uint32_t pcInit;
	
	try{
		uint32_t index = rand()&0x03FFFFFC;	//26 bits, aligned
		mips_cpu_reset(cpu);

		try{
			pcInit= rand()&0x0000FFFC;		//seems reasonable
			if(mnemonic == JR){
				mips_cpu_set_register(cpu, 1, index);
				mips_mem_write(mem, pcInit, 4, Instruction(mnemonic, 1, 0,0,0).bufferedVal());
			}
			else
				mips_mem_write(mem, pcInit, 4, Instruction(mnemonic, index).bufferedVal());
			
			//Make sure not a J/B instruction in the delay slot (UB)
			uint8_t tmp[4]={0};
			mips_mem_write(mem, pcInit+4, 4, tmp);
			
		} catch(mips_error e){
			if(e == mips_ExceptionInvalidAddress){
				pcInit = 0;					//just in case..
				mips_mem_write(mem, pcInit, 4, Instruction(mnemonic, index).bufferedVal());
				
				//Make sure not a J/B instruction in the delay slot (UB)
				uint8_t tmp[4]={0};
				mips_mem_write(mem, pcInit+4, 4, tmp);
			}
			else
				throw e;					//ouch
		}

		mips_cpu_set_pc(cpu, pcInit);
		
		uint32_t after1, after2;
		mips_cpu_step(cpu);
		mips_cpu_get_pc(cpu, &after1);
		
		correct = (after1 == pcInit+4);
		
		if(correct == 0){
			std::cout << "Incorrect PC." << std::endl;
			std::cout << "---PC at: 0x" << after1 << " during branch delay slot;" << std::endl;
			std::cout << "Expected: 0x" << pcInit+4 << std::endl;
			goto END_TEST;	//egh.. sorry
		}
		
		mips_cpu_step(cpu);
		mips_cpu_get_pc(cpu, &after2);
		
		uint32_t exp = verfunc(pcInit, index);
		correct = (after2 == exp);
		
		if(correct == 0){
			std::cout << "Incorrect jump." << std::endl;
			std::cout << "Result was: 0x" << after2 << std::endl;
			std::cout << "--Expected: 0x" << exp << std::endl;
		} else if( mnemonic == JAL ){
			uint32_t r31;
			mips_cpu_get_register(cpu, 31, &r31);
			correct = (r31 == pcInit+8);
			
			if(correct == 0){
				std::cout << "Incorrect link." << std::endl;
				std::cout << "Link register was: 0x" << r31 << std::endl;
				std::cout << "---------Expected: 0x" << pcInit+8 << std::endl;
			}
		}
		
	} catch(mips_error e){
		correct = 0;
		std::cout << "Error performing branch: " << e << std::endl;
	};

END_TEST:
	std::string desc = "Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}


testResult branchResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncB verfunc){
	int correct = -1;
	
	try{
		int16_t trgtOffset = rand();
		uint32_t r1 = rand(), r2 = rand()%2 ? r1 : rand();
		
		mips_cpu_reset(cpu);
		if(mnemonic == BEQ || mnemonic == BNE){
			mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, trgtOffset).bufferedVal());
			
			//Make sure not a J/B instruction in the delay slot (UB)
			uint8_t tmp[4]={0};
			mips_mem_write(mem, 0x4, 4, tmp);
		}
		else{
			mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, trgtOffset).bufferedVal());
			
			//Make sure not a J/B instruction in the delay slot (UB)
			uint8_t tmp[4]={0};
			mips_mem_write(mem, 0x4, 4, tmp);
		}
		mips_cpu_set_register(cpu, 1, r1);
		mips_cpu_set_register(cpu, 2, r2);
		
		uint32_t after1, after2;
		mips_cpu_step(cpu);
		mips_cpu_get_pc(cpu, &after1);
		
		correct = (after1 == 0x4);
		
		if(correct == 0){
			std::cout << "Incorrect result." << std::endl;
			std::cout << "---PC at: 0x" << after1 << " during branch delay slot;" << std::endl;
			std::cout << "Expected: 0x" << 4 << std::endl;
			goto END_TEST;
		}
		
		mips_cpu_step(cpu);
		mips_cpu_get_pc(cpu, &after2);
		
		uint32_t exp = verfunc(r1, r2, trgtOffset);
		correct = (after1 == 0x4) && (after2 == exp);
		
		if(correct == 0){
			std::cout << "Incorrect result." << std::endl;
			std::cout << "Branched to: 0x" << after2 << std::endl;
			std::cout << "---Expected: 0x" << exp << std::endl;
		} else if( (mnemonic == BGEZAL || mnemonic == BLTZAL) && exp != 0x8 ){
			uint32_t r31;
			mips_cpu_get_register(cpu, 31, &r31);
			correct = (r31 == 0x8);
			
			if(correct == 0){
				std::cout << "Incorrect result." << std::endl;
				std::cout << "Link register was: 0x" << r31 << std::endl;
				std::cout << "---------Expected: 0x" << 0x8 << std::endl;
			}
		}

	} catch(mips_error e){
		correct = 0;
		std::cout << "Error performing branch: " << e << std::endl;
	}
	
END_TEST:
	std::string desc = "Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}

testResult hiloResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncHL verfunc){
	int correct;
	hilo exp;
	try{
		uint32_t r1 = rand(), r2 = rand();

		mips_cpu_reset(cpu);
		mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, 0, 0).bufferedVal());
		mips_mem_write(mem, 0x4, 4, Instruction(MFLO, 0, 0, 3, 0).bufferedVal());
		mips_mem_write(mem, 0x8, 4, Instruction(MFHI, 0, 0, 4, 0).bufferedVal());
		mips_cpu_set_register(cpu, 1, r1);
		mips_cpu_set_register(cpu, 2, r2);
		
		mips_cpu_step(cpu);
		mips_cpu_step(cpu);
		mips_cpu_step(cpu);
		
		hilo got;
		mips_cpu_get_register(cpu, 3, &(got.lo));
		mips_cpu_get_register(cpu, 4, &(got.hi));
		
		try{
			exp = verfunc(r1, r2);
			correct = (got.lo == exp.lo) && (got.hi == exp.hi);
		}
		catch(mips_error){
			//Result is undefined, so whatever happened was 'correct'
			correct = 1;
		}
		
		
		if(!correct){
			std::cout << "Incorrect result." << std::endl;
			std::cout << "Result was: [0x" << got.hi << ", 0x" << got.lo << "]" << std::endl;
			std::cout << "--Expected: [0x" << exp.hi << ", 0x" << exp.lo << "]" << std::endl;
		}
		
	} catch(mips_error e){
		correct = 0;
		std::cout << "Error during operation: " << e << std::endl;
	}

	std::string desc = "Check result of ";
	desc += mipsInstruction[mnemonic].mnem;
	desc += "-ing.";
	return {mipsInstruction[mnemonic].mnem, desc.c_str(), correct};
}

testResult loadstoreResult(mips_cpu_h cpu, mips_mem_h mem, mips_asm mnemonic, verifyFuncLS verfunc){
	int correct = -1;
	
	uint32_t rtInit = 0x00010203;
	uint8_t idata[4] = {
		(uint8_t)rand(),
		(uint8_t)rand(),
		(uint8_t)rand(),
		(uint8_t)rand()
	};
	uint8_t zero[4] = {0};
	uint8_t align = 0;
	uint32_t odata;
	
	uint32_t exp;
	
	try{
		int16_t offset	= rand()<<2;	//align
		uint32_t addr	= 0x100;		//avoid program memory
		
		if( mnemonic == LB || mnemonic == LBU || mnemonic == LWL || mnemonic == LWR
		   /*|| mnemonic == SH*/ || mnemonic == SB){
			//UNCOMMENT WHEN EXCEPTIONS WORK
			
			align+= rand()%4;
			addr += align;				//mix up the alignment
		}
		
		/* UNCOMMENT WHEN EXCEPTIONS WORK!
		else if(mnemonic == LW || mnemonic == SW){
			
			align+= rand()%2 ? 1 : 0;	//50% chance of being misaligned (test exception)
			addr += align;
		}
		*/
		
		mips_cpu_reset(cpu);

		mips_mem_write(mem, 0x0, 4, Instruction(mnemonic, 1, 2, offset).bufferedVal());
		mips_cpu_set_register(cpu, 1, addr);
		
		if(mnemonic == LB || mnemonic == LBU ||
		   mnemonic == LW || mnemonic == LWL || mnemonic == LWR){
			//Initial value to test LWL/LWR leave half, and others clear
			mips_cpu_set_register(cpu, 2, rtInit);
			
			//Write a word, even if only testing a byte/half
			mips_mem_write(mem, addr+offset-align, 4, idata);
			//Zero around it
			mips_mem_write(mem, addr+offset-align-4, 4, zero);
			mips_mem_write(mem, addr+offset-align+4, 4, zero);

			mips_cpu_step(cpu);
			mips_cpu_get_register(cpu, 2, &odata);
		}
		else{
			//Zero the address we want to test writing to
			mips_mem_write(mem, addr+offset-align, 4, zero);
			mips_cpu_set_register(cpu, 2, (idata[0]<<24|idata[1]<<16|idata[2]<<8|idata[3]));
			
			mips_cpu_step(cpu);
			
			uint8_t t[4];
			mips_mem_read(mem, addr+offset-align, 4, t);
			odata = t[0]<<24|t[1]<<16|t[2]<<8|t[3];
		}

	} catch(mips_error e) {
		try{
			exp = verfunc(idata[0]<<24|idata[1]<<16|idata[2]<<8|idata[3], align, rtInit);
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
			exp = verfunc(idata[0]<<24|idata[1]<<16|idata[2]<<8|idata[3], align, rtInit);
			if( odata == exp )
				correct = 1;	//No exception, correct result
			else{
				correct = 0;	//Incorrect result.
				std::cout << "Incorrect result." << std::endl;
				std::cout << "Load/Stored: 0x" << odata << std::endl;
				std::cout << "---Expected: 0x" << exp << std::endl;
			}
		} catch(mips_error v){
			correct = 0;		//No exception, should have
			std::cout << "Incorrect result." << std::endl;
			std::cout << "Result was: 0x" << odata << std::endl;
			std::cout << "--Expected: Exception " << v << std::endl;
		}
	}
	else;


	std::string desc = "Check result of ";
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

uint32_t ANDIverify(uint32_t r1, uint16_t i){
	return r1&(0x00000000|i);
}
testResult ANDIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, ANDI, (verifyFuncI)ANDIverify);
}

uint32_t BEQverify(uint32_t r1, uint32_t r2, uint16_t offset){
	return ((signed)r1==(signed)r2) ? 4+(offset<<2) : 8;
}
testResult BEQResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BEQ, (verifyFuncB)BEQverify);
}

uint32_t BGEZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1>=(signed)0) ? 4+(offset<<2) : 8;
}
testResult BGEZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BGEZ, (verifyFuncB)BGEZverify);
}

uint32_t BGEZALverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1>=(signed)0) ? 4+(offset<<2) : 8;
}
testResult BGEZALResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BGEZAL, (verifyFuncB)BGEZALverify);
}

uint32_t BGTZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1>(signed)0) ? 4+(offset<<2) : 8;
}
testResult BGTZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BGTZ, (verifyFuncB)BGTZverify);
}

uint32_t BLEZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1<=(signed)0) ? 4+(offset<<2) : 8;
}
testResult BLEZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BLEZ, (verifyFuncB)BLEZverify);
}

uint32_t BLTZverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1<(signed)0) ? 4+(offset<<2) : 8;
}
testResult BLTZResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BLTZ, (verifyFuncB)BLTZverify);
}

uint32_t BLTZALverify(uint32_t r1, uint32_t, uint16_t offset){
	return ((signed)r1<(signed)0) ? 4+(offset<<2) : 8;
}
testResult BLTZALResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BLTZAL, (verifyFuncB)BLTZALverify);
}

uint32_t BNEverify(uint32_t r1, uint32_t r2, uint16_t offset){
	return ((signed)r1!=(signed)r2) ? 4+(offset<<2) : 8;
}
testResult BNEResult(mips_cpu_h cpu, mips_mem_h mem){
	return branchResult(cpu, mem, BNE, (verifyFuncB)BNEverify);
}
/* (DIV|MULT)U? tests also implicitly test MFHI, MFLO */
hilo DIVverify(uint32_t r1, uint32_t r2){
	if(r2 == 0)
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

uint32_t LBverify(uint32_t data, uint8_t align, uint32_t){
	uint32_t ret = (data>>(24-align*8))&0x000000FF;
	return ret&0x00000080 ? ret|0xFFFFFF00 : ret;
}
testResult LBResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LB, LBverify);
}

uint32_t LBUverify(uint32_t data, uint8_t align, uint32_t){
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

uint32_t LWverify(uint32_t data, uint8_t, uint32_t){
	return data;
}
testResult LWResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LW, LWverify);
}

uint32_t LWLverify(uint32_t data, uint8_t align, uint32_t rt){
	return (data<<(align*8) & (MASK_16b<<16)) | (rt & MASK_16b);
}
testResult LWLResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, LWL, LWLverify);
}

uint32_t LWRverify(uint32_t data, uint8_t align, uint32_t rt){
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

uint32_t SBverify(uint32_t data, uint8_t align, uint32_t){
	return (data&MASK_08b)<<(3-align)*8;
}
testResult SBResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, SB, SBverify);
}

uint32_t SHverify(uint32_t data, uint8_t align, uint32_t){
	if(align%2)
		throw mips_ExceptionInvalidAlignment;
	else
		return align ? data&MASK_16b : (data&MASK_16b)<<16;
}
testResult SHResult(mips_cpu_h cpu, mips_mem_h mem){
	return loadstoreResult(cpu, mem, SH, SHverify);
}

uint32_t SUBverify(uint32_t r1, uint32_t r2, uint8_t){
	int64_t p = (signed)r1-(signed)r2;
	if((~(unsigned)p)&0xFFFFFFFF00000000)
		throw mips_ExceptionArithmeticOverflow;
	else
		return p&0xFFFFFFFF;
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
	return ((signed)r1<(signed)(0x00000000|imm)) ? 1 : 0;
}
testResult SLTIResult(mips_cpu_h cpu, mips_mem_h mem){
	return ITypeResult(cpu, mem, SLTI, (verifyFuncI)SLTIverify);
}

uint32_t SLTIUverify(uint32_t r1, uint16_t imm){
	return (r1<(0x00000000|imm)) ? 1 : 0;
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

uint32_t SWverify(uint32_t data, uint8_t align, uint32_t){
	if(align)
		throw mips_ExceptionInvalidAlignment;
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
