#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <exception>
#include <sstream>

using namespace std;

#define BYTE unsigned char

#define uint32_t unsigned int
#define int32_t int
#define uint8_t unsigned char
#define int8_t char
#define uint64_t unsigned long
#define int64_t long
#define int16_t short
#define uint16_t unsigned short

#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#define MAX(X, Y) ((X)>(Y)?(X):(Y))

struct VECTOR
{
	int x;
	int y;
};

struct WARPPOINTS
{
	VECTOR duv[3];
};


#define MBPRED_SIZE  15

struct Macroblock
{
	/* decoder/encoder */
	VECTOR mvs[4];
	short int pred_values[6][MBPRED_SIZE];
	int acpred_directions[6];
	int mode;
	int quant;					/* absolute quant */

	int field_dct;
	int field_pred;
	int field_for_top;
	int field_for_bot;

	/* encoder specific */

	VECTOR pmvs[4];
	VECTOR qmvs[4];				/* mvs in quarter pixel resolution */

	int32_t sad8[4];			/* SAD values for inter4v-VECTORs */
	int32_t sad16;				/* SAD value for inter-VECTOR */

	int32_t var16;				/* Variance of the 16x16 luma block */
	int32_t rel_var8[6];		/* Relative variances of the 8x8 sub-blocks */

	int dquant;
	int cbp;

	/* lambda for these blocks */
	int lambda[6];

	/* bframe stuff */

	VECTOR b_mvs[4];
	VECTOR b_qmvs[4];

	VECTOR amv; /* average motion vectors from GMC  */
	int32_t mcsel;
	
	VECTOR  mvs_avg;      //CK average of field motion vectors

/* This structure has become way to big! What to do? Split it up?   */
	Macroblock()
	{

		acpred_directions[0] = acpred_directions[1] = acpred_directions[2]
		=acpred_directions[3] = acpred_directions[4] = acpred_directions[5] = 0;

		mode = 0;
		quant = 0;
		field_dct = field_pred = field_for_top = field_for_bot = 0;
		dquant = cbp = 0;
		mcsel = 0;
	}
};


/* zigzag */

static const uint16_t scan_tables[3][64] = {
	/* zig_zag_scan */
	{ 0,  1,  8, 16,  9,  2,  3, 10,
	 17, 24, 32, 25, 18, 11,  4,  5,
	 12, 19, 26, 33, 40, 48, 41, 34,
	 27, 20, 13,  6,  7, 14, 21, 28,
	 35, 42, 49, 56, 57, 50, 43, 36,
	 29, 22, 15, 23, 30, 37, 44, 51,
	 58, 59, 52, 45, 38, 31, 39, 46,
	 53, 60, 61, 54, 47, 55, 62, 63},

	/* horizontal_scan */
	{ 0,  1,  2,  3,  8,  9, 16, 17,
	 10, 11,  4,  5,  6,  7, 15, 14,
	 13, 12, 19, 18, 24, 25, 32, 33,
	 26, 27, 20, 21, 22, 23, 28, 29,
	 30, 31, 34, 35, 40, 41, 48, 49,
	 42, 43, 36, 37, 38, 39, 44, 45,
	 46, 47, 50, 51, 56, 57, 58, 59,
	 52, 53, 54, 55, 60, 61, 62, 63},

	/* vertical_scan */
	{ 0,  8, 16, 24,  1,  9,  2, 10,
	 17, 25, 32, 40, 48, 56, 57, 49,
	 41, 33, 26, 18,  3, 11,  4, 12,
	 19, 27, 34, 42, 50, 58, 35, 43,
	 51, 59, 20, 28,  5, 13,  6, 14,
	 21, 29, 36, 44, 52, 60, 37, 45,
	 53, 61, 22, 30,  7, 15, 23, 31,
	 38, 46, 54, 62, 39, 47, 55, 63}
};

#endif
