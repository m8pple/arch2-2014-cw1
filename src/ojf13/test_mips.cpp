#include "../../include/mips_test.h"

#include <iostream>

int main(){
    mips_mem_h mem = mips_mem_create_ram(1<<5, 4);
    mips_cpu_h cpu = mips_cpu_create(mem);
    
    mips_cpu_reset(cpu);
    
    uint32_t got;
    mips_cpu_get_register(cpu, 28, &got);
    std::cout << std::endl << "Reg 28 is: " << got << std::endl;
    
	return 0;
}
