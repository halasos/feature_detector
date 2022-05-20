#include "nms.h"

static
void cache_scores(uint16_t score, uint16_t col_index,score_lines_t * curr_colmn){
#pragma HLS INLINE


	static uint16_t internal_line1[RAM_LINE];
	#pragma HLS RESOURCE variable=internal_line1 core=RAM_T2P_BRAM
	static uint16_t internal_line2[RAM_LINE];
	#pragma HLS RESOURCE variable=internal_line2 core=RAM_T2P_BRAM

	curr_colmn->scline2 = internal_line2[col_index];
	curr_colmn->scline1 = internal_line1[col_index];
	curr_colmn->scline0 = score;


	internal_line2[col_index] = internal_line1[col_index];
	internal_line1[col_index] = score;
}

static
void nms_matrix(score_lines_t *curr_colmn, score_matrix_t *mtx){
#pragma HLS INLINE
	static uint16_t c0_0,c0_1,c0_2;
	static uint16_t c1_0,c1_1,c1_2;
	static uint16_t c2_0,c2_1,c2_2;

	//line0
	c2_0 = c1_0;
	c1_0 = c0_0;
	c0_0 = curr_colmn->scline0;

	//line1
	c2_1 = c1_1;
	c1_1 = c0_1;
	c0_1 = curr_colmn->scline1;

	//line2
	c2_2 = c1_2;
	c1_2 = c0_2;
	c0_2 = curr_colmn->scline2;

	mtx->score_center = c1_1;
	mtx->score0 = c0_0;
	mtx->score1 = c1_0;
	mtx->score2 = c2_0;
	mtx->score3 = c0_1;
	mtx->score4 = c2_1;
	mtx->score5 = c0_2;
	mtx->score6 = c1_2;
	mtx->score7 = c2_2;

}

static
void suppress_nonmax(score_matrix_t *mtx, uint1 *is_corner){
#pragma HLS INLINE

   *is_corner = (mtx->score0 < mtx->score_center) &
				(mtx->score1 < mtx->score_center) &
				(mtx->score2 < mtx->score_center) &
				(mtx->score3 < mtx->score_center) &
				(mtx->score4 < mtx->score_center) &
				(mtx->score5 < mtx->score_center) &
				(mtx->score6 < mtx->score_center) &
				(mtx->score7 < mtx->score_center);
}

static
void compute_score_coords(uint16_t score_x,uint16_t score_y, coords_t *sc_coord){
#pragma HLS INLINE

	static int16_t x_coord_d0 = 0,x_coord_d1,x_coord_d2;
#pragma HLS RESET variable=x_coord_d0
	static int16_t y_coord_d0=0,y_coord_d1,y_coord_d2,y_coord_d3;
#pragma HLS RESET variable=y_coord_d0


	x_coord_d2 = x_coord_d1;
	x_coord_d1 = x_coord_d0;

	y_coord_d3 = y_coord_d2;
	y_coord_d2 = y_coord_d1;

	if(score_x==IMG_WIDTH-1 ){
		x_coord_d0 = 0;
		y_coord_d1 = y_coord_d0;
		if(score_y == IMG_HEIGH-1)
			y_coord_d0 = 0;
		else
			y_coord_d0++;


	}else{
		x_coord_d0 ++;
	}

	sc_coord->x_coord = x_coord_d2;
	sc_coord->y_coord = y_coord_d3;
}


void nms9(uint16_t i_score[IMG_HEIGH*IMG_WIDTH],
  uint1 *o_is_corner,coords_t o_coords[IMG_HEIGH*IMG_WIDTH]){
#pragma HLS INTERFACE ap_fifo port=i_score
#pragma HLS INTERFACE ap_fifo port=o_coords
 score_lines_t curr_colmn;
 score_matrix_t scores;
 int i,k_y,k_x;
 for( i=0,k_y=0; k_y < IMG_HEIGH; k_y++){
  for(k_x=0; k_x < IMG_WIDTH; k_x++, i++ ){
	#pragma HLS PIPELINE rewind
	  cache_scores(i_score[i], k_x, &curr_colmn);
	   nms_matrix(&curr_colmn,&scores);
		 suppress_nonmax(&scores, o_is_corner);
	  compute_score_coords(k_x,k_y, &(o_coords[i]));
}}}
