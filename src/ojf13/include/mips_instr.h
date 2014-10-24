//
//  mips_instr.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#ifndef MIPSim_mips_instr_h
#define MIPSim_mips_instr_h

#define NUM_INSTR	47U

#define MASK_26b	0x03FFFFFF
#define MASK_16b	0x0000FFFF
#define MASK_08b	0x000000FF
#define MASK_06b    0x0000003F
#define MASK_05b    0x0000001F

#define POS_OPCO    26U
#define POS_SREG    21U
#define POS_TREG    16U
#define POS_DREG    11U
#define POS_SHFT	06U
#define POS_FUNC    00U
#define POS_IMDT	00U
#define POS_TRGT	00U

#define MASK_OPCO	MASK_06b << POS_OPCO
#define MASK_SREG	MASK_05b << POS_SREG
#define MASK_TREG	MASK_05b << POS_TREG
#define MASK_DREG	MASK_05b << POS_DREG
#define MASK_SHFT	MASK_05b << POS_SHFT
#define MASK_FUNC	MASK_06b << POS_FUNC
#define MASK_IMDT	MASK_16b << POS_IMDT
#define MASK_TRGT	MASK_26b << POS_TRGT

#include "mips_core.h"

typedef enum _mips_asm{
	ADD		=0,
	ADDI	=1,
	ADDIU	=2,
	ADDU	=3,
	AND		=4,
	ANDI	=5,
	BEQ		=6,
	BGEZ	=7,
	BGEZAL	=8,
	BGTZ	=9,
	BLEZ	=10,
	BLTZ	=11,
	BLTZAL	=12,
	BNE		=13,
	DIV		=14,
	DIVU	=15,
	J		=16,
	JAL		=17,
	JR		=18,
	LB		=19,
	LBU		=20,
	LUI		=21,
	LW		=22,
	LWL		=23,
	LWR		=24,
	MFHI	=25,
	MFLO	=26,
	MULT	=27,
	MULTU	=28,
	OR		=29,
	ORI		=30,
	SB		=31,
	SH		=32,
	SLL		=33,
	SLLV	=34,
	SLT		=35,
	SLTI	=36,
	SLTIU	=37,
	SLTU	=38,
	SRA		=39,
	SRL		=40,
	SRLV	=41,
	SUB		=42,
	SUBU	=43,
	SW		=44,
	XOR		=45,
	XORI	=46,
} mips_asm;

typedef enum _instrType{
	RType=0,
	IType=1,
	JType=2
} mips_instr_type;

struct mips_instr{
	mips_instr_type	type;
	uint32_t		opco;
	int32_t			func;	//RType func, or IType branch treg -1 for none
};

typedef enum _fields{ FIELD_NOT_EXIST=-1 }fields;

const mips_instr mipsInstruction[NUM_INSTR] = {
	{ RType, 0,							1<<5					},	//ADD
	{ IType, 1<<3,						FIELD_NOT_EXIST			},	//ADDI
	{ IType, 1<<3 | 1,					FIELD_NOT_EXIST			},	//ADDIU
	{ RType, 0,							1<<5 | 1				},	//ADDU
	{ RType, 0,							1<<5 | 1<<2				},	//AND
	{ IType, 1<<3 | 1<<2,				FIELD_NOT_EXIST			},	//ANDI
	{ IType, 1<<2,						FIELD_NOT_EXIST			},	//BEQ
	{ IType, 1,							1						},	//BGEZ
	{ IType, 1,							1<<4 | 1				},	//BGEZAL
	{ IType, 1<<2 | 1<<1 | 1,			0						},	//BGTZ
	{ IType, 1<<2 | 1<<1,				0						},	//BLEZ
	{ IType, 1,							0						},	//BLTZ
	{ IType, 1,							1<<4					},	//BLTZAL
	{ IType, 1<<2 | 1,					FIELD_NOT_EXIST			},	//BNE
	{ RType, 0,							1<<4 | 1<<3 | 1<<1		},	//DIV
	{ RType, 0,							1<<4 | 1<<3 | 1<<1 | 1	},	//DIVU
	{ JType, 1<<1,						FIELD_NOT_EXIST			},	//J
	{ JType, 1<<1 | 1,					FIELD_NOT_EXIST			},	//JAL
	{ RType, 0,							1<<3					},	//JR
	{ IType, 1<<5,						FIELD_NOT_EXIST			},	//LB
	{ IType, 1<<5 | 1<<2,				FIELD_NOT_EXIST			},	//LBU
	{ IType, 1<<3 | 1<<2 | 1<<1 | 1,	FIELD_NOT_EXIST			},	//LUI
	{ IType, 1<<5 | 1<<1 | 1,			FIELD_NOT_EXIST			},	//LW
	{ IType, 1<<5 | 1<<1,				FIELD_NOT_EXIST			},	//LWL
	{ IType, 1<<5 | 1<<2 | 1<<1,		FIELD_NOT_EXIST			},	//LWR
	{ RType, 0,							1<<4					},	//MFHI
	{ RType, 0,							1<<4 | 1<<1				},	//MFLO
	{ RType, 0,							1<<4 | 1<<3				},	//MULT
	{ RType, 0,							1<<4 | 1<<3 | 1			},	//MULTU
	{ RType, 0,							1<<5 | 1<<2 | 1			},	//OR
	{ IType, 1<<3 | 1<<2 | 1,			FIELD_NOT_EXIST			},	//ORI
	{ IType, 1<<5 | 1<<3,				FIELD_NOT_EXIST			},	//SB
	{ IType, 1<<5 | 1<<3 | 1,			FIELD_NOT_EXIST			},	//SH
	{ RType, 0,							0						},	//SLL
	{ RType, 0,							1<<2					},	//SLLV
	{ RType, 0,							1<<5 | 1<<3 | 1<<1		},	//SLT
	{ IType, 1<<3 | 1<<1,				FIELD_NOT_EXIST			},	//SLTI
	{ IType, 1<<3 | 1<<1 | 1,			FIELD_NOT_EXIST			},	//SLTIU
	{ RType, 0,							1<<5 | 1<<3 | 1<<1 | 1	},	//SLTU
	{ RType, 0,							1<<1 | 1				},	//SRA
	{ RType, 0,							1<<1					},	//SRL
	{ RType, 0,							1<<2 | 1<<1				},	//SRLV
	{ RType, 0,							1<<5 | 1<<1				},	//SUB
	{ RType, 0,							1<<5 | 1<<1 | 1			},	//SUBU
	{ IType, 1<<5 | 1<<3 | 1<<1 | 1,	FIELD_NOT_EXIST			},	//SW
	{ RType, 0,							1<<5 | 1<<2 | 1<<1		},	//XOR
	{ IType, 1<<3 | 1<<2 | 1<<1,		FIELD_NOT_EXIST			}	//XORI
};

struct Instruction{
public:
	//Raw
	Instruction(uint32_t);
	//RType
	Instruction(mips_asm, uint32_t, uint32_t, uint32_t, uint32_t);
	//IType
	Instruction(mips_asm, uint32_t, uint32_t, uint32_t);
	//IType branch
	Instruction(mips_asm, uint32_t, uint32_t);
	//JType
	Instruction(mips_asm, uint32_t);
	
	uint8_t* bufferedVal(void) const;
	void setDecoded(const mips_asm, const mips_instr_type);

	mips_asm mnemonic(void) const;
	mips_instr_type type(void) const;
	uint32_t opcode(void) const;
	uint32_t regS(void) const;
	uint32_t regT(void) const;
	uint32_t regD(void) const;
	uint32_t shift(void) const;
	uint32_t function(void) const;
	uint32_t immediate(void) const;
	uint32_t target(void) const;
	
private:
	uint32_t		_value;
	
	mips_asm		_mnem;	//avoid searching array several times
	mips_instr_type	_type;
};

#endif	//defined(MIPSim_mips_instr_h)s
