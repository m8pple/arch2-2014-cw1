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

#define NUM_TESTS		36U

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

struct hilo{
	uint32_t hi;
	uint32_t lo;
};

typedef int (*verifyFuncI)(uint32_t, uint16_t);
typedef int (*verifyFuncR)(uint32_t, uint32_t, uint32_t);
typedef int (*verifyFuncB)(uint32_t, uint32_t, uint32_t);
typedef hilo (*verifyFuncHL)(uint32_t, uint32_t);
testResult RTypeResult(mips_cpu_h, mips_mem_h, mips_asm, verifyFuncR);
testResult ITypeResult(mips_cpu_h, mips_mem_h, mips_asm, verifyFuncI);
testResult branchResult(mips_cpu_h, mips_mem_h, mips_asm, verifyFuncB);
testResult hiloResult(mips_cpu_h, mips_mem_h, mips_asm, verifyFuncHL);

testResult registerReset(mips_cpu_h, mips_mem_h);
testResult memoryIO(mips_cpu_h, mips_mem_h);
testResult constInputs(mips_cpu_h, mips_mem_h);
testResult ADDResult(mips_cpu_h, mips_mem_h);
testResult ADDIResult(mips_cpu_h, mips_mem_h);
testResult ADDIUResult(mips_cpu_h, mips_mem_h);
testResult ADDUResult(mips_cpu_h, mips_mem_h);
testResult ANDResult(mips_cpu_h, mips_mem_h);
testResult ANDIResult(mips_cpu_h, mips_mem_h);
testResult BEQResult(mips_cpu_h, mips_mem_h);
testResult BGEZResult(mips_cpu_h, mips_mem_h);
testResult BGEZALResult(mips_cpu_h, mips_mem_h);
testResult BGTZResult(mips_cpu_h, mips_mem_h);
testResult BLEZResult(mips_cpu_h, mips_mem_h);
testResult BLTZResult(mips_cpu_h, mips_mem_h);
testResult BLTZALResult(mips_cpu_h, mips_mem_h);
testResult BNEResult(mips_cpu_h, mips_mem_h);
testResult DIVResult(mips_cpu_h, mips_mem_h);
testResult DIVUResult(mips_cpu_h, mips_mem_h);
testResult MULTResult(mips_cpu_h, mips_mem_h);
testResult MULTUResult(mips_cpu_h, mips_mem_h);
testResult ORResult(mips_cpu_h, mips_mem_h);
testResult ORIResult(mips_cpu_h, mips_mem_h);

testResult SLLResult(mips_cpu_h, mips_mem_h);
testResult SLLVResult(mips_cpu_h, mips_mem_h);
testResult SLTResult(mips_cpu_h, mips_mem_h);
testResult SLTIResult(mips_cpu_h, mips_mem_h);
testResult SLTIUResult(mips_cpu_h, mips_mem_h);
testResult SLTUResult(mips_cpu_h, mips_mem_h);
testResult SRAResult(mips_cpu_h, mips_mem_h);
testResult SRLResult(mips_cpu_h, mips_mem_h);
testResult SRLVResult(mips_cpu_h, mips_mem_h);
testResult SUBResult(mips_cpu_h, mips_mem_h);
testResult SUBUResult(mips_cpu_h, mips_mem_h);

testResult XORResult(mips_cpu_h, mips_mem_h);
testResult XORIResult(mips_cpu_h, mips_mem_h);

testResult (*tests[NUM_TESTS])(mips_cpu_h, mips_mem_h) = {
	registerReset,
	memoryIO,
	constInputs,
	ADDResult,
	ADDIResult,
	ADDIUResult,
	ADDUResult,
	ANDResult,
	ANDIResult,
	BEQResult,
	BGEZResult,
	BGEZALResult,
	BGTZResult,
	BLEZResult,
	BLTZResult,
	BLTZALResult,
	BNEResult,
	DIVResult,
	DIVUResult,
	MULTResult,
	MULTUResult,
	ORResult,
	ORIResult,
	
	SLLResult,
	SLLVResult,
	SLTResult,
	SLTIResult,
	SLTIUResult,
	SLTUResult,
	SRAResult,
	SRLResult,
	SRLVResult,
	SUBResult,
	SUBUResult,
	
	XORResult,
	XORIResult
};
