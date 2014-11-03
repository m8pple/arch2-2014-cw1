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
mips_register::mips_register(uint32_t v, bool isZero) : _value(v), _isZero(isZero){};

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
mips_reg_sp::mips_reg_sp(FILE** f, const debug_level* dl) : mips_register(false), _debug(dl), _debug_file(f){};
mips_reg_sp::mips_reg_sp(uint32_t v, FILE** f, const debug_level* dl) : mips_register(v, false), _debug(dl), _debug_file(f){};

void mips_reg_sp::internal_set(uint32_t iVal){
	_value = iVal;
}
//

/*
 * MIPS PC register
 */
mips_reg_pc::mips_reg_pc(mips_reg_sp* npc, FILE** f, const debug_level* dl) : mips_reg_sp(f, dl), _npc(npc){
	_value = 0;
	_npc->internal_set(4);
};

void mips_reg_pc::advance(void){
	fprintf(*_debug_file, "Advancing PC to NPC (0x%X).\n", _npc->value());
	internal_set(_npc->value());
	_npc->internal_set(_npc->value()+4);
}
//

/*
 * MIPS ALU
 */
mips_alu::mips_alu(uint32_t* out, const uint32_t* a, const uint32_t* b, hilo* hilo, FILE** f, const debug_level* dl) :
outp(out), in_a(a), in_b(b), _hilo(hilo), _debug_file(f), _debug(dl){};

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
			_operation = alu_subtractnoex;
			break;
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
		case SLT:
		case SLTI:
			_operation = alu_subtractnoex;
			break;
		case SLTIU:
		case SLTU:
			_operation = alu_subtractnoexu;
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
	fprintf(*_debug_file, "Result of ALU operation was 0x%X.\n", *outp);
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
	if(((signed)*b>0 && (signed)*a<INT32_MIN+(signed)*b) ||
	   ((signed)*b<0 && (signed)*a>INT32_MAX-(signed)*b))
		throw mips_ExceptionArithmeticOverflow;
	else
		*out = (signed)*a-(signed)*b;
	return {};
}
hilo mips_alu::alu_subtractnoex(uint32_t* out, const uint32_t* a, const uint32_t* b){
	//nasty hack to fix overflow issues.. :/
	*out = (signed)*a < (signed)*b ? -1 : (signed)*a == (signed)*b ? 0 : 1;
	/*
	 int64_t p = (signed)(*a) - (signed)(*b);
	 if( p>INT32_MAX )
		*out = (uint32_t)INT32_MAX;
	 else if( p<INT32_MIN )
		*out = (uint32_t)INT32_MIN;
	 */
	return {};
}
hilo mips_alu::alu_subtractu(uint32_t* out, const uint32_t* a, const uint32_t* b){
	*out = (*a)-(*b);
	return {};
}
hilo mips_alu::alu_subtractnoexu(uint32_t* out, const uint32_t* a, const uint32_t* b){
	//ugh
	*out = *a < *b ? -1 : *a == *b ? 0 : 1;
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
	for(unsigned i=0; i<MIPS_NUM_REG; ++i)
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
	fetchRegs();
	
	_stage = EX;
	//magic
	try{
		_alu.execute();
	} catch(mips_error e){
		_pc.advance();
		throw e;
	}
	
	_stage = MEM;
	// bit less magic
	bool wb = accessMem();
	
	if(wb){
		_stage = WB;
		// more magic
		writeBack();
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
	if( _debug > ERROR )
		fprintf(_debug_file, "Decoding...\n");
	
	Instruction *ir = new Instruction(_ir.value());
	
	for(unsigned i=0; i<NUM_INSTR; ++i){
		//For each matching opcode, determine if the loaded instruction
		//	matches that expected format.
		
		if( mipsInstruction[i].opco == ir->opcode() ){
			switch( mipsInstruction[i].type ){
				case RType:
					if( mipsInstruction[i].func == ir->function()
					   //Strictly following spec, shift field should be '00000'b
					   //	unless shift instr, in which case rs should be '00000'b.
					   && ( ir->shift() == 0
							|| ((ir->mnemonic() == SRL
								 || ir->mnemonic() == SRA
								 || ir->mnemonic() == SLL
								) && ir->regS() == 0) )
					   
					   //Multiplication/division should have zero rd
					   && ( (ir->mnemonic() != MULT && ir->mnemonic() != MULTU
							 &&	ir->mnemonic() != DIV && ir->mnemonic() != DIVU)
							|| (ir->regD() == 0) )
					   
					   //Move from lo/hi should have zero rs/rt
					   && ( (ir->mnemonic() != MFLO && ir->mnemonic() != MFHI)
							|| (ir->regS() == 0 && ir->regT() == 0) )
					   ){
						match(*ir, i);
						return;
					}
					else
						break;

				case IType:
					if( (	mipsInstruction[i].func == FIELD_NOT_EXIST
						 || mipsInstruction[i].func == ir->regT() )
					   
					   //Load upper immediate should have zero rs
					   && (	(ir->mnemonic() != LUI)
						   || ir->regS() == 0 )
					   ){
						match(*ir, i);
						return;
					}
					else
						break;

				case JType:
					if( mipsInstruction[i].func == FIELD_NOT_EXIST ){
						match(*ir, i);
						return;
					}
					else
						break;
			}
		}
	}

	//no match
	throw mips_ExceptionInvalidInstruction;
}

void mips_cpu::fetchRegs(void){
	if( _debug > ERROR )
		fprintf(_debug_file, "Fetching registers...\n");
	
	mips_instr_type type = _irDecoded->type();
	bool isSigned = true;

	//ALU inputs
	switch(type){
		case RType:
			_alu_in_b = r[ _irDecoded->regT() ].value();
			switch(_irDecoded->mnemonic()){
				case SLL:
				case SRL:
				case SRA:
					_alu_in_a = _irDecoded->shift();
					break;

					//MFHI/LO OR respective with 0 (rt is all zeroes)
				case MFHI:
					_alu_in_a = _hi.value();
					break;
				case MFLO:
					_alu_in_a = _lo.value();
					break;

				default:
					_alu_in_a = r[ _irDecoded->regS() ].value();
			}
			break;

		case IType:
			_alu_in_a = r[ _irDecoded->regS() ].value();
			switch(_irDecoded->mnemonic()){
				case BGEZ:
				case BGEZAL:
				case BGTZ:
				case BLEZ:
				case BLTZ:
				case BLTZAL:
					_alu_in_b  = r[0].value();
					break;

				case BEQ:
				case BNE:
					_alu_in_b  = r[ _irDecoded->regT() ].value();
					break;

				case LUI:	//pointless sign extending on 32bit system
					_alu_in_a = 16;
				case ANDI:
				case ORI:
				case XORI:
					isSigned = false;
				default:
					_alu_in_b	 = isSigned
					? signExtend( (uint16_t)_irDecoded->immediate() )
					: _irDecoded->immediate();
			}
			break;

		case JType:
			_alu_in_a = pc()&0xf0000000;
			_alu_in_b = _irDecoded->target()<<2;
			break;
	}
	
	if( _debug > ERROR ){
		fprintf(_debug_file, "Set ALU input A to: 0x%X.\n", _alu_in_a);
		fprintf(_debug_file, "Set ALU input B to: 0x%X.\n", _alu_in_b);
	}
}

bool mips_cpu::branch(bool cond, bool doLink){
	if( cond ){
		if( doLink )
			link();
		if( _debug > ERROR )
			fprintf(_debug_file, "Condition met. Taking branch next cycle.\n");
		uint32_t tmp = _npc.value();
		_pc.advance();
		_npc.internal_set(tmp+(_irDecoded->immediate()<<2));
	} else{
		if( _debug > ERROR )
			fprintf(_debug_file, "Condition not met. Branch will not be taken.\n");
		_pc.advance();
	}
	//branch has no WB stage
	return false;
}

bool mips_cpu::accessMem(void){
	if( _debug > ERROR )
		fprintf(_debug_file, "Accessing memory...\n");
	
	switch(_irDecoded->mnemonic()){
			//branch
		case BEQ:
			return branch( _alu_out == 0 );

		case BGEZ:
			return branch( (signed)_alu_out >= 0 );

		case BGEZAL:
			return branch( (signed)_alu_out >= 0, true);

		case BGTZ:
			return branch( (signed)_alu_out > 0);

		case BLEZ:
			return branch( (signed)_alu_out <= 0);

		case BLTZ:
			return branch( (signed)_alu_out < 0);
			
		case BLTZAL:
			return branch( (signed)_alu_out < 0, true);
			
		case BNE:
			return branch( (signed)_alu_out != 0);

		case JAL:
			link();
		case J:
		case JR:
			if( _debug > ERROR )
				fprintf(_debug_file, "Jumping to 0x%X next cycle.\n", _alu_out);
			_pc.advance();
			_npc.internal_set(_alu_out);
			//branch has no WB stage
			return false;
			
			
			//load
		case LB:
			_lmd.value( signExtend(readByte(_alu_out)) );
			break;
		case LBU:
			_lmd.value( readByte(_alu_out) );
			break;
		case LW:
			_lmd.value( readWord(_alu_out) );
			break;
		case LWL:
			_lmd.value( (r[_irDecoded->regT()].value() & MASK_HALF)
					   | readHalf(_alu_out, true)<<16);
			break;
		case LWR:
			_lmd.value( (r[_irDecoded->regT()].value() & (MASK_HALF<<16))
					   | readHalf(_alu_out - 1, true));
			break;
			
			//store
		case SB:
			writeByte(_alu_out, r[_irDecoded->regT()].value());
			break;
		case SH:
			writeHalf(_alu_out, r[_irDecoded->regT()].value());
			break;
		case SW:
			writeWord(_alu_out, r[_irDecoded->regT()].value());
			break;
			
		default:;
	}
	return true;
}

void mips_cpu::writeBack(void){
	if( _debug > ERROR )
		fprintf(_debug_file, "Writing back result...\n");
	
	switch(_irDecoded->type()){
		case RType:
			switch(_irDecoded->mnemonic()){

				case SLTU:// if rs<rt, but alu does rs-rt, hence:
					r[ _irDecoded->regD() ].value((signed)_alu_out<0 ? 1 : 0);
					break;
				case SLT:
					r[ _irDecoded->regD() ].value((signed)_alu_out<0 ? 1 : 0);
					break;
					
				case DIV:
				case DIVU:
				case MULT:
				case MULTU:
					_hi.internal_set(_alu_hilo.hi);
					_lo.internal_set(_alu_hilo.lo);
					break;
					
				default:
					r[ _irDecoded->regD() ].value(_alu_out);
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
					r[ _irDecoded->regT() ].value((signed)_alu_out<0 ? 1 : 0);
					break;
				case SLTI:
					r[ _irDecoded->regT() ].value((signed)_alu_out<0 ? 1 : 0);
					break;
					
				default:
					r[ _irDecoded->regT() ].value(_alu_out);
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
		if( _debug > ERROR )
			fprintf(_debug_file, "Read 0x%X from 0x%X.\n", byte, addr);
		return byte;
	}
	else{
		if( _debug )
			fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
}

void mips_cpu::writeByte(uint32_t addr, uint8_t data){
	uint8_t align = addr%4;
	uint8_t buf[4];
	mips_error e = mips_mem_read((mips_mem_h)_mem_ptr, addr-align, 4, buf);
	if(e != mips_Success){
		if( _debug )
			fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	buf[align] = data;
	mips_mem_write((mips_mem_h)_mem_ptr, addr-align, 4, buf);
	if(e != mips_Success){
		if( _debug )
			fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	else if( _debug > ERROR )
		fprintf(_debug_file, "Wrote 0x%X to 0x%X.\n", (uint16_t)data, addr);
}

uint16_t mips_cpu::readHalf(uint32_t addr, bool allowUnaligned){
	uint8_t align = addr%4;
	if(!allowUnaligned && addr%2){
		if( _debug )
			fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw mips_ExceptionInvalidAlignment;
	}
	uint8_t ret[align<3 ? 4 : 8];
	mips_error e = mips_mem_read((mips_mem_h)_mem_ptr, addr-align, align<3 ? 4 : 8, ret);
	if(e == mips_Success){
		uint16_t half = (ret[align]<<8)|ret[align+1];
		if( _debug > ERROR )
			fprintf(_debug_file, "Read 0x%X from 0x%X.\n", half, addr);
		return half;
	}
	else{
		if( _debug )
			fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
}

void mips_cpu::writeHalf(uint32_t addr, uint16_t data, bool allowUnaligned){
	uint8_t align = addr%4;
	if(!allowUnaligned && addr%2){
		if( _debug )
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
		if( _debug )
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
		if( _debug )
			fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	
	uint32_t word = buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3];
	
	if( _debug > ERROR )
		fprintf(_debug_file, "Read 0x%X from 0x%X.\n", word, addr);
	return word;
}

void mips_cpu::writeWord(uint32_t addr, uint32_t data){
	uint8_t buf[4];
	for(unsigned i=0; i<4; ++i)
		buf[3-i] = (data >> (i*8))&0x000000FF;
	mips_error e = mips_mem_write((mips_mem_h)_mem_ptr, addr, 4, buf);
	if(e != mips_Success){
		if( _debug )
			fprintf(_debug_file, "Error accessing mem[0x%X].\n", addr);
		throw e;
	}
	else
		if( _debug > ERROR )
			fprintf(_debug_file, "Wrote 0x%X to 0x%X.\n", data, addr);
}

void mips_cpu::link(void){
	if( _debug > ERROR )
		fprintf(_debug_file, "Linking 0x%X.\n", _npc.value()+4);
	r[31].value(_npc.value()+4);
}

uint32_t mips_cpu::signExtend(uint8_t byte){
	if( _debug > ERROR )
		fprintf(_debug_file, "Sign extending byte: 0x%X.\n", byte);
	if( (byte&(MASK_BYTE>>1)) != byte ){
		if( _debug > INFO )
			fprintf(_debug_file, "--------Padding 1's: 0x%X.\n", (byte|~MASK_BYTE));
		return byte|~MASK_BYTE;
	} else {
		if( _debug > INFO )
			fprintf(_debug_file, "--------Padding 0's: 0x%X.\n", byte);
		return byte;
	};
}

uint32_t mips_cpu::signExtend(uint16_t half){
	if( _debug > ERROR )
		fprintf(_debug_file, "Sign extending halfword: 0x%X.\n", half);
	if( (half&(MASK_HALF>>1)) != half ){
		if( _debug > INFO )
			fprintf(_debug_file, "------------Padding 1's: 0x%X.\n", (half|~MASK_HALF));
		return half|~MASK_HALF;
	} else {
		if( _debug > INFO )
			fprintf(_debug_file, "------------Padding 0's: 0x%X.\n", half);
		return half;
	};
}

void mips_cpu::setDebug(FILE* fptr, debug_level level){
	_debug = level;
	_debug_file = fptr;
}
//
