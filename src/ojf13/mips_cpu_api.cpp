//
//  mips_cpu_api.cpp
//  MIPSim
//
//  Created by Ollie Ford on 23/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "mips.h"
#include "include/mips_cpu.h"

struct mips_cpu_impl: mips_cpu{};

mips_cpu_h mips_cpu_create(mips_mem_h mem){
	return (mips_cpu_h)new mips_cpu((mips_mem*)mem);
}

mips_error mips_cpu_reset(mips_cpu_h cpu){
	try{
		cpu->reset();
		return mips_Success;
	} catch(mips_error  e){
		return e;
	}
}

mips_error mips_cpu_get_register(mips_cpu_h cpu, unsigned idx, uint32_t *oVal){
	try{
		*oVal = cpu->r[idx].value();
		return mips_Success;
	} catch(mips_error  e){
		return e;
	}
}

mips_error mips_cpu_set_register(mips_cpu_h cpu, unsigned idx, uint32_t iVal){
	try{
		cpu->r[idx].value(iVal);
		return mips_Success;
	} catch (mips_error e){
		return e;
	}
}

mips_error mips_cpu_set_pc(mips_cpu_h cpu, uint32_t iVal){
	try{
		cpu->internal_pc_set(iVal);
		return mips_Success;
	} catch(mips_error  e){
		return e;
	}
}

mips_error mips_cpu_get_pc(mips_cpu_h cpu, uint32_t *oVal){
	try{
		*oVal = cpu->pc();
		return mips_Success;
	} catch(mips_error  e){
		return e;
	}
}

mips_error mips_cpu_step(mips_cpu_h cpu){
	try{
		cpu->step();
		return mips_Success;
	}
	catch(mips_error e){
		return e;
	}
}

mips_error mips_cpu_set_debug_level(mips_cpu_h cpu, unsigned level, FILE *dest){
	try{
		cpu->setDebug(dest, (debug_level)level);
		return mips_Success;
	} catch(mips_error e){
		return e;
	}
}

void mips_cpu_free(mips_cpu_h cpu){
	delete cpu;
}
