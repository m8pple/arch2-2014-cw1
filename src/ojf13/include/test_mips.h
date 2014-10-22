#ifndef MIPS_NUM_REG
#define MIPS_NUM_REG 32U
#endif

#define MASK_26b	0x03FFFFFF
#define MASK_16b	0x0000FFFF
#define MASK_06b    0x0000003F
#define MASK_05b    0x0000001F

#define POS_OPCO    27U
#define POS_SREG    21U
#define POS_TREG    16U
#define POS_DREG    11U
#define POS_SHFT	06U
#define POS_FUNC    00U

#define POS_IMDT	00U
#define POS_TRGT	00U

typedef enum _MIPS_asm{
	ADD=0,
	ADDI=1,
	ADDIU=2,
	ADDU=3,
	AND=4,
	ANDI=5,
	BEQ=6,
	BGEZ=7,
	BGEZAL=8,
	BGTZ=9,
	BLEZ=10,
	BLTZ=11,
	BLTZAL=12,
	BNE=13,
	DIV=14,
	DIVU=15,
	J=16,
	JAL=17,
	JR=18,
	LB=19,
	LBU=20,
	LUI=21,
	LW=22,
	LWL=23,
	LWR=24,
	MFHI=25,
	MFLO=26,
	MULT=27,
	MULTU=28,
	OR=29,
	ORI=30,
	SB=31,
	SH=32,
	SLL=33,
	SLLV=34,
	SLT=35,
	SLTI=36,
	SLTIU=37,
	SLTU=38,
	SRA=39,
	SRL=40,
	SRLV=41,
	SUB=42,
	SUBU=43,
	SW=44,
	XOR=45,
	XORI=46,
} MIPS_asm;

typedef enum _instrType{
	RType=0,
	IType=1,
	JType=2
} instrType;

struct MIPS_instr{
	instrType	type;
	unsigned	opco;
	unsigned	func;	//RType func, or IType branch treg
};

const MIPS_instr MIPS_instruction[47] = {
	{ RType, 0,							1<<5					},	//ADD
	{ IType, 1<<3,						NULL					},	//ADDI
	{ IType, 1<<3 | 1,					NULL					},	//ADDIU
	{ RType, 0,							1<<5 | 1				},	//ADDU
	{ RType, 0,							1<<5 | 1<<2				},	//AND
	{ IType, 1<<3 | 1<<2,				NULL					},	//ANDI
	{ IType, 1<<2,						NULL					},	//BEQ
	{ IType, 1,							1						},	//BGEZ
	{ IType, 1,							1<<4 | 1				},	//BGEZAL
	{ IType, 1<<2 | 1<<1 | 1,			0						},	//BGTZ
	{ IType, 1<<2 | 1<<1,				0						},	//BLEZ
	{ IType, 1,							0						},	//BLTZ
	{ IType, 1,							1<<4					},	//BLTZAL
	{ IType, 1<<2 | 1,					NULL					},	//BNE
	{ RType, 0,							1<<4 | 1<<3 | 1<<1		},	//DIV
	{ RType, 0,							1<<4 | 1<<3 | 1<<1 | 1	},	//DIVU
	{ JType, 1<<1,						NULL					},	//J
	{ JType, 1<<1 | 1,					NULL					},	//JAL
	{ RType, 0,							1<<3					},	//JR
	{ IType, 1<<5,						NULL					},	//LB
	{															},	//LBU
	{ IType, 1<<3 | 1<<2 | 1<<1 | 1,	NULL					},	//LUI
	{ IType, 1<<5 | 1<<1 | 1,			NULL					},	//LW
	{															},	//LWL
	{															},	//LWR
	{ RType, 0,							1<<4					},	//MFHI
	{ RType, 0,							1<<4 | 1<<1				},	//MFLO
	{ RType, 0,							1<<4 | 1<<3				},	//MULT
	{ RType, 0,							1<<4 | 1<<3 | 1			},	//MULTU
	{ RType, 0,							1<<5 | 1<<2 | 1			},	//OR
	{ IType, 1<<3 | 1<<2 | 1,			NULL					},	//ORI
	{ IType, 1<<5 | 1<<3,				NULL					},	//SB
	{															},	//SH
	{ RType, 0,							0						},	//SLL
	{ RType, 0,							1<<2					},	//SLLV
	{ RType, 0,							1<<5 | 1<<3 | 1<<1		},	//SLT
	{ IType, 1<<3 | 1<<1,				NULL					},	//SLTI
	{ IType, 1<<3 | 1<<1 | 1,			NULL					},	//SLTIU
	{ RType, 0,							1<<5 | 1<<3 | 1<<1 | 1	},	//SLTU
	{ RType, 0,							1<<1 | 1				},	//SRA
	{ RType, 0,							1<<1					},	//SRL
	{ RType, 0,							1<<2 | 1<<1				},	//SRLV
	{ RType, 0,							1<<5 | 1<<1				},	//SUB
	{ RType, 0,							1<<5 | 1<<1 | 1			},	//SUBU
	{ IType, 1<<5 | 1<<3 | 1<<1 | 1,	NULL					},	//SW
	{ RType, 0,							1<<5 | 1<<2 |1<<1		},	//XOR
	{ IType, 1<<3 | 1<<2 | 1<<1,		NULL					},	//XORI
};

/*typedef enum _MIPS_instr{
 "ADD"=0,
 "ADDI"=1,
 "ADDIU"=2,
 "ADDU"=3,
 "AND"=4,
 "ANDI"=5,
 "BEQ"=6,
 "BGEZ"=7,
 "BGEZAL"=8,
 "BGTZ"=9,
 "BLEZ"=10,
 "BLTZ"=11,
 "BLTZAL"=12,
 "BNE"=13,
 "DIV"=14,
 "DIVU"=15,
 "J"=16,
 "JAL"=17,
 "JR"=18,
 "LB"=19,
 "LBU"=20,
 "LUI"=21,
 "LW"=22,
 "LWL"=23,
 "LWR"=24,
 "MFHI"=25,
 "MFLO"=26,
 "MULT"=27,
 "MULTU"=28,
 "OR"=29,
 "ORI"=30,
 "SB"=31,
 "SH"=32,
 "SLL"=33,
 "SLLV"=34,
 "SLT"=35,
 "SLTI"=36,
 "SLTIU"=37,
 "SLTU"=38,
 "SRA"=39,
 "SRL"=40,
 "SRLV"=41,
 "SUB"=42,
 "SUBU"=43,
 "SW"=44,
 "XOR"=45,
 "XORI"=46,
 }MIPS_instr;*/

struct testResult{
    const char* instr;
    const char* desc;
    int passed;
};

void runTest(testResult test(mips_cpu_h, mips_mem_h), mips_cpu_h, mips_mem_h);
testResult registerReset(mips_cpu_h, mips_mem_h);
testResult rTypeAnd(mips_cpu_h, mips_mem_h);
