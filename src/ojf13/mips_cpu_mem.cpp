//
//  mips_cpu_mem.h
//  MIPSim
//
//  Created by Ollie Ford on 22/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "include/mips_cpu_mem.h"

/*
 * MIPS memory
 */
mips_mem::mips_mem(uint32_t size, uint32_t bSize) : _size(size), _bSize(bSize){
	_data = new uint8_t[_size];
};

mips_mem::~mips_mem(){
	delete[] _data;
}

void mips_mem::read(uint8_t* obuf, uint32_t addr, uint32_t len) const{
	if( addr%_bSize || (addr+len)%_bSize )
		throw mips_ExceptionInvalidAlignment;
	if( addr+len > _size )
		throw mips_ExceptionInvalidAddress;
	
	for(unsigned i=0; i<len; ++i){
		*obuf++ = _data[addr+i];
	}
}

void mips_mem::write(uint32_t addr, uint32_t len, const uint8_t* ibuf){
	if( addr%_bSize || (addr+len)%_bSize )
		throw mips_ExceptionInvalidAlignment;
	if( addr+len > _size )
		throw mips_ExceptionInvalidAddress;
	
	for(unsigned i=0; i<len; ++i){
		_data[addr+i] = *ibuf++;
	}
}
//
