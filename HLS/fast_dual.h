#ifndef __FAST_DUAL_MOD__
#define __FAST_DUAL_MOD__

#define IMG_WIDTH 1920
#define IMG_HEIGH 1080
#define RAM_LINE 2048/2

#include <stdint.h>
#include "ap_cint.h"


typedef struct {
	uint8_t line0_pixel;
	uint8_t line1_pixel;
	uint8_t line2_pixel;
	uint8_t line3_pixel;
	uint8_t line4_pixel;
	uint8_t line5_pixel;
	uint8_t line6_pixel;

}cache_colmn_t;

typedef struct {
	uint8_t pixel_0;
	uint8_t pixel_1;
	uint8_t pixel_2;
	uint8_t pixel_3;
	uint8_t pixel_4;
	uint8_t pixel_5;
	uint8_t pixel_6;
	uint8_t pixel_7;
	uint8_t pixel_8;
	uint8_t pixel_9;
	uint8_t pixel_10;
	uint8_t pixel_11;
	uint8_t pixel_12;
	uint8_t pixel_13;
	uint8_t pixel_14;
	uint8_t pixel_15;
	uint8_t center;
}bhrm_circle_t;


typedef struct {
	int16_t pre_score_0;
	int16_t pre_score_1;
	int16_t pre_score_2;
	int16_t pre_score_3;
	int16_t pre_score_4;
	int16_t pre_score_5;
	int16_t pre_score_6;
	int16_t pre_score_7;
	int16_t pre_score_8;
	int16_t pre_score_9;
	int16_t pre_score_10;
	int16_t pre_score_11;
	int16_t pre_score_12;
	int16_t pre_score_13;
	int16_t pre_score_14;
	int16_t pre_score_15;

} pre_scores16_t;

typedef struct {
	uint1 f0;
	uint1 f1;
	uint1 f2;
	uint1 f3;
	uint1 f4;
	uint1 f5;
	uint1 f6;
	uint1 f7;
	uint1 f8;
	uint1 f9;
	uint1 f10;
	uint1 f11;
	uint1 f12;
	uint1 f13;
	uint1 f14;
	uint1 f15;

}flags_t;

typedef struct{
	pre_scores16_t dark_prscrs;
	pre_scores16_t bright_prscrs;
	flags_t dark_flags;
	flags_t bright_flags;
	uint8_t center;
}classifilter_data_t;


#endif //__FAST_DUAL_MOD__
