//
//  mips_cpu.cpp
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#define MASK_BYTE 0x000000FF
#define MASK_HALF 0x0000FFFF
#define MASK_WORD 0xFFFFFFFF

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
		case LUI:
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
	fprintf(*_debug_file, "Executing...\n");
	*_hilo = _operation(outp, in_a, in_b);
	std::cout << "Result of ALU operation was 0x" << *outp << std::endl;
}

hilo mips_alu::alu_add(uint32_t* out, const uint32_t* a, const uint32_t* b){
	if(((signed)*b>0 && (signed)*a>INT32_MAX-(signed)*b) ||
	   ((signed)*b<0 && (signed)*a<INT32_MIN-(signed)*b))
		throw mips_ExceptionArithmeticOverflow;
	else
		*out = (signed)*a+(signed)*b;
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
hilo mips_alu::alu_divide(uint32_t*, const uint32_t* a, const uint32_t* b){
	if( (signed)*b == 0 )	//Undefined result
		return {};
	else
		return {
			(uint32_t)( ((signed)*a) / ((signed)*b) ),
			(uint32_t)( ((signed)*a) % ((signed)*b) )
		};
}
hilo mips_alu::alu_divideu(uint32_t*, const uint32_t* a, const uint32_t* b){
	if( (signed)*b == 0 )	//Undefined result
		return {};
	else
		return {
			(uint32_t)( *a / *b ),
			(uint32_t)( *a % *b )
		};
}
hilo mips_alu::alu_or(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*a)|(*b);
	return {};
}
hilo mips_alu::alu_multiply(uint32_t*, const uint32_t* a, const uint32_t* b){
	int64_t p = ((signed)*a)*((signed)*b);
	return {
		(uint32_t)( (p&0xFFFFFFFF00000000 ) >> 32 ),
		(uint32_t)( (p&0x00000000FFFFFFFF ) )
	};
}
hilo mips_alu::alu_multiplyu(uint32_t*, const uint32_t* a, const uint32_t* b){
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
mips_cpu::mips_cpu(mips_mem* mem, FILE* f, debug_level dl) :
r(), _alu(&_alu_out, &_alu_in_a, &_alu_in_b, &_alu_hilo, &_debug_file, &_debug),
_pc(&_npc, &_debug_file, &_debug), _npc(4, &_debug_file, &_debug),
_hi(&_debug_file, &_debug), _lo(&_debug_file, &_debug),
_ir(&_debug_file, &_debug), _lmd(&_debug_file, &_debug),
_stage(IF), _mem_ptr(mem), _debug(dl), _debug_file(f){};

void mips_cpu::reset(void){
	
	//Zero registers
	for(int i=0; i<MIPS_NUM_REG; ++i)
		r[i].value(0);
	
	_pc.internal_set(0);
	_npc.internal_set(4);
	_ir.internal_set(0);
	_hi.internal_set(0);
	_lo.internal_set(0);
	
	fprintf(_debug_file, "CPU was reset.\n\n");
}

void mips_cpu::step(void){
	
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
	fprintf(_debug_file, "Fetching instruction...\n");
	//fprintf(_debug_file, "Fetching instruction...\n");
	uint32_t word = readWord(pc());
	_ir.internal_set(word);
	fprintf(_debug_file, "Fetched 0x%X.\n", word);
}
void mips_cpu::match(Instruction& ir, unsigned i){
	fprintf(_debug_file, "Matched: %s.\n", mipsInstruction[i].mnem);
	ir.setDecoded((mips_asm)i, mipsInstruction[i].type);
	_irDecoded = &ir;
	
	_alu.setOperation((mips_asm)i);
	return;
}
void mips_cpu::decode(){
	fprintf(_debug_file, "Decoding...\n");
	Instruction *ir = new Instruction(_ir.value());
	
	for(unsigned i=0; i<NUM_INSTR; ++i){
		
		if( mipsInstruction[i].opco == ir->opcode() ){
			switch( mipsInstruction[i].type ){
				case RType:
					if( mipsInstruction[i].func == ir->function()
					   /*Strictly following spec, shift field should be '00000'b */
					   ){
						match(*ir, i);
						return;
					}
					else
						break;
				case IType:
					if( mipsInstruction[i].func == FIELD_NOT_EXIST
					   || mipsInstruction[i].func == ir->regT() ){
						match(*ir, i);
						return;
					}
					else
						break;
				case JType:
					if( mipsInstruction[i].func == FIELD_NOT_EXIST
					   /*Strictly following spec, shift field should be '00000'b */
					   ){
						match(*ir, i);
						return;
					}
					else
						break;
			}
		}
		/*
		 if(	((uint32_t)mipsInstruction[i].opco == ir->opcode()) &&
		 
		 ((	(uint32_t)mipsInstruction[i].type == RType	//Match RType
		 &&	(uint32_t)mipsInstruction[i].func == ir->function()
		 // Strictly following spec, shift field should be '00000'b
		 )//&&	( shiftError == mips_InternalError ) )
		 
		 ||	(	(uint32_t)mipsInstruction[i].type != RType	//Match JType, most IType
		 &&	(_fields)mipsInstruction[i].func == FIELD_NOT_EXIST)
		 
		 ||	(	(uint32_t)mipsInstruction[i].type == IType	//Match IType branches
		 &&	(uint32_t)mipsInstruction[i].func == ir->regT()
		 // Strictly following spec, shift field should be '00000'b
		 )//&&	( shiftError == mips_InternalError ) )
		 
		 )){
		 
			fprintf(_debug_file, "Matched: " << mipsInstruction[i].mnem << std::endl;
			ir->setDecoded((mips_asm)i, mipsInstruction[i].type);
			_irDecoded = ir;
			
			_alu.setOperation((mips_asm)i);
			return;
		 }
		 */
	}
	
	/* BUG: Sometimes ADD r1,r2,r3 (0xFF7F00) comes here. Mostly it doesn't. Wtf. */
	//no match
	throw mips_ExceptionInvalidInstruction;
}

void mips_cpu::fetchRegs(uint32_t* aluInA, uint32_t* aluInB){
	fprintf(_debug_file, "Fetching registers...\n");
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
					*aluInB  = r[0].value();
					break;

				case BEQ:
				case BNE:
					*aluInB  = r[ _irDecoded->regT() ].value();
					break;

				case LUI:	//pointless sign extending on 32bit system
					*aluInA = 16;
					//case SLTIU:
				case ANDI:
				case ORI:
				case XORI:
					isSigned = false;
				default:
					*aluInB	 = isSigned
					? signExtend( (uint16_t)_irDecoded->immediate() )
					: _irDecoded->immediate();
			}
			break;

		case JType:
			*aluInA = pc()&0xf0000000;
			*aluInB = _irDecoded->target()<<2;
			break;
	}
	
	fprintf(_debug_file, "Set ALU input A to: 0x%X.\n", *aluInA);
	fprintf(_debug_file, "Set ALU input B to: 0x%X.\n", *aluInB);
}

bool mips_cpu::accessMem(const uint32_t* aluOut){
	fprintf(_debug_file, "Accessing memory...\n");
	
	switch(_irDecoded->mnemonic()){
			//branch
		case BEQ:
			if((signed)*aluOut==0){
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;
			
		case BGEZ:
			if((signed)*aluOut>=0){
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;
			
		case BGEZAL:
			if((signed)*aluOut>=0){
				link();
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;
			
		case BGTZ:
			if((signed)*aluOut>0){
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;
		case BLEZ:
			if((signed)*aluOut<=0){
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;

		case BLTZ:
			if((signed)*aluOut<0){
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;
			
		case BLTZAL:
			if((signed)*aluOut<0){
				link();
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;
			
		case BNE:
			if((signed)*aluOut!=0){
				fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
				uint32_t tmp = _npc.value();
				_pc.advance();
				_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
			} else{
				fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
				_pc.advance();
			}
			//branch has no WB stage
			return false;
			
		case JAL:
			link();
		case J:
		case JR:
			fprintf(_debug_file, "Jumping to 0x%X next cycle.\n", *aluOut);
			_pc.advance();
			_npc.internal_set(*aluOut);
			//branch has no WB stage
			return false;
			
			
			//load
		case LB:
			_lmd.value( signExtend(readByte(*aluOut)) );
			break;
		case LBU:
			_lmd.value( readByte(*aluOut) );
			break;
		case LW:
			_lmd.value( readWord(*aluOut) );
			break;
		case LWL:
			_lmd.value( (r[_irDecoded->regT()].value() & MASK_HALF)
					   | readHalf(*aluOut, true)<<16);
			break;
		case LWR:
			_lmd.value( (r[_irDecoded->regT()].value() & (MASK_HALF<<16))
					   | readHalf(*aluOut - 1, true));
			break;
			
			//store
		case SB:
			writeByte(*aluOut, r[_irDecoded->regT()].value());
			break;
		case SH:
			writeHalf(*aluOut, r[_irDecoded->regT()].value());
			break;
		case SW:
			writeWord(*aluOut, r[_irDecoded->regT()].value());
			break;
			
		default:;
	}
	return true;
}

void mips_cpu::writeBack(const uint32_t* aluOut){
	fprintf(_debug_file, "Writing back result...\n");
	
	switch(_irDecoded->type()){
		case RType:
			switch(_irDecoded->mnemonic()){

				case SLTU:// if rs<rt, but alu does rs-rt, hence:
					r[ _irDecoded->regD() ].value((unsigned)*aluOut ? 0 : 1);
					break;
				case SLT:
					r[ _irDecoded->regD() ].value((signed)*aluOut<0 ? 1 : 0);
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
	uint8_t align = addr%4;
	uint8_t ret[4];
	mips_error e = mips_mem_read((mips_mem_h)_mem_ptr, addr-align, 4, ret);
	if(e == mips_Success){
		uint8_t byte = ret[align];
		fprintf(_debug_file, "Read 0x%X from 0x%X.\n", byte, addr);
		return byte;
	}
	else{
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
}

void mips_cpu::writeByte(uint32_t addr, uint8_t data){
	uint8_t align = addr%4;
	uint8_t buf[4];
	mips_error e = mips_mem_read((mips_mem_h)_mem_ptr, addr-align, 4, buf);
	if(e != mips_Success){
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	buf[align] = data;
	mips_mem_write((mips_mem_h)_mem_ptr, addr-align, 4, buf);
	if(e != mips_Success){
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	else
		fprintf(_debug_file, "Wrote 0x%X to 0x%X.\n", (uint16_t)data, addr);
}

uint16_t mips_cpu::readHalf(uint32_t addr, bool allowUnaligned){
	uint8_t align = addr%4;
	if(!allowUnaligned && addr%2){
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw mips_ExceptionInvalidAlignment;
	}
	uint8_t ret[align<3 ? 4 : 8];
	mips_error e = mips_mem_read((mips_mem_h)_mem_ptr, addr-align, align<3 ? 4 : 8, ret);
	if(e == mips_Success){
		uint16_t half = (ret[align]<<8)|ret[align+1];
		fprintf(_debug_file, "Read 0x%X from 0x%X.\n", half, addr);
		return half;
	}
	else{
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
}

void mips_cpu::writeHalf(uint32_t addr, uint16_t data, bool allowUnaligned){
	uint8_t align = addr%4;
	if(!allowUnaligned && addr%2){
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw mips_ExceptionInvalidAlignment;
	}
	uint8_t buf[align<3 ? 4 : 8];
	mips_error e = mips_mem_read((mips_mem_h)_mem_ptr, addr-align, align<3 ? 4 : 8, buf);
	if(e != mips_Success)
		throw e;
	buf[align] = data>>8;
	buf[align+1] = data&MASK_08b;
	mips_mem_write((mips_mem_h)_mem_ptr, addr-align, align<3 ? 4 : 8, buf);
	if(e != mips_Success){
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	else
		fprintf(_debug_file, "Wrote 0x%X to 0x%X.\n", data, addr);
}

uint32_t mips_cpu::readWord(uint32_t addr){
	uint8_t buf[4];
	mips_error e = mips_mem_read((mips_mem_h)_mem_ptr, addr, 4, buf);
	if(e != mips_Success){
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	
	uint32_t word = buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3];
	
	fprintf(_debug_file, "Read 0x%X from 0x%X.\n", word, addr);
	return word;
}

void mips_cpu::writeWord(uint32_t addr, uint32_t data){
	uint8_t buf[4];
	for(unsigned i=0; i<4; ++i)
		buf[3-i] = (data >> (i*8))&0x000000FF;
	mips_error e = mips_mem_write((mips_mem_h)_mem_ptr, addr, 4, buf);
	if(e != mips_Success){
		fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	else
		fprintf(_debug_file, "Wrote 0x%X to 0x%X.\n", data, addr);
}

void mips_cpu::link(void){
	fprintf(_debug_file, "Linking 0x%X.\n", _npc.value()+4);
	r[31].value(_npc.value()+4);
}

uint32_t mips_cpu::signExtend(uint8_t byte){
	fprintf(_debug_file, "Sign extending byte: 0x%X.\n", byte);
	if( (byte&(MASK_BYTE>>1)) != byte ){
		fprintf(_debug_file, "--------Padding 1's: 0x%X.\n", (byte|~MASK_BYTE));
		return byte|~MASK_BYTE;
	} else {
		fprintf(_debug_file, "--------Padding 0's: 0x%X.\n", byte);
		return byte;
	};
}

uint32_t mips_cpu::signExtend(uint16_t half){
	fprintf(_debug_file, "Sign extending halfword: 0x%X.\n", half);
	if( (half&(MASK_HALF>>1)) != half ){
		fprintf(_debug_file, "------------Padding 1's: 0x%X.\n", (half|~MASK_HALF));
		return half|~MASK_HALF;
	} else {
		fprintf(_debug_file, "------------Padding 0's: 0x%X.\n", half);
		return half;
	};
}

void mips_cpu::setDebug(FILE* fptr, debug_level level){
	_debug = level;
	_debug_file = fptr;
}
//
