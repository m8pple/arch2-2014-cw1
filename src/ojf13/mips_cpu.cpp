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
mips_register::mips_register(void) : _value(0), _isZero(false){};
mips_register::mips_register(bool isZero) : _value(0), _isZero(isZero){};

uint32_t mips_register::value(void) const{
	return _value;
}
void mips_register::value(uint32_t iVal){
	if(!_isZero)
		_value = iVal;
	else;
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
		return idx==0 ? _r0 : _r[idx-1];
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

void mips_reg_pc::advance(void){
	internal_set(_npc->value());
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
	int64_t p = (signed)a+(signed)b;
	if((int32_t)p == p)
		return (uint32_t)p;
	else
		throw mips_ExceptionArithmeticOverflow;
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
	return (signed)a-(signed)b;
}
uint32_t mips_alu::alu_subu(uint32_t a, uint32_t b){
	return a-b;
}
uint32_t mips_alu::alu_shiftright(uint32_t a, uint32_t b){
	return ((signed)a)>>b;
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
	for(int i=0; i<MIPS_NUM_REG; ++i)
		r[i].value(0);
	
	_pc.internal_set(0);
	_npc.internal_set(0);
	_ir.internal_set(0);
	_hi.internal_set(0);
	_lo.internal_set(0);
}

void mips_cpu::step(void){
	uint32_t	aluOut;
	
	_stage = IF;
	
	fetchInstr();
	
	_stage = ID;
	
	decode();
	fetchRegs(_alu.in_a, _alu.in_b);
	
	_stage = EX;
	//magic
	_alu.execute(&aluOut);
	
	_stage = MEM;
	// bit less magic
	bool wb = accessMem(&aluOut);

	if(wb){
		_stage = WB;
		// more magic
		writeBack(&aluOut);
	}
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
	
	_npc.internal_set(_npc.value()+4);
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

			std::cout << "Matched: " << mipsInstruction[i].mnem << std::endl;
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
	bool isSigned = true;

	//ALU inputs
	switch(type){
		case RType:
			switch(_irDecoded->mnemonic()){
				case SLL:
				case SRL:
				case SRA:
					*aluInA = _irDecoded->shift();
					break;
				default:
					*aluInA = r[ _irDecoded->regS() ].value();
			}
			
			*aluInB = r[ _irDecoded->regT() ].value();
			break;
		
		case IType:
			*aluInA = r[ _irDecoded->regS() ].value();
			switch(_irDecoded->mnemonic()){
				case BEQ:
				case BGEZ:
				case BGEZAL:
				case BGTZ:
				case BLEZ:
				case BLTZ:
				case BLTZAL:
				case BNE:
					*aluInB  = r[ _irDecoded->regT() ].value();
					break;
				case ADDIU:
				case SLTIU:
				case ANDI:
				case ORI:
				case XORI:
					isSigned = false;
				default:
					*aluInB	 = isSigned
								? signExtendImdt(_irDecoded->immediate())
								: _irDecoded->immediate();
			}
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

bool mips_cpu::accessMem(const uint32_t* aluOut){
	std::cout << "Accessing memory..." << std::endl;
	uint8_t	buf[4];

	switch(_irDecoded->mnemonic()){
			//branch
		case BEQ:
			if((signed)*aluOut==0){
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
		case BGEZ:
			if((signed)*aluOut>=0){
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
		case BGEZAL:
			if((signed)*aluOut>=0){
				link();
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
		case BGTZ:
			if((signed)*aluOut>0){
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
		case BLEZ:
			if((signed)*aluOut<=0){
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
		
		case BLTZ:
			if((signed)*aluOut<0){
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
		case BLTZAL:
			if((signed)*aluOut<0){
				link();
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
		case BNE:
			if((signed)*aluOut!=0){
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = pc();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
			//load
		case LB:
		case LBU:
		case LW:
		case LWL:
		case LWR:
			_mem_ptr->read(buf, *aluOut, 4);
			_lmd.value(buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
			return true;
			
			//store
		case SB:
		case SH:
		case SW:
			for(int i=0; i<4; ++i)
				buf[i] = (uint8_t)( ((_irDecoded->regT())>>(3-i)*8)&MASK_08b );
			_mem_ptr->write(*aluOut, 4, buf);
			return true;
			
		default:
			return true;
	}
}

void mips_cpu::writeBack(const uint32_t* aluOut){
	std::cout << "Writing back result..." << std::endl;
	
	switch(_irDecoded->type()){
		case RType:
			r[ _irDecoded->regD() ].value(*aluOut);
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
					r[ _irDecoded->regT() ].value(*aluOut);
			}
			break;
		case JType:
			throw mips_InternalError;
	}
	
	std::cout << "Advancing PC to NPC (0x" << _npc.value() << ")" << std::endl;
	_pc.advance();
}

void mips_cpu::link(void){
	r[31].value(_npc.value()+4);
}

uint32_t mips_cpu::signExtendImdt(uint16_t imm){
	return ( (imm&(MASK_IMDT>>1)) != imm ) ? imm|~MASK_IMDT : imm;
}
//
