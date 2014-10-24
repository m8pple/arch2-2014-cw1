//
//  mips_cpu_mem_api.cpp
//  MIPSim
//
//  Created by Ollie Ford on 23/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "include/mips_cpu_mem.h"

mips_error mips_mem_read(const mips_mem_h mem, uint32_t addr,  uint32_t len, uint8_t *obuf){
	mips_error ret = mips_Success;
	try{
		((mips_mem*)mem)->read(obuf, addr, len);
	} catch(mips_error ret){};
	
	return ret;
}

mips_error mips_mem_write(mips_mem_h mem, uint32_t addr, uint32_t len, const uint8_t *ibuf){
	mips_error ret = mips_Success;
	try {
		((mips_mem*)mem)->write(addr, len, ibuf);
	} catch (mips_error ret){};
	
	return ret;
}

void mips_mem_free(mips_mem_h mem){
	delete ((mips_mem*)mem);
}

mips_mem_h mips_mem_create_ram(uint32_t size, uint32_t blockSize){
	return (mips_mem_h)new mips_mem(size, blockSize);
}
