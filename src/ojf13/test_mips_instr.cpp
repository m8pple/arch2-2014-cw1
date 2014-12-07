//
//  test_mips_instr.cpp
//  MIPSim
//
//  Created by Ollie Ford on 24/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "include/mips_instr.h"

//RType
Instruction::Instruction(mips_asm mnem,
						 unsigned sreg, unsigned treg, unsigned dreg,
						 unsigned shft) : _value(0){
	
	if( mipsInstruction[mnem].type != RType )
		throw mips_ExceptionInvalidInstruction;
	
	_value |= (mipsInstruction[mnem].opco<<POS_OPCO) & MASK_OPCO;
	_value |= (sreg<<POS_SREG) & MASK_SREG;
	_value |= (treg<<POS_TREG) & MASK_TREG;
	_value |= (dreg<<POS_DREG) & MASK_DREG;
	_value |= (shft<<POS_SHFT) & MASK_SHFT;
	_value |= (mipsInstruction[mnem].func<<POS_FUNC) & MASK_FUNC;
}

//IType
Instruction::Instruction(mips_asm mnem,
						 uint32_t sreg, uint32_t treg,
						 uint32_t imdt) : _value(0){
	
	if(		mipsInstruction[mnem].type != IType
	   ||	mipsInstruction[mnem].func != FIELD_NOT_EXIST )
		throw mips_ExceptionInvalidInstruction;
	
	_value |= (mipsInstruction[mnem].opco<<POS_OPCO) & MASK_OPCO;
	_value |= (sreg<<POS_SREG) & MASK_SREG;
	_value |= (treg<<POS_TREG) & MASK_TREG;
	_value |= (imdt<<POS_IMDT) & MASK_IMDT;
}
Instruction::Instruction(mips_asm mnem,
						 uint32_t sreg,
						 uint32_t imdt) : _value(0){
	
	if(		mipsInstruction[mnem].type != IType
	   ||	mipsInstruction[mnem].func == FIELD_NOT_EXIST )
		throw mips_ExceptionInvalidInstruction;
	
	_value |= (mipsInstruction[mnem].opco<<POS_OPCO) & MASK_OPCO;
	_value |= (sreg<<POS_SREG) & MASK_SREG;
	//mips_instr::func holds the 'treg' for branch in this case
	_value |= (mipsInstruction[mnem].func<<POS_TREG) & MASK_TREG;
	_value |= (imdt<<POS_IMDT) & MASK_IMDT;
}

//JType
Instruction::Instruction(mips_asm mnem,
						 uint32_t trgt) : _value(0){
	
	if( mipsInstruction[mnem].type != JType )
		throw mips_ExceptionInvalidInstruction;
	
	_value |= (mipsInstruction[mnem].opco<<POS_OPCO) & MASK_OPCO;
	_value |= (trgt<<POS_TRGT) & MASK_TRGT;
}

uint8_t* Instruction::bufferedVal() const{
	uint8_t *obuf = new uint8_t[4];
	for(int i=0; i<4; ++i)
		obuf[i] = (_value & (MASK_08b << 8*(3-i))) >> 8*(3-i);
	return obuf;
}
