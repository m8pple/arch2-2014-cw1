#include "../../include/mips.h"
#include "include/mips_cpu.h"

#include <iostream>

#define MIPS_NUM_REG 32

struct mips_register{
public:
    mips_register(void) : _data(0){};
    mips_register(uint32_t val) : _data(val){};
    
    uint32_t data(void) const{
        return _data;
    }
    
    mips_register& operator=(uint32_t rhs){
        _data = rhs;
        return *this;
    }
    void set(uint32_t val){
        _data = val;
    }
    
private:
    uint32_t _data;
};

struct mips_zero_reg: mips_register{
public:
    void set(uint32_t val) const{
        //no set
    }
    
private:
};

struct mips_reg_set{
public:
    mips_reg_set(void){};
    
    mips_register& operator[](int idx){
        return idx==0 ? r0 : r[idx];
    }
    
private:
    mips_zero_reg r0;
    mips_register r[MIPS_NUM_REG-1];
};

struct mips_pc: mips_register{
public:
    void step(void){
        //advance pc
    }
    
private:
};

typedef enum _mips_cpu_state{
    UNDEF=0,
    FETCH=1,
    DECODE=2,
    EXECUTE=3
} mips_cpu_state;

struct mips_cpu_impl{
public:
    mips_cpu_impl(void) : r(), pc(), state(UNDEF){};
    
    mips_error reset(void){
        for(int i=0; i<MIPS_NUM_REG; ++i){
            r[i]=0;
        }
        
        return mips_Success;
    }

    mips_reg_set r;
    mips_pc pc;
    mips_cpu_state state;
    
private:
};

mips_mem_h mips_mem_create_ram(uint32_t size, uint32_t blocksize){
    return nullptr;
}

mips_cpu_h mips_cpu_create(mips_mem_h mem){
    return new mips_cpu_impl();
}

mips_error mips_cpu_reset(mips_cpu_h cpu){
    return cpu->reset();
}

mips_error mips_cpu_get_register(mips_cpu_h cpu, unsigned idx, uint32_t *val){
    mips_error ret = mips_Success;
    if(idx<MIPS_NUM_REG){
        *val = cpu->r[idx].data();
    } else {
        ret = mips_ErrorInvalidArgument;
    }
    return ret;
}
