//
//  test_mips.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#ifndef MIPS_NUM_REG
#define MIPS_NUM_REG	32U
#endif

#define NUM_TESTS		 8U

#if DEBUG==1
#include "mips_instr.h"
#else
#include "include/mips_instr.h"
#endif

struct testResult{
    const char* instr;
    const char* desc;
    int passed;
};

void runTest(testResult(*test)(mips_cpu_h, mips_mem_h), mips_cpu_h, mips_mem_h, unsigned=1024);

typedef int (*verifyFuncI)(uint32_t, uint16_t);
typedef int (*verifyFuncR)(uint32_t, uint32_t, uint32_t);
testResult RTypeResult(mips_cpu_h, mips_mem_h, mips_asm, verifyFuncR);
testResult ITypeResult(mips_cpu_h, mips_mem_h, mips_asm, verifyFuncI);

testResult registerReset(mips_cpu_h, mips_mem_h);
testResult memoryIO(mips_cpu_h, mips_mem_h);
testResult ADDResult(mips_cpu_h, mips_mem_h);
testResult ADDIResult(mips_cpu_h, mips_mem_h);
testResult ADDIUResult(mips_cpu_h, mips_mem_h);
testResult ADDUResult(mips_cpu_h, mips_mem_h);
testResult ANDResult(mips_cpu_h, mips_mem_h);
testResult ANDInputs(mips_cpu_h, mips_mem_h);

testResult (*tests[NUM_TESTS])(mips_cpu_h, mips_mem_h) = {
	registerReset,
	memoryIO,
	ADDResult,
	ADDIResult,
	ADDIUResult,
	ADDUResult,
	ANDResult,
	ANDInputs
};
