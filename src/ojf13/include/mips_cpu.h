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

#if DEBUG==1	//Xcode doesn't want me to put ojf13/ on the path..
#include "mips_instr.h"
#include "mips_cpu_mem.h"
#else
#include "include/mips_instr.h"
#include "include/mips_cpu_mem.h"
#endif

typedef enum _mips_cpu_stage{
	IF	=0,
	ID	=1,
	EX	=2,
	MEM	=3,
	WB	=4
} mips_cpu_stage;

struct mips_register{
public:
	mips_register(void);
	mips_register(bool);
	
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
	mips_reg_sp(void);

	void internal_set(uint32_t);
	
private:
};

struct mips_reg_pc: mips_reg_sp{
public:
	mips_reg_pc(mips_reg_sp*);
	
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
	mips_alu(uint32_t*, const uint32_t*, const uint32_t*, hilo*);
	
	void execute(void) const;
	void setOperation(mips_asm);
	
	uint32_t*	outp;
	const uint32_t* in_a;
	const uint32_t* in_b;
	
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
	static hilo alu_subtractu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_subtractnowrap(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_shiftright(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_shiftrightu(uint32_t*, const uint32_t*, const uint32_t*);
	static hilo alu_xor(uint32_t*, const uint32_t*, const uint32_t*);
};

struct mips_cpu{
public:
	mips_cpu(mips_mem*);
	
	void reset(void);
	void step(void);
	//make sure we know "we are the CPU" when setting
	void internal_pc_set(uint32_t);
	uint32_t pc(void) const;
	
	mips_regset_gp	r;
	
protected:
	void fetchInstr(void);
	void decode(void);
	void fetchRegs(uint32_t*, uint32_t*);
	bool accessMem(const uint32_t*);
	void writeBack(const uint32_t*);
	
	void link(void);
	uint32_t signExtendImdt(uint16_t);
	
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
	uint32_t		_alu_in_a;
	uint32_t		_alu_in_b;
	uint32_t		_alu_out;
	hilo			_alu_hilo;
	Instruction*	_irDecoded;
};
