//
//  mips_instr.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#ifndef MIPS_NUM_REG
#define MIPS_NUM_REG 32U
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
	mips_register(bool isZero);
	
	uint32_t value(void) const;
	void value(uint32_t iVal);
	
protected:
	uint32_t _value;
	
private:
	bool _allowSet;
};

struct mips_gp_regs{
public:
	mips_gp_regs(void);
	
	mips_register& operator[](int idx);
	
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
	void internal_set(uint32_t iVal);
	
private:
};

struct mips_reg_pc: mips_reg_sp{
public:
	mips_reg_pc(void);
	
	void advance(void);
	
private:
};

struct mips_cpu{
public:
	mips_cpu(void);
	
	void reset(void);
	void step(void);
	//make sure we know "we are the CPU" when setting
	void internal_pc_set(uint32_t iVal);
	uint32_t pc(void) const;
	
	mips_gp_regs	r;
	
private:
	mips_reg_pc		_pc;
	mips_reg_sp		_hi;
	mips_reg_sp		_lo;
	mips_cpu_stage	_stage;
};
