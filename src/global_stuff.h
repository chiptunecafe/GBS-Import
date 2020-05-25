//Custom global functions, types and macros

#pragma once

#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

#define STRCMP_EQUAL		0

#define zero_mem(a,b) memset(a,0,b)

byte read_byte(FILE* fp);
dword read_word(FILE* fp);
dword read_dword(FILE* fp);
dword read_inv_word(FILE* fp);
dword read_inv_dword(FILE* fp);

void write_byte(FILE* fp, byte value);
void write_word(FILE* fp, word value);
void write_dword(FILE* fp, dword value);
void write_inv_word(FILE* fp,word value);
void write_inv_dword(FILE* fp,dword value);

void write_zeros(FILE* fp, int count);

inline byte read_byte(FILE* fp)						{ int value; fread(&value,1,1,fp); return value&0xFF; }
inline dword read_word(FILE* fp)					{ dword value; fread(&value,2,1,fp); return value&0xFFFF; }
inline dword read_dword(FILE* fp)					{ dword value; fread(&value,4,1,fp); return value; }
inline dword read_inv_word(FILE* fp)				{ dword value; value=256*read_byte(fp)+read_byte(fp); return value&0xFFFF; } 
inline dword read_inv_dword(FILE* fp)				{ dword value; value=65536*read_inv_word(fp)+read_inv_word(fp); return value; }

inline void write_byte(FILE* fp, byte value)		{ fwrite(&value,1,1,fp); }
inline void write_word(FILE* fp, word value)		{ fwrite(&value,2,1,fp); }
inline void write_dword(FILE* fp, dword value)		{ fwrite(&value,4,1,fp); }
inline void write_inv_word(FILE* fp,word value)		{ write_byte(fp,value>>8); write_byte(fp,value&255); }
inline void write_inv_dword(FILE* fp,dword value)	{ write_inv_word(fp,value>>16); write_inv_word(fp,value&65535); }
inline void write_zeros(FILE* fp, int count)		{ for(int i=0;i<count;i++) write_byte(fp,0); }