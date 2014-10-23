//
//  mips_cpu_impl.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#ifndef MIPS_NUM_REG
#define MIPS_NUM_REG 32U
#endif

#include "./mips_instr.h"
#include "./mips_cpu_mem.h"

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
	bool _allowSet;
};

struct mips_regset_gp{
public:
	mips_regset_gp(void);
	
	mips_register& operator[](int);
	
private:
	mips_register _r0;
	mips_register _r[MIPS_NUM_REG-1];
};

struct mips_reg_sp: mips_register{
public:
	mips_reg_sp(void);
	
	using mips_register::value;
	//make sure we know "we are the CPU" when setting
	void value(uint32_t) const;
	void internal_set(uint32_t);
	
private:
};

struct mips_reg_pc: mips_reg_sp{
public:
	mips_reg_pc(mips_reg_sp*);
	
	void advance(int32_t);
	
private:
	mips_reg_sp* _npc;
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
	void fetchRegs(void);
	
	mips_reg_pc		_pc;
	mips_reg_sp		_npc;
	mips_reg_sp		_ir;
	mips_reg_sp		_hi;
	mips_reg_sp		_lo;
	mips_cpu_stage	_stage;
	
	mips_mem*		_mem_ptr;
	
private:
	mips_instr		_irDecoded;
};
