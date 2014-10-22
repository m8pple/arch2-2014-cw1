
#ifndef MIPS_NUM_REG
#define MIPS_NUM_REG 32U
#endif

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

struct mips_gp_regs{
public:
	mips_gp_regs(void) : _r0(false){};
	
	mips_register& operator[](int idx){
		if(idx >= MIPS_NUM_REG)
			throw mips_ErrorInvalidArgument;
		else
			return idx==0 ? _r0 : _r[idx-1];
	}
	
private:
	mips_register _r0;
	mips_register _r[MIPS_NUM_REG-1];
};

typedef enum _mips_cpu_stage{
	IF	=0,
	ID	=1,
	EX	=2,
	MEM	=3,
	WB	=4
} mips_cpu_stage;

struct mips_cpu_impl{
public:
	mips_cpu_impl(void) : r(), _pc(), _stage(IF){};
	
	mips_error reset(void){
		
		//Zero registers
		for(int i=0; i<MIPS_NUM_REG; ++i){
			r[i].value(0);
		}
		
		_pc.value(0);
		_hi.value(0);
		_lo.value(0);
		
		return mips_Success;
	}
	
	void step(void){
		_pc.value(_pc.value()+4);
	}
	
	uint32_t pc(void) const{
		return _pc.value();
	}
	
	mips_gp_regs	r;
	
private:
	mips_register	_pc;
	mips_register	_hi;
	mips_register	_lo;
	mips_cpu_stage	_stage;
};

