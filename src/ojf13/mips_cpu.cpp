//
//  mips_cpu.cpp
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "include/mips_cpu.h"
#include "mips_mem.h"
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
	std::cout << "Advancing PC to NPC (0x" << _npc->value() << ")" << std::endl;
	internal_set(_npc->value());
	_npc->internal_set(_npc->value()+4);
}
//

/*
 * MIPS ALU
 */
mips_alu::mips_alu(uint32_t* out, const uint32_t* a, const uint32_t* b,
				   hilo* hilo) : outp(out), in_a(a), in_b(b), _hilo(hilo){};

void mips_alu::setOperation(mips_asm mnem){
	switch(mnem){
		case ADD:
		case ADDI:
		case JR:
		case LB:
		case LBU:
		case LW:
		case LWL:
		case LWR:
		case SB:
		case SH:
		case SW:
			_operation = alu_add;
			break;
		case ADDIU:
		case ADDU:
			_operation = alu_addu;
			break;
		case AND:
		case ANDI:
			_operation = alu_and;
			break;
		case BEQ:
		case BGEZ:
		case BGEZAL:
		case BGTZ:
		case BLEZ:
		case BLTZ:
		case BLTZAL:
		case BNE:
		case SLT:
		case SLTI:
		case SUB:
			_operation = alu_subtract;
			break;
		case DIV:
			_operation = alu_divide;
			break;
		case DIVU:
			_operation = alu_divideu;
			break;
		case J:
		case JAL:
		case LUI:
		case MFHI:
		case MFLO:
		case OR:
		case ORI:
			_operation = alu_or;
			break;
		case MULT:
			_operation = alu_multiply;
			break;
		case MULTU:
			_operation = alu_multiplyu;
			break;
		case SLL:
		case SLLV:
			_operation = alu_shiftleft;
			break;
		case SLTIU:
		case SLTU:
			_operation = alu_subtractnowrap;
			break;
		case SUBU:
			_operation = alu_subtractu;
			break;
		case SRA:
			_operation = alu_shiftright;
			break;
		case SRL:
		case SRLV:
			_operation = alu_shiftrightu;
			break;
		case XOR:
		case XORI:
			_operation = alu_xor;
	}
}

void mips_alu::execute(void) const{
	std::cout << "Executing..." << std::endl;
	*_hilo = _operation(outp, in_a, in_b);
	std::cout << "Result of ALU operation was 0x" << *outp << std::endl;
}

hilo mips_alu::alu_add(uint32_t* out, const uint32_t* a, const uint32_t* b){
	int64_t p = (signed)*a+(signed)*b;
	if((int32_t)p == p)
		*out = (uint32_t)p;
	else
		throw mips_ExceptionArithmeticOverflow;
	return {};
}
hilo mips_alu::alu_addu(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = *a + *b;
	return {};
}
hilo mips_alu::alu_and(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*a)&(*b);
	return {};
}
hilo mips_alu::alu_divide(uint32_t* out, const uint32_t* a, const uint32_t* b){\
	//Undefined if b==0
	return {
		(uint32_t)((signed)*a/(*b?(signed)*b:1)),
		(uint32_t)((signed)*a%(*b?(signed)*b:1))
	};
}
hilo mips_alu::alu_divideu(uint32_t* out, const uint32_t* a, const uint32_t* b){
	//Undefined if b==0
	return {
		(uint32_t)(*a/(*b?*b:1)),
		(uint32_t)(*a%(*b?*b:1))
	};
}
hilo mips_alu::alu_or(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*a)|(*b);
	return {};
}
hilo mips_alu::alu_multiply(uint32_t* out, const uint32_t* a, const uint32_t* b){
	return {
		(uint32_t)(( ((signed)*a*(signed)*b)&0xFFFFFFFF00000000 ) >> 32 ),
		(uint32_t)(( ((signed)*a*(signed)*b)&0x00000000FFFFFFFF ) )
	};
}
hilo mips_alu::alu_multiplyu(uint32_t* out, const uint32_t* a, const uint32_t* b){
	uint64_t p = (*a)*(*b);
	return {
		(uint32_t)( (p&0xFFFFFFFF00000000) >> 32 ),
		(uint32_t)( (p&0x00000000FFFFFFFF) )
	};
}
hilo mips_alu::alu_shiftleft(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = ((signed)*b) << (*a);
	return {};
}
hilo mips_alu::alu_subtract(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (signed)(*a) - (signed)(*b);
	return {};
}
hilo mips_alu::alu_subtractu(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*a)-(*b);
	return {};
}
hilo mips_alu::alu_subtractnowrap(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*a)<(*b) ? 0 : (*a)-(*b);
	return {};
}
hilo mips_alu::alu_shiftright(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = ((signed)*b) >> (*a);
	return {};
}
hilo mips_alu::alu_shiftrightu(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*b) >> (*a);
	return {};
}
hilo mips_alu::alu_xor(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*a)^(*b);
	return {};
}
//

/*
 * MIPS CPU
 */
mips_cpu::mips_cpu(mips_mem* mem) : r(), _alu(&_alu_out, &_alu_in_a, &_alu_in_b, &_alu_hilo),
									_pc(&_npc), _npc(), _hi(), _lo(), _stage(IF), _mem_ptr(mem){};

void mips_cpu::reset(void){
	
	//Zero registers
	for(int i=0; i<MIPS_NUM_REG; ++i)
		r[i].value(0);
	
	_pc.internal_set(0);
	_npc.internal_set(4);
	_ir.internal_set(0);
	_hi.internal_set(0);
	_lo.internal_set(0);
	
	std::cout << std::endl << "CPU was reset." << std::endl << std::endl;
}

void mips_cpu::step(void){
	//std::cout << std::endl << "Stepping. PC at: 0x" << pc() << std::endl;
	
	_stage = IF;
	
	fetchInstr();
	
	_stage = ID;
	
	decode();
	fetchRegs(&_alu_in_a, &_alu_in_b);
	
	_stage = EX;
	//magic
	_alu.execute();
	
	_stage = MEM;
	// bit less magic
	bool wb = accessMem(&_alu_out);

	if(wb){
		_stage = WB;
		// more magic
		writeBack(&_alu_out);
		_pc.advance();
	}
}

//make sure we know "we are the CPU" when setting
void mips_cpu::internal_pc_set(uint32_t iVal){
	_pc.internal_set(iVal);
	_npc.internal_set(iVal+4);
}

uint32_t mips_cpu::pc(void) const{
	return _pc.value();
}

void mips_cpu::fetchInstr(){
	std::cout << "Fetching instruction..." << std::endl;
	uint8_t	buf[4];
	uint32_t word = readWord(pc());
	_ir.internal_set(word);
	//_ir.internal_set(buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
	std::cout << "Fetched 0x" << std::hex << (int)buf[0] << (int)buf[1] << (int)buf[2] << (int)buf[3] << std::endl;
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
	
	/* BUG: Sometimes ADD r1,r2,r3 (0xFF7F00) comes here. Mostly it doesn't. Wtf. */
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
			*aluInB = r[ _irDecoded->regT() ].value();
			switch(_irDecoded->mnemonic()){
				case SLL:
				case SRL:
				case SRA:
					*aluInA = _irDecoded->shift();
					break;
				
					//MFHI/LO OR respective with 0
				case MFHI:
					*aluInA = _hi.value();
					//*aluInB = 0;	//not necessary if strictly mandate zeroes instead ignore?
					break;
				case MFLO:
					*aluInA = _lo.value();
					//*aluInB = 0;
					break;
				
				default:
					*aluInA = r[ _irDecoded->regS() ].value();
			}
			break;
		
		case IType:
			*aluInA = r[ _irDecoded->regS() ].value();
			switch(_irDecoded->mnemonic()){
				case BGEZ:
				case BGEZAL:
				case BGTZ:
				case BLEZ:
				case BLTZ:
				case BLTZAL:
					*aluInB  = 0;
					break;
				case BEQ:
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
	
	switch(_irDecoded->mnemonic()){
			//branch
		case BEQ:
			if((signed)*aluOut==0){
				std::cout << "Condition met. Taking branch next cycle." << std::endl;
				uint32_t tmp = _npc.value();
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
				uint32_t tmp = _npc.value();
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
				uint32_t tmp = _npc.value();
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
				uint32_t tmp = _npc.value();
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
				uint32_t tmp = _npc.value();
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
				uint32_t tmp = _npc.value();
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
				uint32_t tmp = _npc.value();
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
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
				//branch has no WB stage
				return false;
			} else{
				std::cout << "Condition not met. Branch will not be taken." << std::endl;
				_pc.advance();
				return false;
			}
			
		case JAL:
			link();
		case J:
		case JR:
			std::cout << "Jumping to 0x" << *aluOut << " next cycle." << std::endl;
			_pc.advance();
			_npc.internal_set(*aluOut);
			//branch has no WB stage
			return false;
			
			
			//load
		case LB:
			_lmd.value( signExtend(readByte(*aluOut)) );
			return true;
		case LBU:
			_lmd.value( readByte(*aluOut) );
			return true;
		case LW:
		case LWL:
		case LWR:
			_lmd.value( readWord(*aluOut) );
			return true;
			
			//store
		case SB:
		case SH:
		case SW:
			writeWord(*aluOut, _irDecoded->regT());
			return true;
			
		default:
			return true;
	}
}

void mips_cpu::writeBack(const uint32_t* aluOut){
	std::cout << "Writing back result..." << std::endl;
	
	switch(_irDecoded->type()){
		case RType:
			switch(_irDecoded->mnemonic()){

				case SLTU:// if rs<rt, alu does rs-rt, hence:
					r[ _irDecoded->regD() ].value((unsigned)*aluOut ? 0 : 1);
					break;
				case SLT:
					r[ _irDecoded->regD() ].value((signed)*aluOut>0 ? 0 : 1);
					break;
					
				case DIV:
				case DIVU:
				case MULT:
				case MULTU:
					_hi.internal_set(_alu_hilo.hi);
					_lo.internal_set(_alu_hilo.lo);
					break;
					
				default:
					r[ _irDecoded->regD() ].value(*aluOut);
			}
			break;

		case IType:
			switch(_irDecoded->mnemonic()){
				case LBU:
				case LB:
				case LW:
				case LWL:
				case LWR:
					r[ _irDecoded->regT() ].value( _lmd.value() );
					break;

				case SLTIU:
					r[ _irDecoded->regT() ].value((unsigned)*aluOut ? 0 : 1);
					break;
				case SLTI:
					r[ _irDecoded->regT() ].value((signed)*aluOut>0 ? 0 : 1);
					break;
					
				default:
					r[ _irDecoded->regT() ].value(*aluOut);
			}
			break;
			
		case JType:
			throw mips_InternalError;
	}
}

uint8_t mips_cpu::readByte(uint32_t addr){
	uint8_t ret[4];
	mips_mem_read((mips_mem_h)_mem_ptr, addr, 4, ret);
	return ret[0];
}

void mips_cpu::writeByte(uint32_t addr, uint8_t data){
	uint8_t buf[4];
	mips_mem_read((mips_mem_h)_mem_ptr, addr, 4, buf);
	buf[0] = data;
	mips_mem_write((mips_mem_h)_mem_ptr, addr, 4, buf);
}

uint32_t mips_cpu::readWord(uint32_t addr){
	uint8_t buf[4];
	mips_mem_read((mips_mem_h)_mem_ptr, addr, 4, buf);

	uint32_t word = 0x0;
	for(unsigned i=0; i<4; ++i)
		word |= buf[3-i]<<(i*8);
	
	return word;
}

void mips_cpu::writeWord(uint32_t addr, uint32_t data){
	uint8_t buf[4];
	for(unsigned i=0; i<4; ++i)
		buf[i] = (data >> (i*8))&0x000000FF;
	mips_mem_write((mips_mem_h)_mem_ptr, addr, 4, buf);
}

void mips_cpu::link(void){
	std::cout << "Linking 0x" << _npc.value()+4 << std::endl;
	r[31].value(_npc.value()+4);
}

uint32_t mips_cpu::signExtendImdt(uint16_t imm){
	std::cout << "Sign extending immediate: 0x" << imm << std::endl;
	if( (imm&(MASK_IMDT>>1)) != imm ){
		std::cout << "-------------Padding 1's: 0x" << (imm|~MASK_IMDT) << std::endl;
		return imm|~MASK_IMDT;
	} else {
		std::cout << "-------------Padding 0's: 0x" << imm << std::endl;
		return imm;
	};
}
//
