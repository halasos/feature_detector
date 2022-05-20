#ifndef __NMS_MOD__
#define __NMS_MOD__
#include <stdint.h>
#include "ap_cint.h"
#include "fast.h"

typedef struct {
	uint16_t scline0;
	uint16_t scline1;
	uint16_t scline2;
} score_lines_t;

typedef struct {
	uint16_t score0;
	uint16_t score1;
	uint16_t score2;
	uint16_t score3;
	uint16_t score4;
	uint16_t score5;
	uint16_t score6;
	uint16_t score7;
	uint16_t score_center;
}score_matrix_t;


typedef struct{
	uint16_t x_coord;
	uint16_t y_coord;
}coords_t;

extern void nms9(uint16_t score[IMG_HEIGH * IMG_WIDTH], uint1* is_corner, coords_t sc_coord[IMG_HEIGH * IMG_WIDTH]);

#endif //__NMS_MOD__
