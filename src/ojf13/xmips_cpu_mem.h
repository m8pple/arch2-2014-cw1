//
//  mips_cpu_mem.h
//  MIPSim
//
//  Created by Ollie Ford on 23/10/2014.
//  Copyright (c) 2014 OJFord. All rights reserved.
//

#include "mips_mem.h"

struct mips_mem{
public:
	mips_mem(uint32_t, uint32_t);
	~mips_mem();
	
	void read(uint8_t*, uint32_t, uint32_t) const;
	void write(uint32_t, uint32_t, const uint8_t*);
	
	uint8_t readByte(uint32_t);
	void writeByte(uint32_t, uint8_t);
	
	uint32_t readWord(uint32_t);
	void writeWord(uint32_t, uint32_t);
	
private:
	uint32_t	_size;
	uint32_t	_bSize;
	uint8_t*	_data;
};
