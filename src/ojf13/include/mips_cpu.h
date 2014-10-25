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
	
	void advance(int32_t);
	
private:
	mips_reg_sp* _npc;
};


typedef uint32_t (*ALU_OP)(uint32_t, uint32_t);

struct mips_alu{
public:
	mips_alu(uint32_t*, uint32_t*);
	
	void execute(uint32_t*) const;
	void setOperation(mips_asm);
	
	uint32_t* in_a;
	uint32_t* in_b;
	
	//Index in with mips_asm enum type
	const ALU_OP aluOp[NUM_INSTR] = {
		alu_add,		//ADD
		alu_add,		//ADDI is just different input
		alu_addu,		//ADDIU
		alu_addu,		//ADDU " "
		alu_and,		//AND
		alu_and,		//ANDI " "
		alu_addu,		//BEQ
		alu_addu,		//BGEZ
		alu_addu,		//BGEZAL
		alu_addu,		//BGTZ		branches all do aluout <- npc+imm
		alu_addu,		//BLEZ
		alu_addu,		//BLTZ
		alu_addu,		//BLTZAL
		alu_addu,		//BNE
		alu_divide,		//DIV
		alu_divideu,	//DIVU
		alu_or,			//J does PC<- PC[31..28] || trgt<<2
		alu_or,			//JAL " "
		alu_add,		//JR has treg:=0 so we do npc<-sreg+0
		alu_add,		//LB does treg <- mem[ sreg+imm ]
		alu_add,		//LBU " " (output not sign extended)
		alu_or,			//LUI does treg<- imm<<16 || 0x0
		alu_add,		//LW
		alu_add,		//LWL
		alu_add,		//LWR
		nullptr,		//MFHI
		nullptr,		//MFLO
		alu_multiply,	//MULT
		alu_multiplyu,	//MULTU
		alu_or,			//OR
		alu_or,			//ORI
		alu_add,		//SB as LB
		alu_add,		//SH
		alu_shiftleft,	//SLL
		alu_shiftleft,	//SLLV
		alu_sub,		//SLT
		alu_sub,		//SLTI
		alu_subu,		//SLTIU
		alu_subu,		//SLTU
		alu_shiftright,	//SRA
		alu_shiftrightu,//SRL
		alu_shiftrightu,//SRLV
		alu_sub,		//SUB
		alu_subu,		//SUBU
		alu_add,		//SW
		alu_xor,		//XOR
		alu_xor,		//XORI
	};
	
private:
	ALU_OP _operation;
	
	static uint32_t alu_add(uint32_t, uint32_t);
	static uint32_t alu_addu(uint32_t, uint32_t);
	static uint32_t alu_and(uint32_t, uint32_t);
	static uint32_t alu_divide(uint32_t, uint32_t);
	static uint32_t alu_divideu(uint32_t, uint32_t);
	static uint32_t alu_or(uint32_t, uint32_t);
	static uint32_t alu_multiply(uint32_t, uint32_t);
	static uint32_t alu_multiplyu(uint32_t, uint32_t);
	static uint32_t alu_shiftleft(uint32_t, uint32_t);
	static uint32_t alu_sub(uint32_t, uint32_t);
	static uint32_t alu_subu(uint32_t, uint32_t);
	static uint32_t alu_shiftright(uint32_t, uint32_t);
	static uint32_t alu_shiftrightu(uint32_t, uint32_t);
	static uint32_t alu_xor(uint32_t, uint32_t);
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
	void accessMem(const uint32_t*);
	
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
	Instruction*	_irDecoded;
};
