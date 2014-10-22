#include "../../include/mips_test.h"
#include "include/test_mips.h"
#include "include/mips_instr.h"
#include <iostream>

MIPS_instr instrFromAsm(MIPS_asm mnem){
	return MIPS_instruction[mnem];
}

struct Instruction{
public:
	//RType
	Instruction(MIPS_asm mnem,
				unsigned sreg, unsigned treg, unsigned dreg,
				unsigned shft) :
	Instruction(MIPS_instruction[mnem].opco,
				sreg, treg, dreg,
				shft, MIPS_instruction[mnem].func){
		
		_mnem=mnem;
		if( MIPS_instruction[mnem].type != RType )
			throw mips_ExceptionInvalidInstruction;
	}
	
	//IType
	Instruction(MIPS_asm mnem,
		  unsigned sreg, unsigned treg,
		  unsigned imdt) :
	Instruction(MIPS_instruction[mnem].opco,
				sreg, MIPS_instruction[mnem].func?MIPS_instruction[mnem].func:treg,
				imdt){
		
		_mnem=mnem;
		if( MIPS_instruction[mnem].type != IType )
			throw mips_ExceptionInvalidInstruction;
	}
	
	//JType
	Instruction(MIPS_asm mnem,
				unsigned trgt) :
	Instruction(MIPS_instruction[mnem].opco, trgt){
		
		_mnem=mnem;
		if( MIPS_instruction[mnem].type != JType )
			throw mips_ExceptionInvalidInstruction;
	}
	
	uint8_t* bytes() const{
		uint8_t *obuf = new uint8_t[4];
		for(int i=0; i<4; ++i)
			obuf[i] = (_value & (MASK_08b << 8*(3-i))) >> 8*(3-i);
		return obuf;
	}
	
	uint32_t value() const{
		return _value;
	}
	
	MIPS_asm mnem() const{
		return _mnem;
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
		
		std::cout << "Creating: " << std::hex << (int)value() << std::endl;
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
	MIPS_asm _mnem;
};

int main(){
    mips_mem_h mem = mips_mem_create_ram(1<<5, 4);
    mips_cpu_h cpu = mips_cpu_create(mem);
    
    mips_test_begin_suite();

	runTest(registerReset, cpu, mem);
	runTest(memoryIO, cpu, mem);
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

testResult memoryIO(mips_cpu_h cpu, mips_mem_h mem){
	Instruction instr = Instruction(AND, 0, 0, 0, 0);
	mips_mem_write(mem, 0x0, 4, instr.bytes());
	
	uint8_t got[4];
	mips_mem_read(mem, 0x0, 4, got);
	
	int correct=0;
	for(int i=0; i<4; ++i)
		correct += got[i]==instr.bytes()[i] ? 1 : 0;
	
	return {"<internal>", "Check can read same value back after write.", correct};
}

testResult rTypeAnd(mips_cpu_h cpu, mips_mem_h mem){
	mips_cpu_set_register(cpu, 1, 0x00FF00F0);
	mips_cpu_set_register(cpu, 2, 0x000F0F0F);
	mips_mem_write(mem, 0x0, 4, Instruction(AND, 1, 2, 3, 0).bytes());
	
	mips_cpu_set_pc(cpu, 0);
	mips_cpu_step(cpu);
	
	uint32_t r1,r2,r3;
	mips_cpu_get_register(cpu, 1, &r1);
	mips_cpu_get_register(cpu, 2, &r2);
	mips_cpu_get_register(cpu, 3, &r3);
	int correct = 1 && r1==0x00FF00F0 && r2==0x000F0F0F && r3==0x000F0000;
    return {"AND", "Check registers after AND-ing.", correct};
}
