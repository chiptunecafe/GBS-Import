#include <cstdio>
#include <string.h>
#include "FTMFile.h"

// Z80_regpair from FUSE

typedef union {
	struct { unsigned char l,h; } b;
	unsigned short w;
} Z80_regpair;

// Gb_Env class from Game Music Emu

class Gb_Env {
public:
	int  env_delay;
	int  volume;
	bool env_enabled;

	int vol,va,vp,vc;

	unsigned char regs[5];
	
	void clock_envelope();
	bool write_register( int frame_phase, int reg, int old_data, int data );
	
	void reset()
	{
		memset(this,0,sizeof(Gb_Env));
	}

	// Non-zero if DAC is enabled
	int dac_enabled() const { return regs [2] & 0xF8; }

	void zombie_volume( int old, int data );
	int reload_env_timer();
};

class CGBS_Emu {
public: 
	FILE* fp_src;
	FILE* fp_dst;
	FILE* fp_log;

	int src_size;

	unsigned char* data;
	unsigned char* bdata;
	int* op_count;

	struct stInfo{
		char file_id[3];
		unsigned char version;
		unsigned char n_songs;
		unsigned char first;
		unsigned short load_addr;
		unsigned short init_addr;
		unsigned short play_addr;
		unsigned short sp_init;
		unsigned char tmod;
		unsigned char tctl;
		char title[32];
		char author[32];
		char ccccc[32];
	} Info;

	Z80_regpair af;
	Z80_regpair bc;
	Z80_regpair de;
	Z80_regpair hl;
	Z80_regpair ix;
	Z80_regpair iy;
	Z80_regpair sp;
	Z80_regpair pc;

	unsigned char wave[16];

	Gb_Env Env[3];

	char sweeped;
	char sq1_vol;	
	char sq1_vp;
	char sq1_va;
	char sq1_vc;
	char sq1_ll;
	char sq1_lc;
	char sq2_vol;
	char sq2_vp;
	char sq2_va;
	char sq2_vc;
	char sq2_ll;
	char sq2_lc;
	char noise_vol;
	char noise_vp;	
	char noise_va;	
	char noise_vc;
	char noise_ll;
	char noise_lc;

	int wave_ll;
	int wave_lc;
	int wave_vol;

	bool is_switched;
	int n_banks;
	int c_bank;

	char errorcode[256];

	CGBS_Emu();
	~CGBS_Emu();
	void Cleanup();

	bool LoadFile(char* filename);
	bool ReadHeader();
	bool ReadData();

	void LogHeader();
	void LogData();
	void LogDisasm();

	void Clock64();

	bool Init(int track);
	bool PlayOnce();
	int RunCpu();
	int RunExtCB();
};

