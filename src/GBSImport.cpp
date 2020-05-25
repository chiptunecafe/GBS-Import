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
#include <cmath>
#include <cstdlib>
#include "ft_stuff.h"
#include "GBS_Emu.h"
#include "GBSImport.h"

#define PLAYBACK_LENGTH 7200
#define NOTE_COUNT		96
#define IMPORT_CHANNELS 4

#define PANIC(msg)		\
{						\
	printf("%s\n",msg);	\
	return false;		\
}

extern void set_progress(int value);

CGBS_Emu* pGBS = new CGBS_Emu();
CFTMFile* pFTM = new CFTMFile();

int noise_ntable[]={
	0,  1,  2,  3,  4,  6,  9, 11, 13, 14, 15, 15, 15, 15, 15, 15,
	1,  2,  3,  4,  6,  9, 11, 13, 14, 15, 15, 15, 15, 15, 15, 15,
	2,  3,  4,  6,  9, 11, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15,
	2,  3,  5,  8, 10, 12, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	3,  4,  6,  9, 11, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	3,  4,  7, 10, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	3,  5,  8, 10, 12, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	4,  5,  8, 11, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
};

int quadlog[8] = {12,16,20,22,24,25,26,27}; // Fancy term for 4 times the 2-logarithm of the divisor
int old_noisetable[] = {  
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,
	14,13,13,13,12,12,12,11,10,10,9,9,9,8,8,8,
	7,7,7,6,6,6,5,5,5,4,4,4,3,3,3,2,
	2,2,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
}; // What an ugly table! :P

int gb_vtable[4]={0,11,6,3};

unsigned int freqtable[NOTE_COUNT];	// For 2A03/MMC5/VRC6
unsigned int n163table[NOTE_COUNT];	// For N163

byte wtable[512][16];

int chmap[6]={0,1,3,5,9,9};

int w_count=0;
int silence_count=0;

void init_tables()
{
	double clock_ntsc = 1789773 / 16.0;
	double clock_gb = 4194304 / 32.0;
	double BASE_FREQ = 32.7032;
	double Freq;
	double Pitch;

	memset(wtable,0,256);

	for (int i=0;i<NOTE_COUNT;i++){
		Freq = BASE_FREQ * pow(2.0,i/12.0); // Freq in Hz
		Pitch = (clock_gb/Freq)+0.5;
		freqtable[i] = (unsigned int)Pitch;

		//N163
		Pitch = (Freq * 1 * 983040.0) / clock_ntsc;
		n163table[i] = (unsigned int)(Pitch) / 4;
		if(n163table[i]>0xFFFF)
			n163table[i]=0xFFFF;
	}
}

void GetNote(int Register, int& Note, int& Octave, int& Pitch)
{
    if (Register==0){
        Note=FT_NOTE_CUT; Octave=0; Pitch=0;
		return;
    }

    int MidiNote = 0;
    int MinDiff = 9999999;
    for (unsigned int i=1; i < 95; i++)
    {
        int Val = freqtable[i];
        int Diff = Val - Register;
        int AbsDiff = (Diff<0)? -Diff: Diff;
        if (AbsDiff < MinDiff)
        {
            MinDiff = AbsDiff;
            MidiNote = i;
            Pitch = 128+Diff;
        }
    }

    Note = (MidiNote%12)+1;
    Octave = MidiNote/12;
}

void GetN163Note(int Register, int& Note, int& Octave, int& Pitch)
{
    if (Register==0){
        Note=FT_NOTE_CUT; Octave=0; Pitch=0;
		return;
    }

    int MidiNote = 0;
    int MinDiff = 9999999;
	double clock_ntsc = 1789773 / 16.0;
	double clock_gb = 4194304 / 32.0;
	int N163Reg = (int)((((clock_gb/Register)*1*983040.0)/clock_ntsc)/4);

    for (unsigned int i=1; i<95; i++)
    {
        int Val = n163table[i];
        int Diff = Val - N163Reg;
        int AbsDiff = (Diff<0)? -Diff: Diff;
        if (AbsDiff < MinDiff)
        {
            MinDiff = AbsDiff;
            MidiNote = i;
            Pitch = 128+(Diff/4);
        }
    } 

    Note = (MidiNote%12)+1;
    Octave = MidiNote/12;
}

bool GBS2FTM(CGBS_Emu* pGBS, CFTMFile* pFTM, int length, int track, bool nosilence)
{
	int i,j,k;
	int c_order=0;
	int c_pattern=0;
	int c_row=0;
	int gb_vol=0;
	int gb_duty=0;
	int gb_pitch=0;
	int gb_sweep=0;
	int c_ftnote=0;
	int c_ftoct=0;
	int c_ftinst=0;
	int c_ftvol=0;
	int c_ftpitch=0;
	int c_ftwave=0;
	int n_items=0;
	int p_pitch=0;

	int p_ftnote[IMPORT_CHANNELS]={0};
	int p_ftoct[IMPORT_CHANNELS]={0};
	int p_ftinst[IMPORT_CHANNELS]={64,64,64,64};
	int p_ftvol[IMPORT_CHANNELS]={16,16,16,16};	
	int p_ftduty[IMPORT_CHANNELS]={0};
	int p_ftpitch[IMPORT_CHANNELS]={0};
	int p_ftsweep=0;

	pFTM->expansion=0x10;
	pFTM->channels=6;
	pFTM->N163_channels=1;
	pFTM->n_frames=1;
	pFTM->n_patterns=IMPORT_CHANNELS*pFTM->n_frames;
	pFTM->speed=1;
	pFTM->tempo=150;
	pFTM->p_length=256;
	pFTM->ch_fx[0]=2;
	pFTM->ch_fx[1]=1;
	//pFTM->ch_fx[5]=1;

	pFTM->n_instruments=1;

	//int Note, Octave, Pitch;

	if(!pGBS->Init(track)) return false;

	memcpy(wtable[w_count],&(pGBS->data[0xFF30]),16);

	for(i=0;i<length;i++)
	{
		bool w_new = true;
		bool silence = true;
		bool gb_sweeped = false;
		bool result = pGBS->PlayOnce();
		if(!result) return false;

		for(j=0;j<w_count;j++){
			for(k=0;k<16;k++){
				if(wtable[j][k]!=pGBS->data[0xFF30+k]) break;
			}
			if(k==16){
				c_ftwave=j;
				w_new=false;
				break;
			}
		}

		if(w_new){
			c_ftwave = w_count;
			memcpy(wtable[w_count],&(pGBS->data[0xFF30]),16);
			w_count++;
		}

		c_pattern = (c_order*IMPORT_CHANNELS);

		gb_vol   = pGBS->Env[0].vol; //pGBS->sq1_vol;
		gb_duty  = pGBS->data[0xFF11]>>6;
		gb_pitch = (((pGBS->data[0xFF14]&7)<<8) | pGBS->data[0xFF13]);
		gb_pitch = 2048 - gb_pitch;
		gb_sweep = pGBS->data[0xFF10];
		gb_sweeped = pGBS->sweeped? true: false;
		pGBS->sweeped = 0;
		if((gb_pitch!=p_pitch) && (gb_sweep&0x77)) gb_sweeped = true;
		GetNote(gb_pitch, c_ftnote, c_ftoct, c_ftpitch);

		if((gb_sweep&0x8) && ((gb_sweep&0x7)>0)) gb_sweep--;

		if(silence && gb_vol && c_ftnote != FT_NOTE_CUT) silence=false;

		pFTM->Pattern[c_pattern].Item[n_items].row		  = c_row;
		pFTM->Pattern[c_pattern].Item[n_items].note       = (c_ftnote==p_ftnote[0] && c_ftoct==p_ftoct[0])? 0: c_ftnote;
		pFTM->Pattern[c_pattern].Item[n_items].octave     = (c_ftnote==p_ftnote[0] && c_ftoct==p_ftoct[0])? 0: c_ftoct;
		pFTM->Pattern[c_pattern].Item[n_items].instrument = (c_ftnote==p_ftnote[0] && c_ftoct==p_ftoct[0])? 64: 0;
		pFTM->Pattern[c_pattern].Item[n_items].volume     = (gb_vol==p_ftvol[0])? 16: gb_vol;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[0]  = (c_ftpitch==p_ftpitch[0])? 0: EF_PITCH;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[1]  = (c_ftpitch==p_ftpitch[0])? 0: c_ftpitch;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[2]  = (gb_duty==p_ftduty[0])? 0: EF_DUTY_CYCLE;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[3]  = (gb_duty==p_ftduty[0])? 0: gb_duty;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[4]  = (!gb_sweeped || (gb_sweep==0 && p_ftsweep==gb_sweep))? 0: (gb_sweep&0x8)? EF_SWEEPDOWN: EF_SWEEPUP;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[5]  = (!gb_sweeped || (gb_sweep==0 && p_ftsweep==gb_sweep))? 0: gb_sweep&0x77;

		p_pitch = gb_pitch;
		p_ftnote[0] = c_ftnote;
		p_ftoct[0] = c_ftoct;
		p_ftvol[0] = gb_vol;
		p_ftduty[0] = gb_duty;
		p_ftpitch[0] = c_ftpitch;
		p_ftsweep = gb_sweep;

		c_pattern = (c_order*IMPORT_CHANNELS)+1;

		gb_vol   = pGBS->Env[1].vol; //pGBS->sq2_vol;
		gb_duty  = pGBS->data[0xFF16]>>6;
		gb_pitch = (((pGBS->data[0xFF19]&7)<<8) | pGBS->data[0xFF18]);
		gb_pitch = 2048 - gb_pitch;
		GetNote(gb_pitch, c_ftnote, c_ftoct, c_ftpitch);

		if(silence && gb_vol && c_ftnote != FT_NOTE_CUT) silence=false;

		pFTM->Pattern[c_pattern].Item[n_items].row		  = c_row;
		pFTM->Pattern[c_pattern].Item[n_items].note       = (c_ftnote==p_ftnote[1] && c_ftoct==p_ftoct[1])? 0: c_ftnote;
		pFTM->Pattern[c_pattern].Item[n_items].octave     = (c_ftnote==p_ftnote[1] && c_ftoct==p_ftoct[1])? 0: c_ftoct;
		pFTM->Pattern[c_pattern].Item[n_items].instrument = (c_ftnote==p_ftnote[1] && c_ftoct==p_ftoct[1])? 64: 0;
		pFTM->Pattern[c_pattern].Item[n_items].volume     = (gb_vol==p_ftvol[1])? 16: gb_vol;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[0]  = (c_ftpitch==p_ftpitch[1])? 0: EF_PITCH;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[1]  = (c_ftpitch==p_ftpitch[1])? 0: c_ftpitch;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[2]  = (gb_duty==p_ftduty[1])? 0: EF_DUTY_CYCLE;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[3]  = (gb_duty==p_ftduty[1])? 0: gb_duty;

		p_ftnote[1]=c_ftnote;
		p_ftoct[1]=c_ftoct;
		p_ftvol[1]=gb_vol;
		p_ftduty[1]=gb_duty;
		p_ftpitch[1]=c_ftpitch;

		c_pattern = (c_order*IMPORT_CHANNELS)+2;

		gb_vol   = pGBS->Env[2].vol; //pGBS->noise_vol;
		gb_duty  = pGBS->data[0xFF22]&8;
		gb_pitch = ((pGBS->data[0xFF22]<<4)|(pGBS->data[0xFF22]>>4))&0x7F;

		int noisenote=16+old_noisetable[4*(gb_pitch&15)+quadlog[gb_pitch>>4]];

		if(noise_ntable[gb_pitch]==0x99 || !(pGBS->data[0xFF25]&0x88)){
			c_ftnote=FT_NOTE_CUT;
			c_ftoct=0;
		} else {
			c_ftnote=((31-noise_ntable[gb_pitch])%12)+1;
			c_ftoct=(31-noise_ntable[gb_pitch])/12;
		}

		if(silence && gb_vol && c_ftnote != FT_NOTE_CUT) silence=false;

		pFTM->Pattern[c_pattern].Item[n_items].row		  = c_row;
		pFTM->Pattern[c_pattern].Item[n_items].note       = (c_ftnote==p_ftnote[2] && c_ftoct==p_ftoct[2])? 0: c_ftnote;
		pFTM->Pattern[c_pattern].Item[n_items].octave     = (c_ftnote==p_ftnote[2] && c_ftoct==p_ftoct[2])? 0: c_ftoct;
		pFTM->Pattern[c_pattern].Item[n_items].instrument = (c_ftnote==p_ftnote[2] && c_ftoct==p_ftoct[2])? 64: 0;
		pFTM->Pattern[c_pattern].Item[n_items].volume     = (gb_vol==p_ftvol[2])? 16:gb_vol;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[0]  = (gb_duty==p_ftduty[2])? 0: EF_DUTY_CYCLE;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[1]  = (gb_duty==p_ftduty[2])? 0: gb_duty>>3;


		p_ftnote[2]=c_ftnote;
		p_ftoct[2]=c_ftoct;
		p_ftvol[2]=gb_vol;
		p_ftduty[2]=gb_duty;

		c_pattern = (c_order*IMPORT_CHANNELS)+3;

		gb_vol   = ((pGBS->data[0xFF26]&0x4)||(pGBS->data[0xFF25]&0x44))? (pGBS->data[0xFF1C]>>5)&3: 0;
		gb_pitch = (((pGBS->data[0xFF1E]&7)<<8) | pGBS->data[0xFF1D]);
		gb_pitch = 2048 - gb_pitch;
		GetNote(gb_pitch, c_ftnote, c_ftoct, c_ftpitch);

		if(silence && gb_vol && c_ftnote != FT_NOTE_CUT) silence=false;

		pFTM->Pattern[c_pattern].Item[n_items].row		  = c_row;
		pFTM->Pattern[c_pattern].Item[n_items].note       = (c_ftnote==p_ftnote[3] && c_ftoct==p_ftoct[3])? 0: c_ftnote;
		pFTM->Pattern[c_pattern].Item[n_items].octave     = (c_ftnote==p_ftnote[3] && c_ftoct==p_ftoct[3])? 0: c_ftoct;
		pFTM->Pattern[c_pattern].Item[n_items].instrument = (c_ftnote==p_ftnote[3] && c_ftoct==p_ftoct[3] && p_ftduty[3]==c_ftwave)? 64: (c_ftwave/16)+1;
		pFTM->Pattern[c_pattern].Item[n_items].volume     = (gb_vol==p_ftvol[3])? 16: gb_vtable[gb_vol];
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[0]  = (p_ftduty[3]==c_ftwave)? 0: EF_DUTY_CYCLE;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[1]  = (p_ftduty[3]==c_ftwave)? 0: c_ftwave&15;

		p_ftnote[3]=c_ftnote;
		p_ftoct[3]=c_ftoct;
		p_ftvol[3]=gb_vol;
		p_ftduty[3]=c_ftwave;

		n_items++;
		c_row++;

		while(c_row>=256){			
			for(j=0;j<IMPORT_CHANNELS;j++){
				c_pattern = (c_order*IMPORT_CHANNELS)+j;
				pFTM->Pattern[c_pattern].items   = n_items;
				pFTM->Pattern[c_pattern].track   = 0;
				pFTM->Pattern[c_pattern].channel = chmap[j];
				pFTM->Pattern[c_pattern].index   = c_order;
			}
			n_items=0;
			c_row-=256;
			c_order++;
			pFTM->n_frames++;
			pFTM->n_patterns += IMPORT_CHANNELS;
		}
		pGBS->Clock64();
		
		if(silence){
			silence_count++;
			if(silence_count>=300 && nosilence) break;
		} else silence_count=0;
	}

	if(!w_count) w_count=1;
	
	pFTM->N163_instruments=(w_count+15)/16;	

	for(i=0;i<w_count;i++){
		int c_inst=i/16;
		if((i&15)==0){
			pFTM->Inst_N163[c_inst].index=c_inst+1;
			pFTM->Inst_N163[c_inst].wave_count=min(w_count-i,16);
		}
		for(j=0;j<16;j++){
			pFTM->Inst_N163[c_inst].wave_data[i&15][2*j]=wtable[i][j]>>4;
			pFTM->Inst_N163[c_inst].wave_data[i&15][2*j+1]=wtable[i][j]&15;
		}
	}

	for(i=0;i<IMPORT_CHANNELS;i++){
		c_pattern = (c_order*IMPORT_CHANNELS)+i;
		pFTM->Pattern[c_pattern].Item[n_items].row       = c_row;
		pFTM->Pattern[c_pattern].Item[n_items].note      = FT_NOTE_CUT;
		pFTM->Pattern[c_pattern].Item[n_items].octave    = 0;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[0] = EF_HALT;
		pFTM->Pattern[c_pattern].Item[n_items].fxdata[1] = 0;
		pFTM->Pattern[c_pattern].items   = n_items+1;
		pFTM->Pattern[c_pattern].track   = 0;
		pFTM->Pattern[c_pattern].channel = chmap[i];
		pFTM->Pattern[c_pattern].index   = c_order;
	}

	for(i=0;i<pFTM->n_frames;i++){
		memset(pFTM->fdata[i],0,6);
		for(j=0;j<IMPORT_CHANNELS;j++){
			if(chmap[j]==9) continue;
			pFTM->fdata[i][chmap[j]]=i;
		}
	}
	return true;
}

bool GBSImport::Scan(char* filename)
{
	//init_tables();
	return pGBS->LoadFile(filename);
}

bool GBSImport::Import(char* gbsname, int track, int seconds, bool alltracks, bool nosilence)
{
	char ftmname[256];
	
	init_tables();

	if(!alltracks){
		memset(ftmname,0,256);
		strcpy(ftmname,gbsname);
		ftmname[strlen(ftmname)-4]=0;
		strcat(ftmname,".ftm");

		if(!pGBS->LoadFile(gbsname))						 return false;
		if(!GBS2FTM(pGBS,pFTM,seconds*60,track-1,nosilence)) return false;
		if(!pFTM->Optimize())								 return false;
		if(!pFTM->SaveFile(ftmname))						 return false;
	} else {
		if(!pGBS->LoadFile(gbsname))						 return false;
		int n_tracks = pGBS->Info.n_songs;
		
		FILE* fp_log=fopen("GBSImport.log","w"); if(!fp_log) PANIC("Couldn't create log file! :(\n");

		fprintf(fp_log,"Attempting to import %s\n\n",gbsname);

		for(int i=0;i<n_tracks;i++){
			memset(ftmname,0,256);
			strcpy(ftmname,gbsname);
			ftmname[strlen(ftmname)-4]=0;
			sprintf(ftmname,"%s_track%i.ftm",ftmname,i+1);

			set_progress((i*100)/n_tracks);

			if(!pGBS->LoadFile(gbsname))					 return false;
			if(!GBS2FTM(pGBS,pFTM,seconds*60,i,nosilence)){
				fprintf(fp_log,"Track %i failed to import: %s\n",i+1,pGBS->errorcode);
				w_count=0;
				pGBS->Cleanup();
				continue;
			}

			if(pGBS->Info.tctl==7 && pGBS->Info.tmod==192){
				pFTM->tempo = 600;
				pFTM->e_speed = 240;
			}

			if(pGBS->Info.tctl==132 && pGBS->Info.tmod==128){
				pFTM->tempo = 160;
				pFTM->e_speed = 64;
			}

			if(!pFTM->Optimize())							 return false;
			if(!pFTM->SaveFile(ftmname))					 return false;
			fprintf(fp_log,"Track %i imported successfully!\n",i+1);
			w_count=0;
			silence_count=0;
			pGBS->Cleanup();
		}
	}

	set_progress(100);

	return true;
}

char* GBSImport::GetError()
{
	return pGBS->errorcode;
}