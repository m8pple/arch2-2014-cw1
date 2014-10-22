//
//  mips_instr.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "../../include/mips.h"
#include "include/mips_cpu.h"

#include <iostream>

struct mips_register{
public:
	mips_register(void) : _value(0), _allowSet(true){};
	mips_register(bool isZero) : _value(0), _allowSet(isZero){};
	
	uint32_t value(void) const{
		return _value;
	}
	void value(uint32_t iVal){
		if(_allowSet)
			_value = iVal;
	};
	
protected:
	uint32_t _value;
	
private:
	bool _allowSet;
};

struct mips_gp_regs{
public:
	mips_gp_regs(void) : _r0(false){};
	
	mips_register& operator[](int idx){
		if(idx >= MIPS_NUM_REG)
			throw mips_ErrorInvalidArgument;
		else
			return idx==0 ? _r0 : _r[idx-1];
	}
	
private:
	mips_register _r0;
	mips_register _r[MIPS_NUM_REG-1];
};

struct mips_reg_sp: mips_register{
public:
	mips_reg_sp(void) : mips_register(false){};
	
	using mips_register::value;
	//make sure we know "we are the CPU" when setting
	void value(uint32_t){
		throw mips_ExceptionAccessViolation;
	}
	void internal_set(uint32_t iVal){
		_value = iVal;
	}
	
private:
};

struct mips_reg_pc: mips_reg_sp{
public:
	mips_reg_pc(void) : mips_reg_sp(){};
	
	void advance(void){
		internal_set(value()+4);
	}
	
private:
};

struct mips_cpu_impl{
public:
	mips_cpu_impl(void) : r(), _pc(), _stage(IF){};
	
	mips_error reset(void){
		
		//Zero registers
		for(int i=0; i<MIPS_NUM_REG; ++i){
			r[i].value(0);
		}
		
		_pc.internal_set(0);
		_hi.internal_set(0);
		_lo.internal_set(0);
		
		return mips_Success;
	}
	
	void step(void){
		_pc.internal_set(_pc.value()+4);
		_pc.advance();
	}
	
	//make sure we know "we are the CPU" when setting
	void internal_pc_set(uint32_t iVal){
		_pc.internal_set(iVal);
	}
	
	uint32_t pc(void) const{
		return _pc.value();
	}
	
	mips_gp_regs	r;
	
private:
	mips_reg_pc		_pc;
	mips_reg_sp		_hi;
	mips_reg_sp		_lo;
	mips_cpu_stage	_stage;
};


mips_cpu_h mips_cpu_create(mips_mem_h mem){
    return new mips_cpu_impl();
}

mips_error mips_cpu_reset(mips_cpu_h cpu){
    return cpu->reset();
}

void mips_cpu_free(mips_cpu_h cpu){
    delete cpu;
}

mips_error mips_cpu_step(mips_cpu_h cpu){
    try{
        cpu->step();
        return mips_Success;
    } catch(mips_error e){
        return e;
    }
}

mips_error mips_cpu_get_register(mips_cpu_h cpu, unsigned idx, uint32_t *oVal){
    mips_error ret = mips_Success;
    if(idx >= MIPS_NUM_REG)
        ret = mips_ErrorInvalidArgument;
    else
        *oVal = cpu->r[idx].value();
    return ret;
}

mips_error mips_cpu_set_register(mips_cpu_h cpu, unsigned idx, uint32_t iVal){
    mips_error ret = mips_Success;
    if(idx >= MIPS_NUM_REG)
        ret = mips_ErrorInvalidArgument;
    else
        try{
            cpu->r[idx].value(iVal);
        } catch (mips_error ret){};
    return ret;
}

mips_error mips_cpu_get_pc(mips_cpu_h cpu, uint32_t *oVal){
    *oVal = cpu->pc();
	
    return mips_Success;
}

mips_error mips_cpu_set_pc(mips_cpu_h cpu, uint32_t iVal){
	cpu->internal_pc_set(iVal);
	return mips_Success;
}
