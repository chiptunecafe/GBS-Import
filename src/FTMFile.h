#pragma once

#include <stdio.h>
#include "global_stuff.h"

#define SEQ_COUNT 5
#define MAX_FRAMES 128
#define VRC7_REGS 8

struct stSequence{
	stSequence::stSequence();
	dword index;
	dword type;
	byte  length;
	dword loop;
	dword release;
	dword setting;
	byte data[256];
};

struct Instrument_2A03{
	Instrument_2A03::Instrument_2A03();
	dword index;
	byte  type;
	dword seq_count;
	byte  seq_data[2*SEQ_COUNT];
	byte  DPCM_data[288];
	dword name_length; // Inst. name length
	char name[64];
};

struct Instrument_FDS{
	Instrument_FDS::Instrument_FDS();
	dword index;
	byte  type;
	byte  wave[64];
	byte  mtable[32];
	int  mspeed;
	int  mdepth;
	int  mdelay;
	stSequence Seq[3];
	dword name_length; // Inst. name length
	char name[64];
};

struct Instrument_VRC6{
	Instrument_VRC6::Instrument_VRC6();
	dword index;
	byte  type;
	dword seq_count;
	byte  seq_data[2*SEQ_COUNT];
	dword name_length; // Inst. name length
	char name[64];
};


struct Instrument_VRC7{
	Instrument_VRC7::Instrument_VRC7();
	dword index;
	byte  type;
	int patch;
	byte data[8];
	dword name_length; // Inst. name length
	char name[64];
};

struct Instrument_N163{
	Instrument_N163::Instrument_N163();
	dword index;
	int type;
	dword seq_count;
	byte  seq_data[2*SEQ_COUNT];
	int wave_size;
	int wave_pos;
	int wave_count;
	byte wave_data[16][32];
	dword name_length; // Inst. name length
	char name[64];
};

struct DPCM_Sample{
	int index;
	int namelength;
	int datalength;
	char name[64];
	byte data[4096];
};

struct FTItem{
	FTItem();
	dword row;
	byte note;
	byte octave;
	byte instrument;
	byte volume;
	byte fxdata[8];
};

struct FTPattern{
	FTPattern();
	dword track;
	dword channel;
	dword index;
	int   items;
	FTItem Item[256];
};

class CFTMFile{
public:	CFTMFile();
	FILE* fp_src;
	FILE* fp_log;
	FILE* fp_dst;
	FILE* fp_asm;

	int src_size;

	char  file_id[24];
	dword file_version;

	char  pr_id[24]; //PARAMS
	dword pr_version;
	dword pr_size;
	byte  expansion;
	int channels;
	dword machine;
	dword e_speed;
	dword v_style;
	dword highlight1;
	dword highlight2;
	dword N163_channels;
	dword s_split;

	char  nf_id[24]; //INFO
	dword nf_version;
	dword nf_size;
	char  author[32];
	char  title[32];
	char  ccccc[32];

	char  he_id[24]; //HEADER
	dword he_version;
	dword he_size;
	byte  n_tracks;
	char  track_name[64];
	char  ch_id[32];
	char  ch_fx[32];

	char  i0_id[24]; //INSTRUMENTS
	dword i0_version;
	dword i0_size;
	int n_instruments;
	int FDS_instruments;
	int VRC6_instruments;
	int VRC7_instruments;
	int N163_instruments;
	Instrument_2A03 Inst_2A03[64];
	Instrument_FDS  Inst_FDS[64];
	Instrument_VRC6 Inst_VRC6[64];
	Instrument_VRC7 Inst_VRC7[64];
	Instrument_N163 Inst_N163[64];

	char  s0_id[24]; //SEQUENCES
	dword s0_version;
	dword s0_size;
	int n_sequences;
	stSequence Sequence[320];

	char  fr_id[24]; //FRAMES
	dword fr_version;
	dword fr_size;
	int   n_frames;
	dword speed;
	dword tempo;
	int p_length;
	byte  fdata[MAX_FRAMES][16];

	char  pt_id[24]; //PATTERNS
	dword pt_version;
	dword pt_size;
	FTPattern Pattern[640];

	char  ds_id[24]; //DPCM SAMPLES
	dword ds_version;
	dword ds_size;
	byte  n_samples;

	char  vrc6s_id[24]; //VRC6 SEQUENCES
	char  vrc6s_version;
	char  vrc6s_size;
	int	  n_VRC6_sequences;
	stSequence VRC6_Sequence[128];

	char  n163s_id[24]; //N163 SEQUENCES
	char  n163s_version;
	char  n163s_size;
	int	  n_N163_sequences;
	stSequence N163_Sequence[128];

	DPCM_Sample Sample[64];

	int n_patterns;

	bool LoadFile(char* filename);
	bool SaveFile(char* filename);
	bool Optimize();
	bool ExportTXT(char* filename);

	bool ReadID();
	bool ReadParams();
	bool ReadInfo();
	bool ReadHeader();
	bool ReadInstruments();
	bool ReadSequences();
	bool ReadFrames();
	bool ReadPatterns();
	bool ReadSamples();

	bool WriteID();
	bool WriteParams();
	bool WriteInfo();
	bool WriteHeader();
	bool WriteInstruments();
	bool WriteSequences();
	bool WriteFrames();
	bool WritePatterns();
	bool WriteSamples();
	bool WriteVRC6Sequences();
	bool WriteN163Sequences();

	bool LogStuff();
};