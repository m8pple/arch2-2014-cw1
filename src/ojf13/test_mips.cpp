#include "../../include/mips_test.h"
#include "include/test_mips.h"
#include <iostream>

MIPS_instr instrFromAsm(MIPS_asm mnem){
	return MIPS_instruction[mnem];
}

struct Instruction{
	//RType
	Instruction(MIPS_asm mnem,
				 unsigned sreg, unsigned treg, unsigned dreg,
				 unsigned shft){
		MIPS_instr instr = MIPS_instruction[mnem];
		if( instr.type == RType )
			Instruction(instr.opco, sreg, treg, dreg, shft, instr.func);
		else
			throw mips_ExceptionInvalidInstruction;
	}
	
	//IType
	Instruction(MIPS_asm mnem,
		  unsigned sreg, unsigned treg,
		  unsigned imdt){
		MIPS_instr instr = MIPS_instruction[mnem];
		if( instr.type == IType )
			Instruction(instr.opco, sreg, instr.func?instr.func:treg, imdt);
		else
			throw mips_ExceptionInvalidInstruction;
	}
	
	//JType
	Instruction(MIPS_asm mnem,
				unsigned trgt){
		MIPS_instr instr = MIPS_instruction[mnem];
		if( instr.type == JType )
			Instruction(instr.opco, trgt);
		else
			throw mips_ExceptionInvalidInstruction;
	}

private:
	//RType
	Instruction(unsigned opco,
				unsigned sreg, unsigned treg, unsigned dreg,
				unsigned shft, unsigned func) : _value(0){
		_value |= (opco & MASK_06b) << POS_OPCO;
		_value |= (sreg & MASK_05b) << POS_SREG;
		_value |= (treg & MASK_05b) << POS_TREG;
		_value |= (dreg & MASK_05b) << POS_DREG;
		_value |= (shft & MASK_05b) << POS_SHFT;
		_value |= (func & MASK_06b) << POS_FUNC;
	}
	//IType
	Instruction(unsigned opco,
		  unsigned sreg, unsigned treg,
		  unsigned imdt) : _value(0){
		_value |= (opco & MASK_06b) << POS_OPCO;
		_value |= (sreg & MASK_05b) << POS_SREG;
		_value |= (treg & MASK_05b) << POS_TREG;
		_value |= (imdt & MASK_16b) << POS_IMDT;
	}
	//JType
	Instruction(unsigned opco, unsigned trgt) : _value(0){
		_value |= (opco & MASK_06b) << POS_OPCO;
		_value |= (trgt & MASK_26b) << POS_TRGT;
	}
	uint32_t _value;
};

int main(){
    mips_mem_h mem = mips_mem_create_ram(1<<5, 4);
    mips_cpu_h cpu = mips_cpu_create(mem);
    
    mips_test_begin_suite();

    runTest(registerReset, cpu, mem);
    runTest(rTypeAnd, cpu, mem);
    
    mips_test_end_suite();
	
    mips_cpu_free(cpu);
    mips_mem_free(mem);
	return 0;
}
                       
void runTest(testResult(*test)(mips_cpu_h, mips_mem_h),
             mips_cpu_h cpu, mips_mem_h mem){

    testResult result = test(cpu, mem);
    mips_test_end_test(
                       mips_test_begin_test(result.instr),
                       result.passed,
                       result.desc);
    
}

testResult registerReset(mips_cpu_h cpu, mips_mem_h mem){
    mips_cpu_reset(cpu);
    uint32_t got;
    int correct;
    
    try{
        mips_cpu_get_pc(cpu, &got);
        correct = (got==0) ? 1 : 0;
        
        for(int i=0; i<32 && correct; ++i){
            mips_cpu_get_register(cpu, i, &got);
            correct |= (got==0);
        }
    } catch(mips_error) {
        correct = 0;
    }
    
    return {"<internal>", "Check all registers zeroed after reset.", correct};
}

testResult rTypeAnd(mips_cpu_h cpu, mips_mem_h mem){
	mips_cpu_set_register(cpu, 1, 0x00FF00F0);
	mips_cpu_set_register(cpu, 2, 0x000F0F0F);
	Instruction instr = Instruction(AND, 1, 2, 3, 0);
	
    mips_cpu_step(cpu);
    return {"AND", "Check several cases of register AND-ing.", 2};
}
