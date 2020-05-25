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

#include <cstdio>
#include <cstring>
#include "FTMFile.h"
#include "GBS_Emu.h"

#define GBS_HEADER_SIZE 0x70
#define MEM_SIZE		65536

#define PANIC(msg)		\
{						\
	printf("%s\n",msg);	\
	return false;		\
}						\

#define PANIC2(msg)			\
{							\
	strcpy(errorcode,msg);	\
	printf("%s\n",msg);		\
	return false;			\
}							\

byte default_wave[16]={0x84,0x40,0x43,0xAA,0x2D,0x78,0x92,0x3C,0x60,0x59,0x59,0xB0,0x34,0xB8,0x2E,0xDA};

// Gb_Env class functions from Game Music Emu

inline int Gb_Env::reload_env_timer()
{
	int raw = regs [2] & 7;
	env_delay = (raw ? raw : 8);
	vp = raw&7;
	va = raw&8;
	vc = raw&7;
	return raw;
}

void Gb_Env::clock_envelope()
{
	if ( env_enabled && --env_delay <= 0 && reload_env_timer() )
	{
		int v = volume + (regs [2] & 0x08 ? +1 : -1);
		if ( 0 <= v && v <= 15 )
			volume = v;
		else{
			env_enabled = false;
			volume = (regs [2] & 0x08 ? 15 : 0);
		}
	}

	if(vp && !(--vc)){
		vc = vp;
		if(va){
			vol++;
			if(vol>15) vol=15;
		}else{
			vol--;
			if(vol<0) vol=0;
		}
	}
}

inline void Gb_Env::zombie_volume( int old, int data )
{
	int v = vol;
	int mode = 5;

	if ( mode == 5 )
	{
		// CGB-05 behavior, very close to AGB behavior as well
		if ( (old ^ data) & 8 )
		{
			if ( !(old & 8) )
			{
				v++;
				if ( old & 7 )
					v++;
			}
			
			v = 16 - v;
		}
		else if ( (old & 0x0F) == 8 )
		{
			v++;
		}
	}
	else
	{
		// CGB-04&02 behavior, very close to MGB behavior as well
		if ( !(old & 7) && env_enabled )
			v++;
		else if ( !(old & 8) )
			v += 2;
		
		if ( (old ^ data) & 8 )
			v = 16 - v;
	}
	vol = v & 0x0F;
}

bool Gb_Env::write_register( int frame_phase, int reg, int old, int data )
{
	int const max_len = 64;
	
	switch ( reg )
	{
	/*case 1:
		length_ctr = max_len - (data & (max_len - 1));
		break;*/
	
	case 2:
		// DAC enable/disable

		if (!(data&0xF8)){
			vol = 0;
			env_enabled = false;
			break;
		}

		zombie_volume( old, data );

		if ( (data & 7) && env_delay == 8 )
		{
			vol = data>>4;
			vp = data&7;
			va = data&8;
			vc = data;
			env_delay = 1;
			clock_envelope(); // TODO: really happens at next length clock
		}
		break;
	
	case 4:
		if ( data&0x80 )
		{
			volume = regs[2] >> 4;
			vol = regs[2] >> 4;
			reload_env_timer();
			env_enabled = true;
			/*if ( frame_phase == 7 )
				env_delay++;*/
			if (!(regs[2]&0xF8)){
				vol = 0;
				env_enabled = false;
			}
			return true;
		}
	}

	regs[reg] = data;

	return false;
}


CGBS_Emu::CGBS_Emu()
{
	memset(this,0,sizeof(CGBS_Emu));
	data=new unsigned char[MEM_SIZE];
	//op_count=new int[256];
	memset(data,0,MEM_SIZE);
	data[0xFF11]=0xBF;
	data[0xFF16]=0x3F;
	data[0xFF1B]=0xFF;
	data[0xFF1C]=0x9F;
	data[0xFF25]=0xFF;
	memcpy(&data[0xFF30],default_wave,16);
	memset(&data[0xFF70],0xFF,16);
	memset(&sweeped,0,19);
}

CGBS_Emu::~CGBS_Emu()
{
	Cleanup();
}

void CGBS_Emu::Cleanup()
{
	if(bdata) delete[] bdata;
	memset(data,0,MEM_SIZE);
	data[0xFF11]=0x3F;
	data[0xFF16]=0x3F;
	data[0xFF1B]=0xFF;
	data[0xFF1C]=0x9F;
	memcpy(&data[0xFF30],default_wave,16);
	memset(&data[0xFF70],0xFF,16);
	memset(&sweeped,0,19);
	memset(Env,0,sizeof(Env));

	af.w = 0;
	bc.w = 0;
	de.w = 0;
	hl.w = 0;
}

bool CGBS_Emu::LoadFile(char* filename)
{
	fp_src = fopen(filename,"rb"); if(!fp_src) PANIC2("Couldn't open/find source file! :(\n");
	//fp_log = fopen("GBS_load.log","w"); if(!fp_log) PANIC2("Couldn't create log file! :(\n");

	fseek(fp_src,0,SEEK_END); src_size=ftell(fp_src); fseek(fp_src,0,SEEK_SET);

	if(!ReadHeader()) return false;
	if(!ReadData()) return false;

	fclose(fp_src);
	//fclose(fp_log);
	fp_src=NULL;
	//fp_log=NULL;
	return true;
}

bool CGBS_Emu::ReadHeader()
{
	if(src_size<GBS_HEADER_SIZE) PANIC2("GBS file too small! :(");
	fread(&Info,GBS_HEADER_SIZE,1,fp_src);

	if(Info.file_id[0]!='G') PANIC2("This is not a valid GBS file. ;)");
	if(Info.file_id[1]!='B') PANIC2("This is not a valid GBS file. ;)");
	if(Info.file_id[2]!='S') PANIC2("This is not a valid GBS file. ;)");
	if(Info.version>1)		 PANIC2("Unsupported GBS version. :(");

	Info.sp_init -= 2;

	//LogHeader();
	return true;
}

bool CGBS_Emu::ReadData()
{
	int read_size=src_size-ftell(fp_src);
	if(read_size+Info.load_addr > 0x8000){
		if(Info.load_addr>=0x4000) PANIC2("Unsupported ROM layout! :(\n");
		is_switched=true;
		fread(&data[Info.load_addr],1,0x4000-Info.load_addr,fp_src);
		read_size=src_size-ftell(fp_src);
		n_banks=(read_size+0x3FFF)>>14;
		bdata=new byte[n_banks<<14];
		memset(bdata,0,n_banks<<14);
		fread(bdata,1,read_size,fp_src);
		memcpy(&data[0x4000],bdata,0x4000);
		c_bank=0;
	}else{
		is_switched=false;
		fread(&data[Info.load_addr],1,read_size,fp_src);
	}

	//LogData();
	return true;
}

void CGBS_Emu::LogHeader()
{
	fprintf(fp_log,"GBS HEADER:\n\n");
	fprintf(fp_log,"Version number: %i\n",Info.version);
	fprintf(fp_log,"Number of songs: %i\n",Info.n_songs);
	fprintf(fp_log,"First song: %i\n",Info.first);
	fprintf(fp_log,"Load address: $%04X\n",Info.load_addr);
	fprintf(fp_log,"Init address: $%04X\n",Info.init_addr);
	fprintf(fp_log,"Play address: $%04X\n",Info.play_addr);
	fprintf(fp_log,"Stack pointer: $%04X\n",Info.sp_init);
	fprintf(fp_log,"Timer modulo: $%02X\n",Info.tmod);
	fprintf(fp_log,"Timer control: $%02X\n",Info.tctl);

	fprintf(fp_log,"Title: %s\n",Info.title);
	fprintf(fp_log,"Author: %s\n",Info.author);
	fprintf(fp_log,"Copyright: %s\n",Info.ccccc);
}

void CGBS_Emu::LogData()
{
	fprintf(fp_log,"\nGBS DATA:\n\n");
	fprintf(fp_log,"Data size: %i\n\n",src_size-GBS_HEADER_SIZE);
}

void CGBS_Emu::Clock64()
{
	Env[0].clock_envelope();
	Env[1].clock_envelope();
	Env[2].clock_envelope();

	if((data[0xFF14]&0x40) && sq1_lc){
		sq1_lc-=4;
		if(sq1_lc<1){
			sq1_lc=0;
			sq1_vol=0;
			Env[0].volume = 0;
			Env[0].env_enabled = false;
			sq1_vp=0;
		}
	}

	if((data[0xFF19]&0x40) && sq2_lc){
		sq2_lc-=4;
		if(sq2_lc<1){
			sq2_lc=0;
			sq2_vol=0;
			Env[1].volume = 0;
			Env[1].env_enabled = false;
			sq2_vp=0;
		}
	}

	if((data[0xFF1E]&0x40) && wave_lc>0){
		wave_lc-=4;
		if(wave_lc<1){
			wave_lc=0;
			data[0xFF1C]=0;
		}
	}

	if((data[0xFF23]&0x40) && noise_lc){
		noise_lc-=4;
		if(noise_lc<1){
			noise_lc=0;
			noise_vol=0;
			Env[2].volume = 0;
			Env[2].env_enabled = false;
			noise_vp=0;
		}
	}
}