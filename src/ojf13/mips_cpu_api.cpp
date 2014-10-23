//
//  mips_cpu_api.cpp
//  MIPSim
//
//  Created by Ollie Ford on 23/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "mips.h"
#include "include/mips_cpu_impl.h"

struct mips_cpu_impl: mips_cpu{};

mips_cpu_h mips_cpu_create(mips_mem_h mem){
	return (mips_cpu_h)new mips_cpu((mips_mem*)mem);
}

mips_error mips_cpu_reset(mips_cpu_h cpu){
	mips_error ret = mips_Success;
	try{
		cpu->reset();
	} catch(mips_error ret){};
	
	return ret;
}

mips_error mips_cpu_get_register(mips_cpu_h cpu, unsigned idx, uint32_t *oVal){
	mips_error ret = mips_Success;
	try{
		*oVal = cpu->r[idx].value();
	} catch(mips_error ret){};
	
	return ret;
}

mips_error mips_cpu_set_register(mips_cpu_h cpu, unsigned idx, uint32_t iVal){
	mips_error ret = mips_Success;
	try{
		cpu->r[idx].value(iVal);
	} catch (mips_error ret){};
	
	return ret;
}

mips_error mips_cpu_set_pc(mips_cpu_h cpu, uint32_t iVal){
	mips_error ret = mips_Success;
	try{
		cpu->internal_pc_set(iVal);
	} catch(mips_error ret){};
	
	return ret;
}

mips_error mips_cpu_get_pc(mips_cpu_h cpu, uint32_t *oVal){
	mips_error ret = mips_Success;
	try{
		*oVal = cpu->pc();
	} catch(mips_error ret){};
	
	return ret;
}

mips_error mips_cpu_step(mips_cpu_h cpu){
	mips_error ret = mips_Success;
	try{
		cpu->step();
	}
	catch(mips_error ret){};
	
	return ret;
}

mips_error mips_cpu_set_debug_level(mips_cpu_h cpu, unsigned level, FILE *dest){
	return mips_ErrorNotImplemented;
}

void mips_cpu_free(mips_cpu_h cpu){
	delete cpu;
}
