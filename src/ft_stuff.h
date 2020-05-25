/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2012  Jonathan Liss
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
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

// Part of this is directly taken from FamiTracker's source, hence the copyright notice. ;)
// -Slimeball

typedef unsigned char byte;

#define EX_NONE		0x00
#define EX_VRC6		0x01
#define EX_VRC7		0x02
#define EX_FDS		0x04
#define EX_MMC5		0x08
#define EX_N163		0x10
#define EX_S5B		0x20

#define FT_NOTE_OFF	0x0D
#define FT_NOTE_CUT	0x0E

enum{
	TYPE_2A03=1,
	TYPE_VRC6,
	TYPE_FDS,
	TYPE_VRC7,
	TYPE_N163,
	TYPE_S5B
};

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

// Channel effects
#define DEF_CMD(x) ((x << 1) | 0x80)
enum {
	EF_NONE = 0,
	EF_SPEED,
	EF_JUMP,
	EF_SKIP,
	EF_HALT,
	EF_VOLUME,
	EF_PORTAMENTO,
	EF_PORTAOFF,				// unused!!
	EF_SWEEPUP,
	EF_SWEEPDOWN,
	EF_ARPEGGIO,
	EF_VIBRATO,
	EF_TREMOLO,
	EF_PITCH,
	EF_DELAY,
	EF_EARRAPE,
	EF_PORTA_UP,
	EF_PORTA_DOWN,
	EF_DUTY_CYCLE,
	EF_SAMPLE_OFFSET,
	EF_SLIDE_UP,
	EF_SLIDE_DOWN,
	EF_VOLUME_SLIDE,
	EF_NOTE_CUT,
	EF_RETRIGGER,
	EF_DELAYED_VOLUME,			// Unimplemented
	EF_FDS_MOD_DEPTH,
	EF_FDS_MOD_SPEED_HI,
	EF_FDS_MOD_SPEED_LO,
	EF_DPCM_PITCH,
	EF_SUNSOFT_ENV_LO,
	EF_SUNSOFT_ENV_HI,
	EF_SUNSOFT_ENV_TYPE,
//	EF_TARGET_VOLUME_SLIDE, 
/*
	EF_VRC7_MODULATOR,
	EF_VRC7_CARRIER,
	EF_VRC7_LEVELS,
*/
	EF_COUNT
};

inline char ftmn2not(byte ftmn)
{
	if(ftmn==0x00) return '-';
	if(ftmn==0x13) return '=';
	if(ftmn==0x14) return '^';
	switch((ftmn+1)%12){
		case  0: case  1: return 'C';
		case  2: case  3: return 'D';
		case  4: case  5: return 'E';
		case  6:		  return 'F';
		case  7: case  8: return 'G';
		case  9: case 10: return 'A';
		case 11:		  return 'B';
		default:		  return 'X';
	}
}

inline char ftmn2acc(byte ftmn)
{
	if(ftmn==0x00) return '-';
	if(ftmn==0x13) return '=';
	if(ftmn==0x14) return '^';
	switch((ftmn+1)%12){
		case  1: return '#';
		case  3: return '#';
		case  6: return '#';
		case  8: return '#';
		case 10: return '#';
		default: return '-';
	}	
}

inline char ftmno2oct(byte ftmn, byte ftmo)
{
	if(ftmn==0x00) return '-';
	if(ftmn==0x13) return '=';
	if(ftmn==0x14) return '^';;
	return 48+ftmo;
}

inline char ftmfx(byte rawfx)
{
	switch(rawfx){
		case 0:					return '-';
		case EF_SPEED:			return 'F';
		case EF_JUMP:			return 'B';
		case EF_SKIP:			return 'D';
		case EF_HALT:			return 'C';
		case EF_VOLUME:			return 'E';
		case EF_PORTAMENTO:		return '3';
		case EF_PORTA_UP:		return '1';
		case EF_PORTA_DOWN:		return '2';
		case EF_SWEEPUP:		return 'H';
		case EF_SWEEPDOWN:		return 'I';
		case EF_ARPEGGIO:		return '0';
		case EF_VIBRATO:		return '4';
		case EF_TREMOLO:		return '7';
		case EF_PITCH:			return 'P';
		case EF_DELAY:			return 'G';
		case EF_EARRAPE:		return 'Z';
		case EF_DUTY_CYCLE:		return 'V';
		case EF_SAMPLE_OFFSET:	return 'Y';
		case EF_SLIDE_UP:		return 'Q';
		case EF_SLIDE_DOWN:		return 'R';
		case EF_VOLUME_SLIDE:	return 'A';
		case EF_NOTE_CUT:		return 'S';
		case EF_RETRIGGER:		return 'X';
		case EF_DPCM_PITCH:		return 'W';
		default: return '?';
	}	
}