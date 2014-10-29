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

uint8_t mips_mem::readByte(uint32_t addr){
	uint8_t ret[4];
	read(ret, addr, 4);
	return ret[0];
}

void mips_mem::writeByte(uint32_t addr, uint8_t data){
	uint8_t buf[4];
	read(buf, addr, 4);
	buf[0] = data;
	write(addr, 1, buf);
}

uint32_t mips_mem::readWord(uint32_t addr){
	uint8_t buf[4];
	read(buf, addr, 4);
	
	uint32_t word = 0x0;
	for(unsigned i=0; i<4; ++i)
		word |= buf[3-i]<<(i*8);
	
	return word;
}

void mips_mem::writeWord(uint32_t addr, uint32_t data){
	uint8_t buf[4];
	for(unsigned i=0; i<4; ++i)
		buf[i] = (data >> (i*8))&0x000000FF;
	write(addr, 4, buf);
}

//
