//
//  mips_cpu.cpp
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "include/mips_cpu.h"
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
	else
		throw mips_InternalError;
};
//

/*
 * MIPS GP register
 */
mips_regset_gp::mips_regset_gp(void) : _r0(false){};
	
mips_register&  mips_regset_gp::operator[](unsigned idx){
	if(idx >= MIPS_NUM_REG)
		throw mips_ErrorInvalidArgument;
	else
		return idx==0 ? _r0 : _r[idx];
}
//

/*
 * MIPS SP register
 */
mips_reg_sp::mips_reg_sp(void) : mips_register(false){};

void mips_reg_sp::internal_set(uint32_t iVal){
	_value = iVal;
}
//

/*
 * MIPS PC register
 */
mips_reg_pc::mips_reg_pc(mips_reg_sp* npc) : _npc(npc), mips_reg_sp(){};

void mips_reg_pc::advance(int32_t offset=0){
	internal_set(_npc->value()+offset);
}
//

/*
 * MIPS ALU
 */
mips_alu::mips_alu(uint32_t* a, uint32_t* b) : in_a(a), in_b(b){};

void mips_alu::setOperation(mips_asm mnem){
	_operation = aluOp[mnem];
}

void mips_alu::execute(uint32_t* out) const{
	std::cout << "Executing..." << std::endl;
	*out = _operation(*in_a, *in_b);
	std::cout << "Result of ALU operation was 0x" << *out << std::endl;
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
mips_cpu::mips_cpu(mips_mem* mem) : r(), _alu(&_alu_in_a, &_alu_in_b),
									_pc(&_npc), _npc(), _hi(), _lo(), _stage(IF), _mem_ptr(mem){};

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
	uint32_t	aluOut;
	uint8_t		buf[4];
	
	fetchInstr();
	_npc.internal_set(pc()+4);
	
	_stage = ID;
	
	decode();
	fetchRegs(_alu.in_a, _alu.in_b);
	
	_stage = EX;
	//magic
	_alu.execute(&aluOut);
	
	_stage = MEM;
	// bit less magic
	switch(_irDecoded->mnemonic()){
		//branch
		case BEQ:
			aluOut ? _pc.advance() : _pc.advance(aluOut);
			//branch has no WB stage
			_stage = IF;
			return;
		case BGEZ:
			~(aluOut&0x80000000) ? _pc.advance(aluOut) : _pc.advance();
			_stage = IF;
			return;
		case BGEZAL:
			if(~aluOut&0x7FFFFFFF){
				_pc.advance(aluOut);
				link();
			} else{
				_pc.advance();
			}
			_stage = IF;
			return;
		case BGTZ:
			if(~(aluOut&0x80000000)){
				_pc.advance(aluOut);
				link();
			} else{
				_pc.advance();
			}
			_stage = IF;
			return;
		case BLEZ:
			aluOut&0x7FFFFFFF ? _pc.advance(aluOut) : _pc.advance();
			_stage = IF;
			return;
		case BLTZ:
			aluOut&0x7FFFFFFF && ~aluOut ? _pc.advance(aluOut) : _pc.advance();
			_stage = IF;
			return;
		case BLTZAL:
			if(aluOut&0x7FFFFFFF && ~aluOut){
				_pc.advance(aluOut);
				link();
			} else{
				_pc.advance();
			}
			_stage = IF;
			return;
		case BNE:
			aluOut ? _pc.advance(aluOut) : _pc.advance();
			_stage = IF;
			return;
		
		//load
		case LB:
		case LBU:
		case LW:
		case LWL:
		case LWR:
			_mem_ptr->read(buf, aluOut, 4);
			_lmd.value(buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
		
		//store
		case SB:
		case SH:
		case SW:
			for(int i=0; i<4; ++i)
				buf[i] = (uint8_t)( ((_irDecoded->regT())>>(3-i)*8)&MASK_08b );
			_mem_ptr->write(aluOut, 4, buf);
		
		default:
			break;
	}
	
	_stage = WB;
	// more magic
	switch(_irDecoded->type()){
		case RType:
			r[ _irDecoded->regD() ].value(aluOut);
			break;
		case IType:
			switch(_irDecoded->mnemonic()){
				case LB:
				case LBU:
				case LW:
				case LWL:
				case LWR:
					r[ _irDecoded->regT() ].value(_lmd.value());
				default:
					r[ _irDecoded->regT() ].value(aluOut);
			}
			break;
		case JType:
			throw mips_InternalError;
	}
	
	_pc.advance();
}

//make sure we know "we are the CPU" when setting
void mips_cpu::internal_pc_set(uint32_t iVal){
	_pc.internal_set(iVal);
}

uint32_t mips_cpu::pc(void) const{
	return _pc.value();
}

void mips_cpu::fetchInstr(){
	std::cout << "Fetching instruction..." << std::endl;
	uint8_t	buf[4];
	_mem_ptr->read(buf, pc(), 4);
	_ir.internal_set(buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
	std::cout << "Fetched 0x" << std::hex << (int)buf[0] << (int)buf[1] << (int)buf[2] << (int)buf[3] << std::endl;
	_npc.internal_set(pc()+4);
	std::cout << "Set NPC to " << std::hex << _npc.value() << std::endl;
}

void mips_cpu::decode(){
	std::cout << "Decoding..." << std::endl;
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
			std::cout << "Matched: instr #" << i << " (see enum)" << std::endl;
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
	std::cout << "Fetching registers..." << std::endl;
	mips_instr_type type = _irDecoded->type();
	bool isSigned;

	//ALU inputs
	switch(type){
		case RType:
			*aluInA = r[ _irDecoded->regS() ].value();
			*aluInB = r[ _irDecoded->regT() ].value() << _irDecoded->shift();
			break;
		
		case IType:
			*aluInA = r[ _irDecoded->regS() ].value();
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
	
	std::cout << "Set ALU input A to: 0x" << std::hex << *aluInA << std::endl;
	std::cout << "Set ALU input B to: 0x" << std::hex << *aluInB << std::endl;
}

void mips_cpu::link(void){
	r[31].value(_npc.value()+4);
}
//
