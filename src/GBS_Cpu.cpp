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

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include "GBZ80_tables.h"
#include "GBZ80_macros.h"
#include "GBS_Emu.h"

#define PANIC(msg)		\
{						\
	printf("%s\n",msg);	\
	return false;		\
}

#define PANIC2(msg)			\
{							\
	strcpy(errorcode,msg);	\
	printf("%s\n",msg);		\
	return false;			\
}		

void CGBS_Emu::LogDisasm()
{
	fprintf(fp_log,"\nGBS DISASM:\n\n");

	for(int i=Info.init_addr;i<(Info.init_addr+100);i++){
		unsigned char c_byte=data[i];
		fprintf(fp_log,"$%04X $%02X - %s",i,c_byte,gbs_opcodes[c_byte]);
		for(int j=0;j<gbs_opbytes[c_byte]-1;j++){
			fprintf(fp_log," $%02X",data[++i]);
		}
		fprintf(fp_log,"\n");
	}
}

bool CGBS_Emu::Init(int track)
{
	//fp_log=fopen("GBS_init.log","w"); if(!fp_log) PANIC("Couldn't create log file! :(\n");

	PC = Info.init_addr;
	SP = Info.sp_init;
	BC=0;
	DE=0;
	HL=0;
	A=track;
	flags=0;
	int i=0;

	//fprintf(fp_log,"\nGBS INIT LOG:\n\n");

	while(true){
		//fprintf(fp_log,"PC: %04X OP: %02X A:%02X F:%02X HL:%04X BC:%04X DE:%04X SP:%04X ASM: %s\n", PC, data[PC], A, flags, HL, BC, DE, SP, gbs_opcodes[data[PC]]);
		int result=RunCpu();
		if(result==0) break;
		if(result==-1) return false;
		if(i++>200000) PANIC2("Init routine uses an abnormally high amount of instructions! PCM isn't supported yet. ;)\n");
	}

	//fprintf(fp_log,"\nInit routine exited properly :)\n\n");
	return true;
}

bool CGBS_Emu::PlayOnce()
{
	PC = Info.play_addr;
	SP = Info.sp_init;
	int i=0;

	while(true){
		//fprintf(fp_log,"PC: %04X OP: %02X A:%02X F:%02X HL:%04X BC:%04X DE:%04X SP:%04X ASM: %s\n", PC, data[PC], A, flags, HL, BC, DE, SP, gbs_opcodes[data[PC]]);
		int result=RunCpu();
		if(result==0){
			 //fprintf(fp_log,"\nPlay routine exited properly :)\n\n");
			break;
		}
		if(result==-1) return false;
		if(i++>200000) PANIC2("Play routine uses an abnormally high amount of instructions! PCM isn't supported yet. ;)\n");
	}
	return true;
}

int CGBS_Emu::RunCpu(void)
{
	unsigned char temp;
	unsigned short temp16;

	unsigned char op = FETCH();

	switch(op){

		// Ext. ops

		case 0xCB: return RunExtCB();

		// JR

		case 0x18: BRANCH(); break;
		case 0x20: if(!(flags&F_Z)) BRANCH() else PC++; break;
		case 0x30: if(!(flags&F_C)) BRANCH() else PC++; break;
		case 0x28: if(flags&F_Z) BRANCH() else PC++; break;
		case 0x38: if(flags&F_C) BRANCH() else PC++; break;

		// LD

		case 0x01: BC=IMM16(); PC+=2; break;
		case 0x11: DE=IMM16(); PC+=2; break;
		case 0x21: HL=IMM16(); PC+=2; break;
		case 0x31: SP=IMM16(); PC+=2; break;

		case 0x02: WRITE(BC,A); break;
		case 0x12: WRITE(DE,A); break;
		case 0x22: WRITE(HL++,A); break;
		case 0x32: WRITE(HL--,A); break;

		case 0x06: B=IMM8(); PC+=1; break;
		case 0x16: D=IMM8(); PC+=1; break;
		case 0x26: H=IMM8(); PC+=1; break;
		case 0x36: WRITE(HL,IMM8()); PC+=1; break;

		case 0x08: MEM(IMM16()) = SP&0xFF; MEM(IMM16()+1) = SP>>8; PC+=2; break;

		case 0x0A: READ(BC); A=MEM(BC); break;
		case 0x1A: READ(DE); A=MEM(DE); break;
		case 0x2A: READ(HL); A=MEM(HL++); break;
		case 0x3A: READ(HL); A=MEM(HL--); break;

		case 0x0E: C=IMM8(); PC+=1; break;
		case 0x1E: E=IMM8(); PC+=1; break;
		case 0x2E: L=IMM8(); PC+=1; break;
		case 0x3E: A=IMM8(); PC+=1; break;

		case 0xE0: WRITE(0xFF00+IMM8(),A); PC+=1; break;
		case 0xF0: READ(0xFF00+IMM8()); A=MEM(0xFF00+IMM8()); PC+=1; break;

		case 0xF8: HL=SP+(signed char)(IMM8()); PC+=1; break;
		case 0xF9: SP=HL; break;

		case 0xEA: WRITE(IMM16(),A); PC+=2; break;
		case 0xFA: READ(IMM16()); A=MEM(IMM16()); PC+=2; break;

		case 0x40: B=B; break;
		case 0x41: B=C; break;
		case 0x42: B=D; break;
		case 0x43: B=E; break;
		case 0x44: B=H; break;
		case 0x45: B=L; break;
		case 0x46: READ(HL); B=MEM(HL); break;
		case 0x47: B=A; break;

		case 0x48: C=B; break;
		case 0x49: C=C; break;
		case 0x4A: C=D; break;
		case 0x4B: C=E; break;
		case 0x4C: C=H; break;
		case 0x4D: C=L; break;
		case 0x4E: READ(HL); C=MEM(HL); break;
		case 0x4F: C=A; break;

		case 0x50: D=B; break;
		case 0x51: D=C; break;
		case 0x52: D=D; break;
		case 0x53: D=E; break;
		case 0x54: D=H; break;
		case 0x55: D=L; break;
		case 0x56: READ(HL); D=MEM(HL); break;
		case 0x57: D=A; break;

		case 0x58: E=B; break;
		case 0x59: E=C; break;
		case 0x5A: E=D; break;
		case 0x5B: E=E; break;
		case 0x5C: E=H; break;
		case 0x5D: E=L; break;
		case 0x5E: READ(HL); E=MEM(HL); break;
		case 0x5F: E=A; break;

		case 0x60: H=B; break;
		case 0x61: H=C; break;
		case 0x62: H=D; break;
		case 0x63: H=E; break;
		case 0x64: H=H; break;
		case 0x65: H=L; break;
		case 0x66: READ(HL); H=MEM(HL); break;
		case 0x67: H=A; break;

		case 0x68: L=B; break;
		case 0x69: L=C; break;
		case 0x6A: L=D; break;
		case 0x6B: L=E; break;
		case 0x6C: L=H; break;
		case 0x6D: L=L; break;
		case 0x6E: READ(HL); L=MEM(HL); break;
		case 0x6F: L=A; break;

		case 0x70: WRITE(HL,B); break;
		case 0x71: WRITE(HL,C); break;
		case 0x72: WRITE(HL,D); break;
		case 0x73: WRITE(HL,E); break;
		case 0x74: WRITE(HL,H); break;
		case 0x75: WRITE(HL,L); break;
		//case 0x76: H=MEM(HL); break; HALT!!!!!!!
		case 0x77: WRITE(HL,A); break;

		case 0x78: A=B; break;
		case 0x79: A=C; break;
		case 0x7A: A=D; break;
		case 0x7B: A=E; break;
		case 0x7C: A=H; break;
		case 0x7D: A=L; break;
		case 0x7E: READ(HL); A=MEM(HL); break;
		case 0x7F: A=A; break;

		case 0xE2: WRITE(0xFF00+C,A); break;
		case 0xF2: READ(0xFF00+C); A=MEM(0xFF00+C); break;

		// INC/DEC

		case 0x03: INC16(BC); break;
		case 0x13: INC16(DE); break;
		case 0x23: INC16(HL); break;
		case 0x33: INC16(SP); break;
		case 0x04: INC8(B); break;
		case 0x14: INC8(D); break;
		case 0x24: INC8(H); break;
		case 0x34: INC8(MEM(HL)); break;
		case 0x05: DEC8(B); break;
		case 0x15: DEC8(D); break;
		case 0x25: DEC8(H); break;
		case 0x35: DEC8(MEM(HL)); break;
		case 0x0B: DEC16(BC); break;
		case 0x1B: DEC16(DE); break;
		case 0x2B: DEC16(HL); break;
		case 0x3B: DEC16(SP); break;
		case 0x0C: INC8(C); break;
		case 0x1C: INC8(E); break;
		case 0x2C: INC8(L); break;
		case 0x3C: INC8(A); break;
		case 0x0D: DEC8(C); break;
		case 0x1D: DEC8(E); break;
		case 0x2D: DEC8(L); break;
		case 0x3D: DEC8(A); break;

		// ADD/ADC/SUB/SBC

		case 0x09: ADD16(HL,BC); break;
		case 0x19: ADD16(HL,DE); break;
		case 0x29: ADD16(HL,HL); break;
		case 0x39: ADD16(HL,SP); break;

		case 0x80: ADD8(B); break;
		case 0x81: ADD8(C); break;
		case 0x82: ADD8(D); break;
		case 0x83: ADD8(E); break;
		case 0x84: ADD8(H); break;
		case 0x85: ADD8(L); break;
		case 0x86: ADD8(MEM(HL)); break;
		case 0x87: ADD8(A); break;
		case 0xC6: ADD8(IMM8()); PC+=1; break;

		case 0x88: ADC8(B); break;
		case 0x89: ADC8(C); break;
		case 0x8A: ADC8(D); break;
		case 0x8B: ADC8(E); break;
		case 0x8C: ADC8(H); break;
		case 0x8D: ADC8(L); break;
		case 0x8E: ADC8(MEM(HL)); break;
		case 0x8F: ADC8(A); break;
		case 0xCE: ADC8(IMM8()); PC+=1; break;

		case 0x90: SUB8(B); break;
		case 0x91: SUB8(C); break;
		case 0x92: SUB8(D); break;
		case 0x93: SUB8(E); break;
		case 0x94: SUB8(H); break;
		case 0x95: SUB8(L); break;
		case 0x96: SUB8(MEM(HL)); break;
		case 0x97: SUB8(A); break;
		case 0xD6: SUB8(IMM8()); PC+=1; break;

		case 0x98: SBC8(B); break;
		case 0x99: SBC8(C); break;
		case 0x9A: SBC8(D); break;
		case 0x9B: SBC8(E); break;
		case 0x9C: SBC8(H); break;
		case 0x9D: SBC8(L); break;
		case 0x9E: SBC8(MEM(HL)); break;
		case 0x9F: SBC8(A); break;
		case 0xDE: SBC8(IMM8()); PC+=1; break;

		case 0xE8: ADD16(SP,(signed char)IMM8()); PC+=1; break; // Possibly broken

		// RR/RL

		case 0x07: RLCA(); break;
		case 0x0F: RRCA(); break;
		case 0x17: RLA(); break;
		case 0x1F: RRA(); break;

		// Flags

		case 0x2F: CPL(); break;
		case 0x37: SCF(); break;
		case 0x3F: CCF(); break;

		// AND/XOR/OR/CP

		case 0xA0: AND(B); break;
		case 0xA1: AND(C); break;
		case 0xA2: AND(D); break;
		case 0xA3: AND(E); break;
		case 0xA4: AND(H); break;
		case 0xA5: AND(L); break;
		case 0xA6: AND(MEM(HL)); break;
		case 0xA7: AND(A); break;
		case 0xE6: AND(IMM8()); PC+=1; break;

		case 0xA8: XOR(B); break;
		case 0xA9: XOR(C); break;
		case 0xAA: XOR(D); break;
		case 0xAB: XOR(E); break;
		case 0xAC: XOR(H); break;
		case 0xAD: XOR(L); break;
		case 0xAE: XOR(MEM(HL)); break;
		case 0xAF: XOR(A); break;
		case 0xEE: XOR(IMM8()); PC+=1; break;

		case 0xB0: OR(B); break;
		case 0xB1: OR(C); break;
		case 0xB2: OR(D); break;
		case 0xB3: OR(E); break;
		case 0xB4: OR(H); break;
		case 0xB5: OR(L); break;
		case 0xB6: OR(MEM(HL)); break;
		case 0xB7: OR(A); break;
		case 0xF6: OR(IMM8()); PC+=1; break;

		case 0xB8: CP(B); break;
		case 0xB9: CP(C); break;
		case 0xBA: CP(D); break;
		case 0xBB: CP(E); break;
		case 0xBC: CP(H); break;
		case 0xBD: CP(L); break;
		case 0xBE: CP(MEM(HL)); break;
		case 0xBF: CP(A); break;
		case 0xFE: CP(IMM8()); PC+=1; break;

		// RET

		case 0xC0: if(!(flags&F_Z)) RET(); break;
		case 0xC8: if(flags&F_Z) RET(); break;
		case 0xC9: RET(); break;
		case 0xD0: if(!(flags&F_C)) RET(); break;
		case 0xD8: if(flags&F_C) RET(); break;
		case 0xD9: RET(); break;

		// POP/PUSH

		case 0xC1: POP(BC); break;
		case 0xC5: PUSH(BC); break;
		case 0xD1: POP(DE); break;
		case 0xD5: PUSH(DE); break;
		case 0xE1: POP(HL); break;
		case 0xE5: PUSH(HL); break;
		case 0xF1: POP(AF); flags&=0xF0; break;
		case 0xF5: PUSH(AF); break;

		// JP/CALL

		case 0xC2: if(!(flags&F_Z)) PC=IMM16(); else PC+=2; break;
		case 0xC3: PC=IMM16(); break;
		case 0xCA: if(flags&F_Z) PC=IMM16(); else PC+=2; break;
		case 0xD2: if(!(flags&F_C)) PC=IMM16(); else PC+=2; break;
		case 0xDA: if(flags&F_C) PC=IMM16(); else PC+=2; break;

		case 0xC4: if(!(flags&F_Z)) CALL(IMM16()) else PC+=2; break;
		case 0xCC: if(flags&F_Z) CALL(IMM16()) else PC+=2; break;
		case 0xCD: CALL(IMM16()); break;
		case 0xD4: if(!(flags&F_C)) CALL(IMM16()) else PC+=2; break;
		case 0xDC: if(flags&F_C) CALL(IMM16()) else PC+=2; break;

		case 0xE9: PC=HL; break;

		case 0x00: break; // NOP
		case 0xF3: break; // DI
		case 0xFB: break; // EI

		// RST Vectors

		case 0xC7: RSTCALL(Info.load_addr+0x00); break;
		case 0xCF: RSTCALL(Info.load_addr+0x08); break;
		case 0xD7: RSTCALL(Info.load_addr+0x10); break;
		case 0xDF: RSTCALL(Info.load_addr+0x18); break;
		case 0xE7: RSTCALL(Info.load_addr+0x20); break;
		case 0xEF: RSTCALL(Info.load_addr+0x28); break;
		case 0xF7: RSTCALL(Info.load_addr+0x30); break;
		case 0xFF: RSTCALL(Info.load_addr+0x38); break;

		default:
			sprintf(errorcode,"Error: Unknown opcode $%02X at $%04X\n",op,PC-1);
			printf("Error: Unknown opcode $%02X at $%04X\n",op,PC-1);
			return -1;
	}

	return 1;
}

int CGBS_Emu::RunExtCB(void)
{
	unsigned char temp;
	unsigned char op = FETCH();

	switch(op){
		case 0x00: RLC(B); break;
		case 0x01: RLC(C); break;
		case 0x02: RLC(D); break;
		case 0x03: RLC(E); break;
		case 0x04: RLC(H); break;
		case 0x05: RLC(L); break;
		case 0x06: RLC(MEM(HL)); break;
		case 0x07: RLC(A); break;
		case 0x08: RRC(B); break;
		case 0x09: RRC(C); break;
		case 0x0A: RRC(D); break;
		case 0x0B: RRC(E); break;
		case 0x0C: RRC(H); break;
		case 0x0D: RRC(L); break;
		case 0x0E: RRC(MEM(HL)); break;
		case 0x0F: RRC(A); break;

		case 0x10: RL(B); break;
		case 0x11: RL(C); break;
		case 0x12: RL(D); break;
		case 0x13: RL(E); break;
		case 0x14: RL(H); break;
		case 0x15: RL(L); break;
		case 0x16: RL(MEM(HL)); break;
		case 0x17: RL(A); break;
		case 0x18: RR(B); break;
		case 0x19: RR(C); break;
		case 0x1A: RR(D); break;
		case 0x1B: RR(E); break;
		case 0x1C: RR(H); break;
		case 0x1D: RR(L); break;
		case 0x1E: RR(MEM(HL)); break;
		case 0x1F: RR(A); break;

		case 0x20: SLA(B); break;
		case 0x21: SLA(C); break;
		case 0x22: SLA(D); break;
		case 0x23: SLA(E); break;
		case 0x24: SLA(H); break;
		case 0x25: SLA(L); break;
		case 0x26: SLA(MEM(HL)); break;
		case 0x27: SLA(A); break;
		case 0x28: SRA(B); break;
		case 0x29: SRA(C); break;
		case 0x2A: SRA(D); break;
		case 0x2B: SRA(E); break;
		case 0x2C: SRA(H); break;
		case 0x2D: SRA(L); break;
		case 0x2E: SRA(MEM(HL)); break;
		case 0x2F: SRA(A); break;

		case 0x30: SWAP(B); break;
		case 0x31: SWAP(C); break;
		case 0x32: SWAP(D); break;
		case 0x33: SWAP(E); break;
		case 0x34: SWAP(H); break;
		case 0x35: SWAP(L); break;
		case 0x36: SWAP(MEM(HL)); break;
		case 0x37: SWAP(A); break;
		case 0x38: SRL(B); break;
		case 0x39: SRL(C); break;
		case 0x3A: SRL(D); break;
		case 0x3B: SRL(E); break;
		case 0x3C: SRL(H); break;
		case 0x3D: SRL(L); break;
		case 0x3E: SRL(MEM(HL)); break;
		case 0x3F: SRL(A); break;

		// BIT

		case 0x40: case 0x48: case 0x50: case 0x58:
		case 0x60: case 0x68: case 0x70: case 0x78:
			BIT((op&0x3F)>>3,B); break;

		case 0x41: case 0x49: case 0x51: case 0x59:
		case 0x61: case 0x69: case 0x71: case 0x79:
			BIT((op&0x3F)>>3,C); break;

		case 0x42: case 0x4A: case 0x52: case 0x5A:
		case 0x62: case 0x6A: case 0x72: case 0x7A:
			BIT((op&0x3F)>>3,D); break;

		case 0x43: case 0x4B: case 0x53: case 0x5B:
		case 0x63: case 0x6B: case 0x73: case 0x7B:
			BIT((op&0x3F)>>3,E); break;

		case 0x44: case 0x4C: case 0x54: case 0x5C:
		case 0x64: case 0x6C: case 0x74: case 0x7C:
			BIT((op&0x3F)>>3,H); break;

		case 0x45: case 0x4D: case 0x55: case 0x5D:
		case 0x65: case 0x6D: case 0x75: case 0x7D:
			BIT((op&0x3F)>>3,L); break;

		case 0x46: case 0x4E: case 0x56: case 0x5E:
		case 0x66: case 0x6E: case 0x76: case 0x7E:
			BIT((op&0x3F)>>3,MEM(HL)); break;

		case 0x47: case 0x4F: case 0x57: case 0x5F:
		case 0x67: case 0x6F: case 0x77: case 0x7F:
			BIT((op&0x3F)>>3,A); break;

		// RES

		case 0x80: case 0x88: case 0x90: case 0x98:
		case 0xA0: case 0xA8: case 0xB0: case 0xB8:
			RES((op&0x3F)>>3,B); break;

		case 0x81: case 0x89: case 0x91: case 0x99:
		case 0xA1: case 0xA9: case 0xB1: case 0xB9:
			RES((op&0x3F)>>3,C); break;

		case 0x82: case 0x8A: case 0x92: case 0x9A:
		case 0xA2: case 0xAA: case 0xB2: case 0xBA:
			RES((op&0x3F)>>3,D); break;

		case 0x83: case 0x8B: case 0x93: case 0x9B:
		case 0xA3: case 0xAB: case 0xB3: case 0xBB:
			RES((op&0x3F)>>3,E); break;

		case 0x84: case 0x8C: case 0x94: case 0x9C:
		case 0xA4: case 0xAC: case 0xB4: case 0xBC:
			RES((op&0x3F)>>3,H); break;

		case 0x85: case 0x8D: case 0x95: case 0x9D:
		case 0xA5: case 0xAD: case 0xB5: case 0xBD:
			RES((op&0x3F)>>3,L); break;

		case 0x86: case 0x8E: case 0x96: case 0x9E:
		case 0xA6: case 0xAE: case 0xB6: case 0xBE:
			RES((op&0x3F)>>3,MEM(HL)); break;

		case 0x87: case 0x8F: case 0x97: case 0x9F:
		case 0xA7: case 0xAF: case 0xB7: case 0xBF:
			RES((op&0x3F)>>3,A); break;

		// SET

		case 0xC0: case 0xC8: case 0xD0: case 0xD8:
		case 0xE0: case 0xE8: case 0xF0: case 0xF8:
			SET((op&0x3F)>>3,B); break;

		case 0xC1: case 0xC9: case 0xD1: case 0xD9:
		case 0xE1: case 0xE9: case 0xF1: case 0xF9:
			SET((op&0x3F)>>3,C); break;

		case 0xC2: case 0xCA: case 0xD2: case 0xDA:
		case 0xE2: case 0xEA: case 0xF2: case 0xFA:
			SET((op&0x3F)>>3,D); break;

		case 0xC3: case 0xCB: case 0xD3: case 0xDB:
		case 0xE3: case 0xEB: case 0xF3: case 0xFB:
			SET((op&0x3F)>>3,E); break;

		case 0xC4: case 0xCC: case 0xD4: case 0xDC:
		case 0xE4: case 0xEC: case 0xF4: case 0xFC:
			SET((op&0x3F)>>3,H); break;

		case 0xC5: case 0xCD: case 0xD5: case 0xDD:
		case 0xE5: case 0xED: case 0xF5: case 0xFD:
			SET((op&0x3F)>>3,L); break;

		case 0xC6: case 0xCE: case 0xD6: case 0xDE:
		case 0xE6: case 0xEE: case 0xF6: case 0xFE:
			SET((op&0x3F)>>3,MEM(HL)); break;

		case 0xC7: case 0xCF: case 0xD7: case 0xDF:
		case 0xE7: case 0xEF: case 0xF7: case 0xFF:
			SET((op&0x3F)>>3,A); break;

		default:
			sprintf(errorcode,"Error: Unknown extended opcode $%02X at $%04X\n",op,PC-1);
			printf("Error: Unknown extended opcode $%02X at $%04X\n",op,PC-1);
			return -1;
	}
	return 1;
}
