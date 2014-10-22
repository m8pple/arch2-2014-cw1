#include "../../include/mips.h"
#include "include/mips_cpu.h"

#include <iostream>

struct mips_register{
public:
    mips_register(void) : _value(0), _allowSet(true){};
    mips_register(bool isZero) : _value(0), _allowSet(isZero){};
    
    uint32_t value(void) const{
        return _value;
    }
    void value(uint32_t iVal){
        if(_allowSet)
            _value = iVal;
    };
    
private:
    uint32_t _value;
    bool _allowSet;
};

/*
struct mips_zero_reg: mips_register{
public:
    mips_zero_reg(void) : mips_register(){};
    
    using mips_register::value;
    void value(uint32_t iVal){
        std::cout << "setting reg 0" << std::endl;
        mips_register::value(0);
    }
    
private:
};*/

struct mips_reg_set{
public:
    mips_reg_set(void) : r0(false){};
    
    mips_register& operator[](int idx){
        if(idx >= MIPS_NUM_REG)
            throw mips_ErrorInvalidArgument;
        else
            return idx==0 ? r0 : r[idx-1];
    }
    
private:
    mips_register r0;
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
        
        //Zero registers
        for(int i=0; i<MIPS_NUM_REG; ++i){
            r[i].value(0);
        }
        
        return mips_Success;
    }

    mips_reg_set r;
    mips_pc pc;
    mips_cpu_state state;
    
private:
};

/*mips_mem_h mips_mem_create_ram(uint32_t size, uint32_t blocksize){
    return nullptr;
}*/

mips_cpu_h mips_cpu_create(mips_mem_h mem){
    return new mips_cpu_impl();
}

mips_error mips_cpu_reset(mips_cpu_h cpu){
    return cpu->reset();
}

void mips_cpu_free(mips_cpu_h cpu){
    delete cpu;
}

mips_error mips_cpu_step(mips_cpu_h cpu){
    try{
        cpu->pc.step();
        return mips_Success;
    } catch(mips_error e){
        return e;
    }
}

mips_error mips_cpu_get_register(mips_cpu_h cpu, unsigned idx, uint32_t *oVal){
    mips_error ret = mips_Success;
    if(idx >= MIPS_NUM_REG)
        ret = mips_ErrorInvalidArgument;
    else
        *oVal = cpu->r[idx].value();
    return ret;
}

mips_error mips_cpu_set_register(mips_cpu_h cpu, unsigned idx, uint32_t iVal){
    mips_error ret = mips_Success;
    if(idx >= MIPS_NUM_REG)
        ret = mips_ErrorInvalidArgument;
    else
        try{
            cpu->r[idx].value(iVal);
        } catch (mips_error ret){};
    return ret;
}

mips_error mips_cpu_get_pc(mips_cpu_h cpu, uint32_t *oVal){
    *oVal = cpu->pc.value();
    
    return mips_Success;
}
