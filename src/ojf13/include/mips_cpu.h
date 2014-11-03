//
//  mips_cpu_impl.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#ifndef MIPS_NUM_REG
#define MIPS_NUM_REG	32U
#endif

#include "mips_mem.h"
typedef mips_mem_h *mips_mem;

#if DEBUG==1	//Xcode doesn't want me to put ojf13/ on the path..
#include "mips_instr.h"
#else
#include "include/mips_instr.h"
#endif

typedef enum _mips_cpu_stage{
	IF	=0,
	ID	=1,
	EX	=2,
	MEM	=3,
	WB	=4
} mips_cpu_stage;

typedef enum _debug_level{
	NONE	=0,
	ERROR	=1,
	INFO	=2,
	VERBOSE	=3
} debug_level;

struct mips_register{
public:
	mips_register(void);
	mips_register(bool);
	mips_register(uint32_t, bool);
	
	uint32_t value(void) const;
	void value(uint32_t);
	
protected:
	uint32_t _value;
	
private:
	bool _isZero;
};

struct mips_regset_gp{
public:
	mips_regset_gp(void);
	
	mips_register& operator[](unsigned);
	
private:
	mips_register _r0;
	mips_register _r[MIPS_NUM_REG-1];
};

struct mips_reg_sp: mips_register{
public:
	mips_reg_sp(FILE**, const debug_level*);
	mips_reg_sp(uint32_t, FILE**, const debug_level*);

	void internal_set(uint32_t);
	
protected:
	const debug_level* _debug;
	FILE** _debug_file;
	
private:
};

struct mips_reg_pc: mips_reg_sp{
public:
	mips_reg_pc(mips_reg_sp*, FILE**, const debug_level*);
	
	void advance(void);

private:
	mips_reg_sp* _npc;
};

struct hilo{
	uint32_t hi;
	uint32_t lo;
};

struct mips_alu{
public:
	mips_alu(uint32_t*, const uint32_t*, const uint32_t*, hilo*, FILE**, const debug_level*);
	
	void execute(void) const;
	void setOperation(mips_asm);
	
	uint32_t*	outp;
	const uint32_t* in_a;
	const uint32_t* in_b;
	
protected:
	const debug_level* _debug;
	FILE** _debug_file;
	
private:
	hilo* _hilo;
	
	hilo (*_operation)(uint32_t*, const uint32_t*, const uint32_t*);
	
	static hilo alu_add(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_addu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_and(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_divide(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_divideu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_or(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_multiply(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_multiplyu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_shiftleft(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_subtract(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_subtractnoex(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_subtractu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_subtractnoexu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_shiftright(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_shiftrightu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_xor(uint32_t*, const uint32_t*, const uint32_t*);
};

struct mips_cpu{
public:
	mips_cpu(mips_mem*, FILE* =stdout, debug_level=ERROR);
	
	void reset(void);
	void step(void);
	//make sure we know "we are the CPU" when setting
	void internal_pc_set(uint32_t);
	uint32_t pc(void) const;
	void setDebug(FILE*, debug_level);
	
	mips_regset_gp	r;
	
protected:
	void fetchInstr(void);
	void decode(void);
	void fetchRegs(void);
	bool accessMem(void);
	void writeBack(void);
	
	uint8_t readByte(uint32_t);
	void writeByte(uint32_t, uint8_t);
	uint16_t readHalf(uint32_t, bool=false);
	void writeHalf(uint32_t, uint16_t, bool=false);
	uint32_t readWord(uint32_t);
	void writeWord(uint32_t, uint32_t);
	
	void link(void);
	uint32_t signExtend(uint8_t);
	uint32_t signExtend(uint16_t);
	
	mips_alu		_alu;
	
	mips_reg_pc		_pc;
	mips_reg_sp		_npc;
	
	mips_reg_sp		_ir;
	mips_reg_sp		_hi;
	mips_reg_sp		_lo;
	mips_reg_sp		_lmd;

	mips_cpu_stage	_stage;
	
	mips_mem*		_mem_ptr;
	
private:
	void match(Instruction&, unsigned);
	bool branch(bool, bool=false);
	
	uint32_t		_alu_in_a;
	uint32_t		_alu_in_b;
	uint32_t		_alu_out;
	hilo			_alu_hilo;
	Instruction*	_irDecoded;
	debug_level		_debug;
	FILE*			_debug_file;
};
