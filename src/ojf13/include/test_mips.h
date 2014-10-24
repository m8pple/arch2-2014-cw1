//
//  test_mips.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#ifndef MIPS_NUM_REG
#define MIPS_NUM_REG 32U
#endif

struct testResult{
    const char* instr;
    const char* desc;
    int passed;
};

void runTest(testResult (mips_cpu_h, mips_mem_h), mips_cpu_h, mips_mem_h);
testResult registerReset(mips_cpu_h, mips_mem_h);
testResult memoryIO(mips_cpu_h, mips_mem_h);
testResult rTypeAnd(mips_cpu_h, mips_mem_h);
