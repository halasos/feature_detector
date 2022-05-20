#include "nms_dual.h"

static
void compute_score_coords_double(uint16_t score_x0, uint16_t *x_coord0, uint16_t *x_coord1, uint16_t *y_coord){
#pragma HLS INLINE

	static int16_t x_coord_d0,x_coord_d1,x_coord_d2,x_coord_d2_p;
	static int16_t y_coord_d0=-1,y_coord_d1,y_coord_d2,y_coord_d3;
	x_coord_d2_p =x_coord_d1+1;
	x_coord_d2 = x_coord_d1;
	x_coord_d1 = x_coord_d0;
	x_coord_d0 = score_x0;


	y_coord_d3 = y_coord_d2;
	y_coord_d2 = y_coord_d1;
	y_coord_d1 = y_coord_d0;

	if(score_x0==IMG_WIDTH-2 ){
		if(y_coord_d0 < IMG_HEIGH-1){
			y_coord_d0++;
		}else
			y_coord_d0=0;
	}


	*x_coord0 = x_coord_d2;
	*y_coord = y_coord_d3;
	*x_coord1 = x_coord_d2_p;


}

static
void cache_scores_double(uint16_t score1,uint16_t score0, uint16_t col_index,score_lines_t * col0,score_lines_t * col1){
#pragma HLS INLINE


	static uint32_t internal_line1[RAM_LINE];
	#pragma HLS RESOURCE variable=internal_line1 core=RAM_T2P_BRAM
	static uint32_t internal_line2[RAM_LINE];
	#pragma HLS RESOURCE variable=internal_line2 core=RAM_T2P_BRAM

	col0->scline2 = (uint16_t)(internal_line2[col_index]>>16);
	col1->scline2 = (uint16_t)(internal_line2[col_index]);

	col0->scline1 = (uint16_t)(internal_line1[col_index]>>16);
	col1->scline1 = (uint16_t)(internal_line1[col_index]);


	col0->scline0 = score1;
	col1->scline0 = score0;


	internal_line2[col_index] = internal_line1[col_index];
	internal_line1[col_index] = (score1<<16)|score0;
}

static
void nms_matrix_double(score_lines_t *col0, score_lines_t *col1,score_matrix_t *mtx0,score_matrix_t *mtx1){
#pragma HLS INLINE
	static uint16_t c0_0,c0_1,c0_2;
	static uint16_t c1_0,c1_1,c1_2;
	static uint16_t c2_0,c2_1,c2_2;
	static uint16_t c3_0,c3_1,c3_2;

	mtx0->score_center = c1_1;
	mtx0->score0 = c0_0;
	mtx0->score1 = c1_0;
	mtx0->score2 = c2_0;
	mtx0->score3 = c0_1;
	mtx0->score4 = c2_1;
	mtx0->score5 = c0_2;
	mtx0->score6 = c1_2;
	mtx0->score7 = c2_2;

	mtx1->score_center = c2_1;
	mtx1->score0 = c1_0;
	mtx1->score1 = c2_0;
	mtx1->score2 = c3_0;
	mtx1->score3 = c1_1;
	mtx1->score4 = c3_1;
	mtx1->score5 = c1_2;
	mtx1->score6 = c2_2;
	mtx1->score7 = c3_2;

	//line0
	c3_0 = c1_0;
	c2_0 = c0_0;
	c1_0 = col1->scline0;
	c0_0 = col0->scline0;

	//line1
	c3_1 = c1_1;
	c2_1 = c0_1;
	c1_1 = col1->scline1;
	c0_1 = col0->scline1;

	//line2
	c3_2 = c1_2;
	c2_2 = c0_2;
	c1_2 = col1->scline2;
	c0_2 = col0->scline2;


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

void mux_coord_double(uint1 is_corner1,uint1 is_corner0,
					uint16_t x_coord0, uint16_t x_coord1,
					uint16_t y_coord,
					uint1 * p_is_corner,
					uint16_t *p_x_coord, uint16_t*p_y_coord){
#pragma HLS INLINE
	static uint16_t x_coord_i = 0;
	static uint16_t y_coord_i = 0;
	static uint1 is_corner_i = 0;

	*p_x_coord = x_coord_i;
	*p_y_coord = y_coord_i;
	*p_is_corner = is_corner_i;

	y_coord_i = y_coord;

	if(is_corner0==1){
		x_coord_i = x_coord0;
		is_corner_i = 1;
	}
	else {
		if(is_corner1==1){
			x_coord_i = x_coord1;
			is_corner_i = 1;
		}else{

		x_coord_i = x_coord0;
		is_corner_i=0;
		}
	}

}
void delay_score(uint1 is_corner,  uint1 *p_is_corner){


	static uint1 is_corner_i=0;

	*p_is_corner=is_corner_i;


	is_corner_i = is_corner;


}

void nms_double(uint16_t score1[IMG_HEIGH*IMG_WIDTH],uint16_t score0[IMG_HEIGH*IMG_WIDTH],
		uint1 *is_corner,
		uint16_t *x_coord, uint16_t*y_coord){
#pragma HLS INTERFACE ap_fifo port=score0
#pragma HLS INTERFACE ap_fifo port=score1
#pragma HLS INTERFACE ap_fifo port=is_corner


 score_lines_t col0,col1;
 score_matrix_t mtx0,mtx1;
  uint1 is_corner0_i,is_corner0_d1_i;
 uint1 is_corner1_i;
 uint16_t x_coord0_i, x_coord1_i, y_coord_i;


 uint16_t p,k_y,k_x, coord_x0;
 for( p=0,k_y=0; k_y < IMG_HEIGH; k_y++){
  for(k_x=0,coord_x0=0; k_x < IMG_WIDTH/2; k_x++, p++ ,coord_x0+=2){
	#pragma HLS PIPELINE rewind
	 compute_score_coords_double(coord_x0, &x_coord0_i,&x_coord1_i,&y_coord_i);
	 cache_scores_double(score1[p],score0[p], k_x,&col0,&col1);
	   nms_matrix_double(&col0,&col1,&mtx0,&mtx1);
		 suppress_nonmax(&mtx0, &is_corner0_i);
		 delay_score(is_corner0_i, &is_corner0_d1_i);
		 suppress_nonmax(&mtx1, &is_corner1_i);
		 	 mux_coord_double(is_corner1_i, is_corner0_d1_i, x_coord0_i, x_coord1_i, y_coord_i, is_corner, x_coord, y_coord);


}}}
