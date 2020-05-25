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

#include <stdio.h>
#include <string>
#include <math.h>
#include "ft_stuff.h"
#include "FTMFile.h"

#define MAGIC_NUMBER	110250

//TODO: Allocate stuff dynamically ;)

byte saw_wave[32]={0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15};
byte dummy_seq[32]={15,15,14,14,13,13,12,12,11,11,10,10,9,9,8,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0};

stSequence::stSequence()
{
	index=0;
	type=0;
	length=0;
	loop=0xFFFFFFFF;
	release=0xFFFFFFFF;
	setting=0;
	memset(data,0,256);
}

Instrument_2A03::Instrument_2A03()
{
	index=0;
	type=1;
	seq_count=SEQ_COUNT;
	memset(seq_data,0,2*SEQ_COUNT);
	memset(DPCM_data,0,288); //Earrape alert! :(
	name_length=0x0E;
	sprintf(name,"New instrument");
}

Instrument_VRC6::Instrument_VRC6()
{
	index=0;
	type=2;
	seq_count=SEQ_COUNT;
	memset(seq_data,0,2*SEQ_COUNT);
	name_length=0x0E;
	sprintf(name,"New instrument");
}

Instrument_VRC7::Instrument_VRC7()
{
	index=0;
	type=3;
	patch=4;
	memset(data,0,8);
	name_length=0x0E;
	sprintf(name,"New instrument");
}

Instrument_FDS::Instrument_FDS()
{
	index=0;
	type=4;
	memset(wave,0,64);
	memset(mtable,0,32);
	mspeed=0;
	mdepth=0;
	mdelay=0;
	name_length=0x0E;
	sprintf(name,"New instrument");
}

Instrument_N163::Instrument_N163()
{
	index=0;
	type=5;
	seq_count=SEQ_COUNT;
	memset(seq_data,0,2*SEQ_COUNT);
	wave_size=32;
	wave_pos=0;
	wave_count=1;
	memset(wave_data,0,16*32);
	memcpy(wave_data,saw_wave,32);
}

FTItem::FTItem()
{
	memset(this,0,sizeof(FTItem));
	instrument=64;
	volume=16;
}

FTPattern::FTPattern()
{
	track=0;
	channel=0;
	index=0;
	items=0;
}

CFTMFile::CFTMFile()
{
	memset(file_id,0,24);
	sprintf(file_id,"FamiTracker Module");
	file_version=0x440;

	memset(pr_id,0,24);
	sprintf(pr_id,"PARAMS");
	pr_version=6;
	pr_size=0x1D;
	expansion=0;
	channels=5;
	machine=0;
	e_speed=0;
	v_style=1;
	highlight1=4;
	highlight2=16;
	N163_channels=0;
	s_split=32;

	memset(nf_id,0,24);
	sprintf(nf_id,"INFO");
	nf_version=1;
	nf_size=0x60;
	memset(author,0,32);
	memset(title,0,32);
	memset(ccccc,0,32);

	memset(he_id,0,24);
	sprintf(he_id,"HEADER");
	he_version=3;
	he_size=10+2*channels;
	n_tracks=0;
	ch_id[0]=0; ch_id[1]=1; ch_id[2]=2; ch_id[3]=3; ch_id[4]=4;
	memset(ch_fx,0,16);

	memset(i0_id,0,24);
	sprintf(i0_id,"INSTRUMENTS");
	i0_version=6;
	i0_size=329; 
	n_instruments=0;
	FDS_instruments=0;
	VRC6_instruments=0;
	VRC7_instruments=0;
	N163_instruments=0;

	memset(s0_id,0,24);
	sprintf(s0_id,"SEQUENCES");
	s0_version=6;
	s0_size=4;
	n_sequences=0;
	n_VRC6_sequences=0;
	n_N163_sequences=0;

	memset(fr_id,0,24);
	sprintf(fr_id,"FRAMES");
	fr_version=3;
	fr_size=27;
	n_frames=1;
	speed=6;
	tempo=150;
	p_length=64;
	memset(fdata[0],0,channels);
	memset(fdata[1],0,channels);

	memset(pt_id,0,24);
	sprintf(pt_id,"PATTERNS");
	pt_version=5;
	pt_size=0;
	n_patterns=0;

	memset(ds_id,0,24);
	sprintf(ds_id,"DPCM SAMPLES");
	ds_version=1;
	ds_size=1;	
	n_samples=0;

	memset(n163s_id,0,24);
	sprintf(n163s_id,"SEQUENCES_N163");
	n163s_version=1;
	n163s_size=0x38;
	n_N163_sequences=0;

	memset(vrc6s_id,0,24);
	sprintf(vrc6s_id,"SEQUENCES_VRC6");
	vrc6s_version=6;
	vrc6s_size=0x38;
	n_VRC6_sequences=0;
}

bool CFTMFile::LoadFile(char* filename)
{
	//fopen_s(&fp_log,"FTM_load.log","w"); if(fp_log==NULL) {printf("Error! Could not create log file. :(\n"); return false;}
	fopen_s(&fp_src,filename,"rb");		 if(fp_src==NULL) {printf("Error! Source file not found. :(\n"); return false;}
	
	fseek(fp_src,0,SEEK_END); src_size=ftell(fp_src); fseek(fp_src,0,SEEK_SET); //Get file size

	if(!ReadID())			return false;
	if(!ReadParams())		return false;
	if(!ReadInfo())			return false;
	if(!ReadHeader())		return false;
	if(!ReadInstruments())	return false;
	if(!ReadSequences())	return false;
	if(!ReadFrames())		return false;
	if(!ReadPatterns())		return false;

	//if(!LogStuff())			return false;

	printf("Current file position: 0x%X\n",ftell(fp_src));

	fclose(fp_src);
	//fclose(fp_log);
	return true;
}

bool CFTMFile::SaveFile(char* filename)
{
	//fopen_s(&fp_log,"FTM_save.log","w"); if(fp_log==NULL) {printf("Error! Could not create log file. :(\n"); return false;}
	fopen_s(&fp_dst,filename,"wb");		 if(fp_dst==NULL) {printf("Error! Source file not found. :(\n"); return false;}

	if(!WriteID())			  return false;
	if(!WriteParams())		  return false;
	if(!WriteInfo())		  return false;
	if(!WriteHeader())		  return false;
	if(!WriteInstruments())	  return false;
	if(!WriteSequences())	  return false;
	if(!WriteFrames())		  return false;
	if(!WritePatterns())	  return false;
	if(!WriteSamples())		  return false;
	if(!WriteVRC6Sequences()) return false;
	if(!WriteN163Sequences()) return false;
	fwrite("END",1,3,fp_dst);

	//if(!LogStuff())			return false;

	fclose(fp_dst);
	//fclose(fp_log);
	return true;
}

bool CFTMFile::Optimize()
{
	FTPattern Temp;
	FTItem Prev;

	for(int i=0;i<n_patterns;i++){
		Temp.track=Pattern[i].track;
		Temp.channel=Pattern[i].channel;
		Temp.index=Pattern[i].index;
		Temp.items=0;

		for(int j=0;j<p_length;j++){
			bool non_empty=false;
			bool new_item=true;

			if(Pattern[i].Item[j].note!=0) non_empty=true;
			if(Pattern[i].Item[j].instrument!=64) non_empty=true;
			if(Pattern[i].Item[j].volume!=16) non_empty=true;
			for(int k=0;k<8;k+=2){
				if(Pattern[i].Item[j].fxdata[k]!=0) non_empty=true;
			}

			if(!non_empty) continue;

			if(Pattern[i].Item[j].note!=Prev.note) new_item=true;
			if(Pattern[i].Item[j].octave!=Prev.octave) new_item=true;
			if(Pattern[i].Item[j].instrument!=Prev.instrument) new_item=true;
			if(Pattern[i].Item[j].volume!=Prev.volume) new_item=true;
			for(int k=0;k<8;k++){
				if(Pattern[i].Item[j].fxdata[k]!=Prev.fxdata[k]) new_item=true;
			}

			if(!new_item) continue;
			
			memcpy(&Prev,&Pattern[i].Item[j],sizeof(FTItem));
			memcpy(&Temp.Item[Temp.items++],&Prev,sizeof(FTItem));

		}
		memcpy(&Pattern[i],&Temp,sizeof(FTPattern));
	}
	return true;
}

bool CFTMFile::ExportTXT(char* filename)
{
	int i,j,k;
	FTItem Cell;

	fopen_s(&fp_dst,filename,"w"); if(fp_dst==NULL) {printf("Error! Could not create log file. :(\n"); return false;}
	fprintf(fp_dst,"Expansion: 0x%X\n",expansion);
	fprintf(fp_dst,"Channels: %i\n",channels);
	fprintf(fp_dst,"N163 channels: %i\n",N163_channels);
	fprintf(fp_dst,"2A03 instruments: %i\n",n_instruments);
	fprintf(fp_dst,"VRC6 instruments: %i\n",VRC6_instruments);
	fprintf(fp_dst,"VRC7 instruments: %i\n",VRC7_instruments);
	fprintf(fp_dst,"FDS instruments: %i\n",FDS_instruments);
	fprintf(fp_dst,"N163 instruments: %i\n",N163_instruments);
	fprintf(fp_dst,"2A03 sequences: %i\n",n_sequences);
	fprintf(fp_dst,"Frames: %i\n",n_frames);
	fprintf(fp_dst,"Patterns: %i\n",n_patterns);
	fprintf(fp_dst,"Samples: %i\n",n_samples);

	fprintf(fp_dst,"\nINSTRUMENTS:\n\n");

	for(i=0;i<n_instruments;i++){
		fprintf(fp_dst,"Instrument %i index: %i\n",i,Inst_2A03[i].index);
		fprintf(fp_dst,"Instrument %i type: %i\n",i,Inst_2A03[i].type);
		fprintf(fp_dst,"Instrument %i sequence count: %i\n",i,Inst_2A03[i].seq_count);
		fprintf(fp_dst,"Instrument %i sequence data:",i);
		for(j=0;j<2*SEQ_COUNT;j++){
			fprintf(fp_dst," %02X",Inst_2A03[i].seq_data[j]);
		}
		fprintf(fp_dst,"\n");
		fprintf(fp_dst,"Instrument %i name length: %i\n",i,Inst_2A03[i].name_length);
	}

	if(expansion&0x01){
		fprintf(fp_dst,"\nVRC6 INSTRUMENTS:\n\n");

		for(i=0;i<VRC6_instruments;i++){
			fprintf(fp_dst,"VRC6 Instrument %i index: %i\n",i,Inst_VRC6[i].index);
			fprintf(fp_dst,"VRC6 Instrument %i type: %i\n",i,Inst_VRC6[i].type);
			fprintf(fp_dst,"VRC6 Instrument %i sequence count: %i\n",i,Inst_VRC6[i].seq_count);
			fprintf(fp_dst,"VRC6 Instrument %i sequence data:",i);
			for(j=0;j<2*SEQ_COUNT;j++){
				fprintf(fp_dst," %02X",Inst_VRC6[i].seq_data[j]);
			}
			fprintf(fp_dst,"\n");
			fprintf(fp_dst,"VRC6 Instrument %i name length: %i\n",i,Inst_VRC6[i].name_length);
		}
	}

	if(expansion&0x04){
		fprintf(fp_dst,"\nFDS INSTRUMENTS:\n\n");

		for(i=0;i<FDS_instruments;i++){
			fprintf(fp_dst,"FDS Instrument %i index: %i\n",i,Inst_FDS[i].index);
			fprintf(fp_dst,"FDS Instrument %i type: %i\n",i,Inst_FDS[i].type);
			fprintf(fp_dst,"FDS Instrument %i name length: %i\n",i,Inst_FDS[i].name_length);
			for(j=0;j<3;j++){
				fprintf(fp_dst,"FDS Instrument %i sequence %i length: %i\n",i,j,Inst_FDS[i].Seq[j].length);
			}
			fprintf(fp_dst,"\n");
		}
	}

	if(expansion&0x10){
		fprintf(fp_dst,"\nN163 INSTRUMENTS:\n\n");

		for(i=0;i<N163_instruments;i++){
			fprintf(fp_dst,"N163 Instrument %i index: %i\n",i,Inst_N163[i].index);
			fprintf(fp_dst,"N163 Instrument %i type: %i\n",i,Inst_N163[i].type);
			fprintf(fp_dst,"N163 Instrument %i name length: %i\n",i,Inst_N163[i].name_length);
			fprintf(fp_dst,"N163 Instrument %i sequence count: %i\n",i,Inst_N163[i].seq_count);
			fprintf(fp_dst,"N163 Instrument %i wave size: %i\n",i,Inst_N163[i].wave_size);
			fprintf(fp_dst,"N163 Instrument %i wave position: %i\n",i,Inst_N163[i].wave_pos);
			fprintf(fp_dst,"N163 Instrument %i wave count: %i\n",i,Inst_N163[i].wave_count);
			fprintf(fp_dst,"N163 Instrument %i sequence data:",i);
			for(j=0;j<2*SEQ_COUNT;j++){
				fprintf(fp_dst," %02X",Inst_N163[i].seq_data[j]);
			}
			fprintf(fp_dst,"\n");
			for(j=0;j<Inst_N163[i].wave_count;j++){
				fprintf(fp_dst,"N163 Instrument %i wave %i data:",i,j);
				for(k=0;k<Inst_N163[i].wave_size;k++){
					fprintf(fp_dst," %02X",Inst_N163[i].wave_data[j][k]);
				}
				fprintf(fp_dst,"\n");
			}			
			
		}
	}


	fprintf(fp_dst,"\nSEQUENCES:\n\n");

	for(i=0;i<n_sequences;i++){
		fprintf(fp_dst,"Sequence %i index: %i\n",i,Sequence[i].index);
		fprintf(fp_dst,"Sequence %i type: %i\n",i,Sequence[i].type);
		fprintf(fp_dst,"Sequence %i length: %i\n",i,Sequence[i].length);
	}

	if(expansion&0x10){
		fprintf(fp_dst,"\nN163 SEQUENCES:\n\n");

		for(i=0;i<n_sequences;i++){
			fprintf(fp_dst,"N163 Sequence %i index: %i\n",i,N163_Sequence[i].index);
			fprintf(fp_dst,"N163 Sequence %i type: %i\n",i,N163_Sequence[i].type);
			fprintf(fp_dst,"N163 Sequence %i length: %i\n",i,N163_Sequence[i].length);
		}
	}

	fprintf(fp_dst,"\nFRAMES:\n\n");

	for(i=0;i<n_frames;i++){
		fprintf(fp_dst,"%02X |",i);
		for(j=0;j<channels;j++){
			fprintf(fp_dst," %02X",fdata[i][j]);
		}
		fprintf(fp_dst,"\n");
	}

	fprintf(fp_dst,"\nPATTERNS:\n\n");

	for(i=0;i<n_patterns;i++){
		fprintf(fp_dst,"Pattern %i track: %i\n",i,Pattern[i].track);
		fprintf(fp_dst,"Pattern %i channel: %i\n",i,Pattern[i].channel);
		fprintf(fp_dst,"Pattern %i index: %i\n",i,Pattern[i].index);
		fprintf(fp_dst,"Pattern %i items: %i\n",i,Pattern[i].items);
	}

	fprintf(fp_dst,"\nSAMPLES:\n\n");

	for(i=0;i<n_samples;i++){
		fprintf(fp_dst,"Sample %i index: %i\n",i,Sample[i].index);
		fprintf(fp_dst,"Sample %i name length: %i\n",i,Sample[i].namelength);
		fprintf(fp_dst,"Sample %i data length: %i\n",i,Sample[i].datalength);
	}

	fclose(fp_dst);

	return true;

	fopen_s(&fp_dst,"patterns.log","w"); if(fp_dst==NULL) {printf("Error! Could not create log file. :(\n"); return false;}

	for(i=0;i<n_patterns;i++){
		fprintf(fp_dst,"Pattern %i track: %i\n",i,Pattern[i].track);
		fprintf(fp_dst,"Pattern %i channel: %i\n",i,Pattern[i].channel);
		fprintf(fp_dst,"Pattern %i index: %i\n",i,Pattern[i].index);
		fprintf(fp_dst,"Pattern %i items: %i\n\n",i,Pattern[i].items);
		for(j=0;j<Pattern[i].items;j++){
			memcpy(&Cell,&Pattern[i].Item[j],sizeof(FTItem));
			fprintf(fp_dst,"%02X | %c%c%c %02X %01X %c%02X %c%02X %c%02X %c%02X\n",Cell.row,ftmn2not(Cell.note),ftmn2acc(Cell.note),ftmno2oct(Cell.note,Cell.octave),Cell.instrument,Cell.volume&0xF,ftmfx(Cell.fxdata[0]),Cell.fxdata[1],ftmfx(Cell.fxdata[2]),Cell.fxdata[3],ftmfx(Cell.fxdata[4]),Cell.fxdata[5],ftmfx(Cell.fxdata[6]),Cell.fxdata[7]);
		}
		fprintf(fp_dst,"\n");
	}

	fclose(fp_dst);
	return true;
}

bool CFTMFile::ReadID()
{
	fread(&file_id,1,18,fp_src);
	file_version=read_dword(fp_src);
	return true;
}

bool CFTMFile::ReadParams()
{
	fread(&pr_id,1,16,fp_src);
	pr_version=read_dword(fp_src);
	pr_size=read_dword(fp_src);
	expansion=read_byte(fp_src);
	channels=read_dword(fp_src);
	machine=read_dword(fp_src);
	e_speed=read_dword(fp_src);
	v_style=read_dword(fp_src);
	highlight1=read_dword(fp_src);
	highlight2=read_dword(fp_src);
	if(expansion&0x10) N163_channels=read_dword(fp_src);
	s_split=read_dword(fp_src);
	return true;
}

bool CFTMFile::ReadInfo()
{
	fread(&nf_id,1,16,fp_src);
	nf_version=read_dword(fp_src);
	nf_size=read_dword(fp_src);
	fread(&author,1,32,fp_src);
	fread(&title,1,32,fp_src);
	fread(&ccccc,1,32,fp_src);
	return true;
}

bool CFTMFile::ReadHeader()
{
	fread(&he_id,1,16,fp_src);
	he_version=read_dword(fp_src);
	he_size=read_dword(fp_src);
	n_tracks=read_byte(fp_src);
	while(getc(fp_src)>0); //Skip null-terminated string
	for (int i=0;i<channels;i++){
		ch_id[i]=read_byte(fp_src);
		ch_fx[i]=read_byte(fp_src);
	}
	return true;
}

bool CFTMFile::ReadInstruments()
{
	fread(&i0_id,1,16,fp_src);
	i0_version=read_dword(fp_src);
	i0_size=read_dword(fp_src);
	n_instruments=read_dword(fp_src);
	for(int i=0;i<n_instruments;i++){
		Inst_2A03[i].index=read_dword(fp_src);
		Inst_2A03[i].type=read_byte(fp_src);
		Inst_2A03[i].seq_count=read_dword(fp_src);
		fread(Inst_2A03[i].seq_data,1,10,fp_src);
		fread(Inst_2A03[i].DPCM_data,1,288,fp_src);
		Inst_2A03[i].name_length=read_dword(fp_src);
		fread(Inst_2A03[i].name,1,Inst_2A03[i].name_length,fp_src);
	}	
	return true;
}

bool CFTMFile::ReadSequences()
{
	fread(&s0_id,1,16,fp_src);
	s0_version=read_dword(fp_src);
	s0_size=read_dword(fp_src);
	n_sequences=read_dword(fp_src); // Sequence count
	for(int i=0;i<n_sequences;i++){
		Sequence[i].index = read_dword(fp_src);
		Sequence[i].type = read_dword(fp_src);
		Sequence[i].length = read_byte(fp_src);
		Sequence[i].loop = read_dword(fp_src);
		fread(Sequence[i].data,1,Sequence[i].length,fp_src);		
	}
	for(int i=0;i<n_sequences;i++){
		Sequence[i].release = read_dword(fp_src);
		Sequence[i].setting = read_dword(fp_src);
	}
	return true;
}

bool CFTMFile::ReadFrames()
{
	fread(&fr_id,1,16,fp_src);
	fr_version=read_dword(fp_src);
	fr_size=read_dword(fp_src);
	n_frames=read_dword(fp_src);
	speed=read_dword(fp_src);
	tempo=read_dword(fp_src);
	p_length=read_dword(fp_src);
	for(int i=0;i<n_frames;i++)
		fread(fdata[i],1,channels,fp_src);
	return true;
}

bool CFTMFile::ReadPatterns()
{
	fread(&pt_id,1,16,fp_src);
	pt_version=read_dword(fp_src);
	pt_size=read_dword(fp_src);
	int pt_end=ftell(fp_src)+pt_size;
	int i=0;
	while(ftell(fp_src)<pt_end){
		Pattern[i].track=read_dword(fp_src);
		Pattern[i].channel=read_dword(fp_src);
		Pattern[i].index=read_dword(fp_src);
		Pattern[i].items=read_dword(fp_src);
		int item_size=10+2*ch_fx[Pattern[i].channel];
		for(int j=0;j<Pattern[i].items;j++){
			memset(Pattern[i].Item[j].fxdata,0,8);
			fread(&Pattern[i].Item[j],1,item_size,fp_src);
		}
		i++;
	}
	n_patterns=i;
	return true;
}

bool CFTMFile::WriteID()
{
	fwrite(&file_id,1,18,fp_dst);
	write_dword(fp_dst,file_version);
	return true;
}

bool CFTMFile::WriteParams()
{
	fwrite(&pr_id,1,16,fp_dst);
	write_dword(fp_dst,pr_version);
	write_dword(fp_dst,(expansion&0x10)? 33: 29);
	write_byte(fp_dst,expansion);
	write_dword(fp_dst,channels);
	write_dword(fp_dst,machine);
	write_dword(fp_dst,e_speed);
	write_dword(fp_dst,v_style);
	write_dword(fp_dst,highlight1);
	write_dword(fp_dst,highlight2);
	if(expansion&0x10) write_dword(fp_dst,N163_channels);
	write_dword(fp_dst,s_split);
	return true;
}

bool CFTMFile::WriteInfo()
{
	fwrite(&nf_id,1,16,fp_dst);
	write_dword(fp_dst,nf_version);
	write_dword(fp_dst,nf_size);
	fwrite(&author,1,32,fp_dst);
	fwrite(&title,1,32,fp_dst);
	fwrite(&ccccc,1,32,fp_dst);
	return true;
}

bool CFTMFile::WriteHeader()
{
	fwrite(&he_id,1,16,fp_dst);
	write_dword(fp_dst,he_version);
	write_dword(fp_dst,10+2*channels);
	write_byte(fp_dst,n_tracks);
	fwrite("New song",1,9,fp_dst);
	for (int i=0;i<channels;i++){
		//write_byte(fp_dst,ch_id[i]);
		write_byte(fp_dst,i);
		write_byte(fp_dst,ch_fx[i]);
	}
	return true;
}

bool CFTMFile::WriteInstruments()
{
	if(!(n_instruments + VRC6_instruments + VRC7_instruments + FDS_instruments + N163_instruments)) return true;
	fwrite(&i0_id,1,16,fp_dst);
	write_dword(fp_dst,i0_version);
	i0_size = (0x4 + 0x145*n_instruments + 0x25*VRC6_instruments + 0x23*VRC7_instruments + 0xAA*FDS_instruments + 0x31*N163_instruments);
	for(int i=0;i<FDS_instruments;i++){for(int j=0;j<3;j++){i0_size+=Inst_FDS[i].Seq[j].length;}}
	for(int i=0;i<N163_instruments;i++) i0_size+=Inst_N163[i].wave_size*Inst_N163[i].wave_count;
	write_dword(fp_dst,i0_size);
	write_dword(fp_dst,n_instruments + VRC6_instruments + VRC7_instruments + FDS_instruments + N163_instruments);

	for(int i=0;i<n_instruments;i++){
		write_dword(fp_dst,Inst_2A03[i].index);
		write_byte(fp_dst,Inst_2A03[i].type);
		write_dword(fp_dst,SEQ_COUNT);
		fwrite(Inst_2A03[i].seq_data,1,10,fp_dst);
		fwrite(Inst_2A03[i].DPCM_data,1,288,fp_dst);
		write_dword(fp_dst,0xE);
		fwrite("New instrument",1,14,fp_dst);
	}

	for(int i=0;i<VRC6_instruments;i++){
		write_dword(fp_dst,Inst_VRC6[i].index);
		write_byte(fp_dst,Inst_VRC6[i].type);
		write_dword(fp_dst,SEQ_COUNT);
		fwrite(Inst_VRC6[i].seq_data,1,10,fp_dst);
		write_dword(fp_dst,0xE);
		fwrite("New instrument",1,14,fp_dst);
	}

	for(int i=0;i<VRC7_instruments;i++){
		write_dword(fp_dst,Inst_VRC7[i].index);
		write_byte(fp_dst,Inst_VRC7[i].type);
		write_dword(fp_dst,Inst_VRC7[i].patch);
		for(int j=0;j<VRC7_REGS;j++) write_byte(fp_dst,Inst_VRC7[i].data[j]);
		write_dword(fp_dst,0xE);
		fwrite("New instrument",1,14,fp_dst);
	}

	for(int i=0;i<FDS_instruments;i++){
		write_dword(fp_dst,Inst_FDS[i].index);
		write_byte(fp_dst,Inst_FDS[i].type);
		for(int j=0;j<64;j++){write_byte(fp_dst,Inst_FDS[i].wave[j]);}
		for(int j=0;j<32;j++){write_byte(fp_dst,Inst_FDS[i].mtable[j]);}
		write_dword(fp_dst,Inst_FDS[i].mspeed);
		write_dword(fp_dst,Inst_FDS[i].mdepth);
		write_dword(fp_dst,Inst_FDS[i].mdelay);
		for(int j=0;j<3;j++){
			write_byte(fp_dst,Inst_FDS[i].Seq[j].length);
			write_dword(fp_dst,Inst_FDS[i].Seq[j].loop);
			write_dword(fp_dst,Inst_FDS[i].Seq[j].release);
			write_dword(fp_dst,Inst_FDS[i].Seq[j].setting);
			for(int k=0;k<Inst_FDS[i].Seq[j].length;k++){write_byte(fp_dst,Inst_FDS[i].Seq[j].data[k]);}
		}
		write_dword(fp_dst,0xE); // Inst. name length
		fwrite("New instrument",1,14,fp_dst);
	}

	for(int i=0;i<N163_instruments;i++){
		write_dword(fp_dst,Inst_N163[i].index);
		write_byte(fp_dst,Inst_N163[i].type);
		write_dword(fp_dst,SEQ_COUNT);
		fwrite(Inst_N163[i].seq_data,1,10,fp_dst);
		write_dword(fp_dst,Inst_N163[i].wave_size);
		write_dword(fp_dst,Inst_N163[i].wave_pos);
		write_dword(fp_dst,Inst_N163[i].wave_count);
		for(int j=0;j<Inst_N163[i].wave_count;j++) fwrite(Inst_N163[i].wave_data[j],1,Inst_N163[i].wave_size,fp_dst);
		write_dword(fp_dst,0xE);
		fwrite("New instrument",1,14,fp_dst);
	}

	return true;
}

bool CFTMFile::WriteSequences()
{
	if(!n_sequences) return true;
	fwrite(&s0_id,1,16,fp_dst);
	write_dword(fp_dst,s0_version);
	int pblocksize=4; for(int i=0;i<n_sequences;i++) pblocksize+=21+Sequence[i].length;
	write_dword(fp_dst,pblocksize);
	write_dword(fp_dst,n_sequences);
	for(int i=0;i<n_sequences;i++){
		write_dword(fp_dst,Sequence[i].index);
		write_dword(fp_dst,Sequence[i].type);
		write_byte(fp_dst,Sequence[i].length);
		write_dword(fp_dst,Sequence[i].loop);
		fwrite(Sequence[i].data,1,Sequence[i].length,fp_dst);		
	}
	for(int i=0;i<n_sequences;i++){
		write_dword(fp_dst,Sequence[i].release);
		write_dword(fp_dst,Sequence[i].setting);
	}
	return true;
}

bool CFTMFile::WriteFrames()
{
	fwrite(&fr_id,1,16,fp_dst);
	write_dword(fp_dst,fr_version);
	write_dword(fp_dst,0x10+channels*n_frames);
	write_dword(fp_dst,n_frames);
	write_dword(fp_dst,speed);
	write_dword(fp_dst,tempo);
	write_dword(fp_dst,p_length);
	for(int i=0;i<n_frames;i++)
		fwrite(fdata[i],1,channels,fp_dst);
	return true;
}

bool CFTMFile::WritePatterns()
{
	fwrite(&pt_id,1,16,fp_dst);
	write_dword(fp_dst,pt_version);
	int pblocksize=0;
	for (int i=0;i<n_patterns;i++){		
		if(Pattern[i].items!=0){
			pblocksize+=16+(10+2*ch_fx[Pattern[i].channel])*Pattern[i].items;
		}
	}
	write_dword(fp_dst,pblocksize);
	for(int i=0;i<n_patterns;i++){
		if(Pattern[i].items==0) continue;
		write_dword(fp_dst,Pattern[i].track);
		write_dword(fp_dst,Pattern[i].channel);
		write_dword(fp_dst,Pattern[i].index);
		write_dword(fp_dst,Pattern[i].items);
		int item_size=10+2*ch_fx[Pattern[i].channel];
		for(int j=0;j<Pattern[i].items;j++){
			fwrite(&Pattern[i].Item[j],1,item_size,fp_dst);
		}
	}
	return true;
}

bool CFTMFile::WriteSamples()
{
	fwrite(&ds_id,1,16,fp_dst);
	write_dword(fp_dst,ds_version);
	int pblocksize=1+9*n_samples;
	for (int i=0;i<n_samples;i++){		
		pblocksize+=0xE;
		pblocksize+=Sample[i].datalength;
	}
	
	write_dword(fp_dst,pblocksize);
	write_byte(fp_dst,n_samples);

	for (int i=0;i<n_samples;i++){
		write_byte(fp_dst,i);
		write_dword(fp_dst,0xE);
		fwrite("New instrument",1,14,fp_dst);
		write_dword(fp_dst,Sample[i].datalength);
		fwrite(Sample[i].data,Sample[i].datalength,1,fp_dst);
	}
	return true;
}

bool CFTMFile::WriteVRC6Sequences()
{
	if(!(expansion&0x1)) return true;
	fwrite(&vrc6s_id,1,16,fp_dst);
	write_dword(fp_dst,vrc6s_version);
	int pblocksize=4; for(int i=0;i<n_VRC6_sequences;i++){pblocksize+=21+VRC6_Sequence[i].length;}
	write_dword(fp_dst,pblocksize);
	write_dword(fp_dst,n_VRC6_sequences); // Sequence count
	for(int i=0;i<n_VRC6_sequences;i++){
		write_dword(fp_dst,VRC6_Sequence[i].index);
		write_dword(fp_dst,VRC6_Sequence[i].type);
		write_byte(fp_dst,VRC6_Sequence[i].length);
		write_dword(fp_dst,VRC6_Sequence[i].loop);
		fwrite(VRC6_Sequence[i].data,1,VRC6_Sequence[i].length,fp_dst);		
	}

	for(int i=0;i<n_VRC6_sequences;i++){
		write_dword(fp_dst,VRC6_Sequence[i].release);
		write_dword(fp_dst,VRC6_Sequence[i].setting);
	}

	return true;
}

bool CFTMFile::WriteN163Sequences()
{
	if(!(expansion&0x10)) return true;
	if(!n_N163_sequences) return true;
	fwrite(&n163s_id,1,16,fp_dst);
	write_dword(fp_dst,n163s_version);
	int pblocksize=4; for(int i=0;i<n_N163_sequences;i++){pblocksize+=21+N163_Sequence[i].length;}
	write_dword(fp_dst,pblocksize);
	write_dword(fp_dst,n_N163_sequences); // Sequence count
	for(int i=0;i<n_N163_sequences;i++){
		write_dword(fp_dst,N163_Sequence[i].index);
		write_dword(fp_dst,N163_Sequence[i].type);
		write_byte(fp_dst,N163_Sequence[i].length);
		write_dword(fp_dst,N163_Sequence[i].loop);
		write_dword(fp_dst,N163_Sequence[i].release);
		write_dword(fp_dst,N163_Sequence[i].setting);
		fwrite(N163_Sequence[i].data,1,N163_Sequence[i].length,fp_dst);		
	}
	return true;
}

bool CFTMFile::LogStuff()
{
	fprintf(fp_log,"\nFRAMES\n\n");

	for(int i=0;i<n_frames;i++){
		fprintf(fp_log,"%02X:",i);
		for(int j=0;j<channels;j++) fprintf(fp_log," %02X",fdata[i][j]);
		fprintf(fp_log,"\n");
	}

	fprintf(fp_log,"\nPATTERNS\n\n");

	for(int i=0;i<n_patterns;i++){
		fprintf(fp_log,"Pattern %i index: %i channel %i: items: %i\n",i,Pattern[i].index,Pattern[i].channel,Pattern[i].items);
	}

	return true;
}