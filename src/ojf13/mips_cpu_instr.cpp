//
//  mips_cpu_instr.cpp
//  MIPSim
//
//  Created by Ollie Ford on 24/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "include/mips_instr.h"

Instruction::Instruction(uint32_t value) : _value(value){};

mips_asm Instruction::mnemonic(void) const{
	return _mnem;
}
mips_instr_type Instruction::type(void) const{
	return _type;
}
void Instruction::setDecoded(const mips_asm mnem, const mips_instr_type type){
	_mnem = mnem;
	_type = type;
}
uint32_t Instruction::opcode(void) const{
	return (_value&MASK_OPCO)>>POS_OPCO;
}
uint32_t Instruction::regS(void) const{
	if( type() != JType )
		return (_value&MASK_SREG)>>POS_SREG;
	else
		throw mips_InternalError;
}
uint32_t Instruction::regT(void) const{
	if( type() != JType )
		return (_value&MASK_TREG)>>POS_TREG;
	else
		throw mips_InternalError;
}
uint32_t Instruction::regD(void) const{
	if( type() == RType )
		return (_value&MASK_DREG)>>POS_DREG;
	else
		throw mips_InternalError;
}
uint32_t Instruction::shift(void) const{
	switch( mnemonic() ){
		case SRL:
		case SRA:
		case SLL:
			return (_value&MASK_SHFT)>>POS_SHFT;
		default:
			throw mips_InternalError;
	}
}
uint32_t Instruction::function(void) const{
	if( type() == RType )
		return (_value&MASK_FUNC)>>POS_FUNC;
	else
		throw mips_InternalError;
}
uint32_t Instruction::immediate(void) const{
	if( type() == IType )
		return (_value&MASK_IMDT)>>POS_IMDT;
	else
		throw mips_InternalError;
}
uint32_t Instruction::target(void) const{
	if( type() == JType )
		return (_value&MASK_TRGT)>>POS_TRGT;
	else
		throw mips_InternalError;
}
