/*
** GBS Import
** Copyright (C) 2014 Slimeball
**
** Based on:
**
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2012  Jonathan Liss
**
** NSF Importer v0.5 - Written by Brad Smith 11/21/2011
**
** Game Music Emu - Copyright (C) 2003-2009 Shay Green
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

//Flags

#define F_C		0x10
#define F_H 	0x20
#define F_N		0x40
#define F_Z		0x80

// Registers

#define AF  	af.w
#define BC  	bc.w
#define DE  	de.w
#define HL  	hl.w
#define IX  	ix.w
#define IY  	iy.w
#define SP  	sp.w
#define PC  	pc.w

#define flags   af.b.l
#define A   	af.b.h
#define C   	bc.b.l
#define B   	bc.b.h
#define E  		de.b.l
#define D   	de.b.h
#define L   	hl.b.l
#define H   	hl.b.h

#define IXL 	ix.b.l
#define IXH 	ix.b.h
#define IYL 	iy.b.l
#define IYH 	iy.b.h
#define SPL 	sp.b.l
#define SPH 	sp.b.h
#define PCL 	pc.b.l
#define PCH 	pc.b.h

#define MEM(address) (data[address])
#define LO() (MEM(PC))
#define HI() (MEM(PC+1))
#define FETCH() (MEM(PC++))
#define IMM8() (LO())
#define IMM16() (LO() | (HI() << 8))
#define IND16() (MEM(IMM16()))

#define BANKSWITCH(val)										\
{															\
	if(c_bank!=val && val!=-1){								\
		printf("Switch! Old: %i New: %i\n",c_bank,val);		\
		memcpy(&bdata[c_bank<<14],&data[0x4000],0x4000);	\
		printf("Pass 1\n");									\
		memcpy(&data[0x4000],&bdata[(val)<<14],0x4000);		\
		printf("Pass 2\n");									\
		c_bank=val;											\
	}														\
}

#define WRITE_NR11(val)										\
{															\
	sq1_ll=val&0x3F;										\
	sq1_lc=val&0x3F;										\
}

#define WRITE_NR21(val)										\
{															\
	sq2_ll=val&0x3F;										\
	sq2_lc=val&0x3F;										\
}

#define WRITE_NR31(val)										\
{															\
	wave_ll=val;											\
	wave_lc=val;											\
}

#define WRITE_NR41(val)										\
{															\
	noise_ll=val&0x3F;										\
	noise_lc=val&0x3F;										\
}

#define WRITE_NR12(val)										\
{															\
	Env[0].write_register(0, 2, data[0xFF12], val);			\
}

#define WRITE_NR22(val)										\
{															\
	Env[1].write_register(0, 2, data[0xFF17], val);			\
}

#define WRITE_NR42(val)										\
{															\
	Env[2].write_register(0, 2, data[0xFF21], val);			\
}

#define WRITE_NR14(val)										\
{															\
	if(val&0x80){											\
		Env[0].write_register(0, 4, data[0xFF14],val);		\
		if(!sq1_lc) sq1_lc=sq1_ll? sq1_ll: 64;				\
	}														\
}

#define WRITE_NR24(val)										\
{															\
	if(val&0x80){											\
		Env[1].write_register(0, 4, data[0xFF19],val);		\
		if(!sq2_lc) sq2_lc=sq2_ll? sq2_ll: 64;				\
	}														\
}

#define WRITE_NR34(val)										\
{															\
	if(val&0x80){											\
		if(wave_lc<1) wave_lc=wave_ll? wave_ll: 256;		\
		wave_vol = data[0xFF1C];							\
	}														\
}

#define WRITE_NR44(val)										\
{															\
	if(val&0x80){											\
		Env[2].write_register(0, 4, data[0xFF23],val);		\
		if(!noise_lc) noise_lc=noise_ll? noise_ll: 64;		\
	}														\
}


#define WRITE_WAVE(addr,val)								\
{															\
	/*fprintf(fp_log,"PENIS!\n");*/							\
}									

#define WRITE(addr,val)												\
{																	\
	temp16=addr;													\
	temp=val;														\
	if(temp16<0x3FFF && temp16>=0x2000 && is_switched)				\
		BANKSWITCH(temp-1);											\
	if(temp16>=0xFF00){												\
			/*if(temp16==0xFF25 || temp16==0xFF1E) fprintf(fp_log,"WRITE: %04X %02X\n",temp16,temp);*/	\
			switch(temp16&0xFF){									\
			case 0x10:												\
				sweeped=1;											\
				break;												\
			case 0x11:												\
				WRITE_NR11(temp);									\
				temp|=0x3F;											\
				break;												\
			case 0x16:												\
				WRITE_NR21(temp);									\
				temp|=0x3F;											\
				break;												\
			case 0x1B:												\
				WRITE_NR31(temp);									\
				break;												\
			case 0x20:												\
				WRITE_NR41(temp);									\
				temp|=0x3F;											\
				break;												\
			case 0x14:												\
				WRITE_NR14(temp);									\
				break;												\
			case 0x19:												\
				WRITE_NR24(temp);									\
				break;												\
			case 0x1E:												\
				WRITE_NR34(temp);									\
				break;												\
			case 0x23:												\
				WRITE_NR44(temp);									\
				break;												\
			case 0x12:												\
				WRITE_NR12(temp);									\
				break;												\
			case 0x17:												\
				WRITE_NR22(temp);									\
				break;												\
			case 0x1C:												\
				temp|=0x9F;											\
				break;												\
			case 0x21:												\
				WRITE_NR42(temp);									\
				break;												\
			/*if(temp16>=0xFF30 && temp16<0xFF40)*/					\
				/*WRITE_WAVE(temp16,temp);*/						\
			default:												\
				break;												\
		}															\
	}																\
	MEM(temp16)=temp;												\
}

#define READ(addr)											\
{															\
	/*fprintf(fp_log,"READ: %04X %02X\n",addr,data[addr])*/;\
}

#define BRANCH()								\
{												\
	temp = FETCH();								\
	if (temp>=0x80) PC-=0x100;					\
	PC+=temp;									\
}

#define INC8(dest)                      		\
{                                       		\
	dest++;										\
	flags&=F_C;									\
	if(!dest)									\
		flags |= F_Z;							\
	if(!(dest&0x0F))							\
		flags |= F_H;							\
}

#define INC16(dest)                     		\
{                                       		\
	dest++;										\
}

#define DEC8(dest)                      		\
{                                       		\
	flags&=F_C;									\
	/*if(!dest) flags|=F_C;*/					\
	dest--;										\
	flags|=F_N/*|F_H*/;							\
	if(!dest)									\
		flags |= F_Z;							\
	if((dest&0x0F)==0x0F)						\
		flags |= F_H;							\
}

#define DEC16(dest)                      		\
{                                       		\
	dest--;										\
}

#define SUB8(val)								\
{												\
	temp=A;										\
	A-=val;										\
	flags=F_N;									\
	if(!A) flags|=F_Z;							\
	if(A>temp) flags|=F_C;						\
	if((A&0x0F) > (temp&0x0F))					\
		flags|=F_H;								\
}

#define SBC8(val)								\
{												\
	temp=A;										\
	A-=val;										\
	if(flags&F_C) A--;							\
	flags=F_N;									\
	if(!A) flags|=F_Z;							\
	if(A>temp) flags|=F_C;						\
	if((A&0x0F) > (temp&0x0F))					\
		flags|=F_H;								\
}

#define ADD8(val)								\
{												\
	temp=A;										\
	A+=val;										\
	flags=0;									\
	if(!A) flags|=F_Z;							\
	if(A<temp) flags|=F_C;						\
	if((A&0x0F) < (temp&0x0F))					\
		flags|=F_H;								\
}

#define ADC8(val)								\
{												\
	temp=A;										\
	A+=val;										\
	if(flags&F_C) {temp++; A++;}				\
	flags=0;									\
	if(!A) flags|=F_Z;							\
	if(A<temp) flags|=F_C;						\
	if((A&0x0F) < (temp&0x0F))					\
		flags|=F_H;								\
}

#define SUB16(dest,val)							\
{												\
	temp16=dest;								\
	dest-=val;									\
	flags=F_N;									\
	if(!(dest&0xFF00)) flags=F_Z;				\
	if(dest>temp16) flags|=F_C;					\
	if((dest&0x0F00) > (temp16&0x0F00))			\
		flags|=F_H;								\
}

#define SBC16(dest,val)							\
{												\
	temp16=dest;								\
	dest-=val;									\
	if(flags&F_C) dest--;						\
	flags=F_N;									\
	if(!(dest&0xFF00)) flags=F_Z;				\
	if(dest>temp16) flags|=F_C;					\
	if((dest&0x0F00) > (temp16&0x0F00))			\
		flags|=F_H;								\
}

#define ADD16(dest,val)									\
{														\
	temp16=dest;										\
	dest+=val;											\
	flags&=F_Z;											\
	/*if(!(dest&0xFF00)) flags=F_Z;*/					\
	if(dest<temp16) flags|=F_C;							\
	flags|=(((dest&0x0FFF)-(temp16&0x0FFF))>>7)&F_H;	\
}

#define ADC16(dest,val)							\
{												\
	temp16=dest;								\
	dest+=val;									\
	if(flags&F_C) dest++;						\
	flags=0;									\
	if(!(dest&0xFF00)) flags=F_Z;				\
	if(dest<temp16) flags|=F_C;					\
	if((dest&0x0F00) < (temp16&0x0F00))			\
		flags|=F_H;								\
}

#define OR(val)									\
{												\
	A|=val;										\
	flags=0;									\
	if(!A) flags=F_Z;							\
}

#define XOR(val)								\
{												\
	A^=val;										\
	flags=0;									\
	if(!A) flags=F_Z;							\
}

#define AND(val)								\
{												\
	A&=val;										\
	flags=F_H;									\
	if(!A) flags|=F_Z;							\
}

#define CPL()									\
{												\
	A=~A;										\
	flags|=(F_N|F_H);							\
}

#define CP(val)									\
{												\
	temp=A-val;									\
	flags=F_N;									\
	if(!temp) flags|=F_Z;						\
	if(temp>A) flags|=F_C;						\
	if((temp&0x0F) > (A&0x0F))					\
		flags|=F_H;								\
}

#define PUSH(val)								\
{												\
	MEM(--SP)=(val)>>8;							\
	MEM(--SP)=(val)&255;						\
}

#define POP(val)								\
{												\
	val=MEM(SP++);								\
	val|=MEM(SP++)<<8;							\
}

#define SRL(dest)								\
{												\
	flags=0;									\
	if(dest&1) flags|=F_C;						\
	dest>>=1;									\
	if(!dest) flags|=F_Z;						\
}

#define SRA(dest)								\
{												\
	flags=0;									\
	if(dest&1) flags|=F_C;						\
	dest=(dest&128)|(dest>>1);					\
	if(!dest) flags|=F_Z;						\
}

#define SLA(dest)								\
{												\
	flags=0;									\
	if(dest&128) flags|=F_C;					\
	dest<<=1;									\
	if(!dest) flags|=F_Z;						\
}

#define RR(dest)								\
{												\
	temp=(flags&F_C)? 128: 0;					\
	flags=(dest&1)? F_C: 0;						\
	dest=temp|(dest>>1);						\
	if(!dest) flags|=F_Z;						\
}

#define RRA()								\
{											\
	temp = (flags&F_C)? 128: 0;				\
	flags = (A&1)? F_C: 0;					\
	A = temp|(A>>1);						\
}


#define RRC(dest)								\
{												\
	flags=(dest&1)? F_C: 0;						\
	dest=((dest&1)<<7)|(dest>>1);				\
	if(!dest) flags|=F_Z;						\
}

#define RRCA()									\
{												\
	flags = (A&1)? F_C: 0;						\
	A = ((A&1)<<7)|(A>>1);						\
}

#define RL(dest)								\
{												\
	temp=(flags&F_C)? 1: 0;						\
	flags=(dest&128)? F_C: 0;					\
	dest=(dest<<1)|temp;						\
	if(!dest) flags|=F_Z;						\
}

#define RLA()									\
{												\
	temp=(flags&F_C)? 1: 0;						\
	flags=(A&128)? F_C: 0;					\
	A=(A<<1)|temp;						\
	if(!A) flags|=F_Z;						\
}

#define RLC(dest)								\
{												\
	flags=(dest&128)? F_C: 0;					\
	dest=(dest<<1)|(dest>>7);					\
	if(!dest) flags|=F_Z;						\
}

#define RLCA()									\
{												\
	flags = (A&128)? F_C: 0;					\
	A = (A<<1)|(A>>7);							\
}		

#define BIT(b,val)								\
{												\
	flags&=F_C;									\
	flags|=F_H;									\
	if(!(val&(1<<(b)))){						\
		flags|=F_Z;								\
	}											\
}

#define SET(b,dest)								\
{												\
	dest|=(1<<(b));								\
}

#define RES(b,dest)								\
{												\
	dest&=~(1<<(b));							\
}

#define SWAP(dest)								\
{												\
	flags=0;									\
	dest=(dest>>4)|((dest&15)<<4);				\
	if(!dest) flags=F_Z;						\
}

#define SCF()									\
{												\
	flags &= F_Z;								\
	flags |= F_C;								\
}

#define CCF()									\
{												\
	flags &= F_C|F_Z;							\
	flags ^= F_C;								\
}

#define RSTCALL(dest)							\
{												\
	PUSH(PC);									\
	PC=dest;									\
}

#define CALL(dest)								\
{												\
	PUSH((PC+2));								\
	PC=dest;									\
}

#define RET()									\
{												\
	if(SP==Info.sp_init) return 0;				\
	POP(PC);									\
}
