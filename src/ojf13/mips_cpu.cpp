//
//  mips_instr.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "mips.h"
#include "include/mips_cpu_impl.h"

#include <iostream>

/*
 * MIPS register
 */
mips_register::mips_register(void) : _value(0), _allowSet(true){};
mips_register::mips_register(bool isZero) : _value(0), _allowSet(isZero){};

uint32_t mips_register::value(void) const{
	return _value;
}
void mips_register::value(uint32_t iVal){
	if(_allowSet)
		_value = iVal;
};
//

/*
 * MIPS GP register
 */
mips_gp_regs::mips_gp_regs(void) : _r0(false){};
	
mips_register&  mips_gp_regs::operator[](int idx){
	if(idx >= MIPS_NUM_REG)
		throw mips_ErrorInvalidArgument;
	else
		return idx==0 ? _r0 : _r[idx-1];
}
//

/*
 * MIPS SP register
 */
mips_reg_sp::mips_reg_sp(void) : mips_register(false){};

//make sure we know "we are the CPU" when setting
void mips_reg_sp::value(uint32_t) const{
	throw mips_ExceptionAccessViolation;
}
void mips_reg_sp::internal_set(uint32_t iVal){
	_value = iVal;
}
//

/*
 * MIPS
 */
mips_reg_pc::mips_reg_pc(void) : mips_reg_sp(){};

void mips_reg_pc::advance(void){
	internal_set(value()+4);
}
//

/*
 * MIPS CPU
 */
mips_cpu::mips_cpu(mips_mem_h mem) : r(), _pc(), _hi(), _lo(), _stage(IF), _mem_ptr(mem){};

void mips_cpu::reset(void){
	
	//Zero registers
	for(int i=0; i<MIPS_NUM_REG; ++i){
		r[i].value(0);
	}
	
	_pc.internal_set(0);
	_hi.internal_set(0);
	_lo.internal_set(0);
}

void mips_cpu::step(void){
	_pc.internal_set(_pc.value()+4);
	_pc.advance();
}

//make sure we know "we are the CPU" when setting
void mips_cpu::internal_pc_set(uint32_t iVal){
	_pc.internal_set(iVal);
}

uint32_t mips_cpu::pc(void) const{
	return _pc.value();
}
//
