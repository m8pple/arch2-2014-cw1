//
//  mips_cpu.cpp
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "include/mips_cpu.h"

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
mips_regset_gp::mips_regset_gp(void) : _r0(false){};
	
mips_register&  mips_regset_gp::operator[](int idx){
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
 * MIPS PC register
 */
mips_reg_pc::mips_reg_pc(mips_reg_sp* npc) : _npc(npc), mips_reg_sp(){};

void mips_reg_pc::advance(int32_t offset=4){
	internal_set(_npc->value()+offset);
}
//

/*
 * MIPS ALU
 */
void mips_alu::setOperation(mips_asm mnem){
	_operation = aluOp[mnem];
}

void mips_alu::execute(uint32_t* out) const{
	*out = _operation(*in_a, *in_b);
}

uint32_t mips_alu::alu_add(uint32_t a, uint32_t b){
	return (int32_t)a+(int32_t)b;
}
uint32_t mips_alu::alu_addu(uint32_t a, uint32_t b){
	return a+b;
}
uint32_t mips_alu::alu_and(uint32_t a, uint32_t b){
	return a&b;
}
uint32_t mips_alu::alu_divide(uint32_t a, uint32_t b){
	throw mips_ErrorNotImplemented;
}
uint32_t mips_alu::alu_divideu(uint32_t a, uint32_t b){
	throw mips_ErrorNotImplemented;
}
uint32_t mips_alu::alu_or(uint32_t a, uint32_t b){
	return a|b;
}
uint32_t mips_alu::alu_multiply(uint32_t a, uint32_t b){
	throw mips_ErrorNotImplemented;
}
uint32_t mips_alu::alu_multiplyu(uint32_t a, uint32_t b){
	throw mips_ErrorNotImplemented;
}
uint32_t mips_alu::alu_shiftleft(uint32_t a, uint32_t b){
	return a<<b;
}
uint32_t mips_alu::alu_sub(uint32_t a, uint32_t b){
	return (int32_t)a-(int32_t)b;
}
uint32_t mips_alu::alu_subu(uint32_t a, uint32_t b){
	return a-b;
}
uint32_t mips_alu::alu_shiftright(uint32_t a, uint32_t b){
	return ((int32_t)a)>>b;
}
uint32_t mips_alu::alu_shiftrightu(uint32_t a, uint32_t b){
	return a>>b;
}
uint32_t mips_alu::alu_xor(uint32_t a, uint32_t b){
	return a^b;
}
//

/*
 * MIPS CPU
 */
mips_cpu::mips_cpu(mips_mem* mem) : r(), _pc(&_npc), _npc(), _hi(), _lo(), _stage(IF), _mem_ptr(mem){};

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
	uint32_t pcOffset = 4;
	uint32_t aluOut;
	uint32_t aluInA, aluInB;
	
	fetchInstr();
	_npc.value(pc()+4);
	
	_stage = ID;
	
	decode();
	fetchRegs(&aluInA, &aluInB);
	
	_stage = EX;
	
	// magic
	
	_stage = MEM;
	
	// bit less magic
	bool cond;
	bool isBranch;
	if(isBranch){
		if(cond)
			pcOffset = aluOut;
		else
			pcOffset = 4;
		
		//no WB stage for branch
		_stage = IF;
		return;
	}
	
	_stage = WB;
	
	// more magic
	_pc.advance(pcOffset);
}

//make sure we know "we are the CPU" when setting
void mips_cpu::internal_pc_set(uint32_t iVal){
	_pc.internal_set(iVal);
}

uint32_t mips_cpu::pc(void) const{
	return _pc.value();
}

void mips_cpu::fetchInstr(){
	uint8_t	buf[4];
	_mem_ptr->read(buf, pc(), 4);
	_ir.value((uint32_t)*buf);
}

void mips_cpu::decode(){
	Instruction *ir = new Instruction(_ir.value());
	
	for(int i=0; i<NUM_INSTR; ++i){
		if(	mipsInstruction[i].opco == ir->opcode() && (
			(	mipsInstruction[i].type == RType
			 &&	mipsInstruction[i].func == ir->function() )
		  ||(	mipsInstruction[i].type != RType
			 &&	mipsInstruction[i].func == FIELD_NOT_EXIST)
		  ||(	mipsInstruction[i].type == IType
			 &&	mipsInstruction[i].func == ir->regT())
		)){
			ir->setDecoded((mips_asm)i, mipsInstruction[i].type);
			_irDecoded = ir;
			
			_alu.setOperation((mips_asm)i);
			return;
		}
	}
	//no match
	
	throw mips_ExceptionInvalidInstruction;
}

void mips_cpu::fetchRegs(uint32_t* aluInA, uint32_t* aluInB){
	mips_instr_type type = _irDecoded->type();
	bool isSigned;
	
	//Immediate
	if( type == IType ){
		
	}
	
	//ALU inputs
	switch(type){
		case RType:
			*aluInA = _irDecoded->regS();
			*aluInB = _irDecoded->regT();	//this should be shifted?
			break;
		
		case IType:
			*aluInA = _irDecoded->regS();
			isSigned = (_irDecoded->immediate()&(MASK_IMDT>>1))
							!=	(_irDecoded->immediate()&(MASK_IMDT));
			*aluInB = isSigned
						?	_irDecoded->immediate()|(0xFFFFFFFF&~MASK_IMDT)
						:	_irDecoded->immediate();
			break;
		
		case JType:
			*aluInA = pc()&0xf0000000;
			*aluInB = _irDecoded->target()<<2;
			break;
		
		default:
			throw mips_InternalError;
	}
	
}
//
