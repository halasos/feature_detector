#include "fast.h"



static
void compute_coordinates(uint16_t k_x, uint16_t k_y,uint16_t* x_coord, uint16_t *y_coord,uint1* coord_valid){
#pragma HLS INLINE

static int16_t x_coord_d0,x_coord_d1,x_coord_d2,x_coord_d3,x_coord_d4;
#pragma HLS RESET variable=x_coord_d0
static int16_t y_coord_d0,y_coord_d1,y_coord_d2,y_coord_d3,y_coord_d4,y_coord_d5,y_coord_d6,y_coord_d7;
#pragma HLS RESET variable=y_coord_d0
static uint1 l1,l2,l3,p0,p1,p2,p3;
#pragma HLS RESET variable=l1
#pragma HLS RESET variable=l2
#pragma HLS RESET variable=l3
#pragma HLS RESET variable=p3
#pragma HLS RESET variable=p2
#pragma HLS RESET variable=p1
#pragma HLS RESET variable=p0

	p3=p2;
	p2=p1;
	p1=p0;
	p0=l3;
	x_coord_d4 = x_coord_d3;
	x_coord_d3 = x_coord_d2;
	x_coord_d2 = x_coord_d1;
	x_coord_d1 = x_coord_d0;

	y_coord_d7 = y_coord_d6;
	y_coord_d6 = y_coord_d5;
	y_coord_d5 = y_coord_d4;
	y_coord_d4 = y_coord_d3;


	if(k_x==IMG_WIDTH-1 ){

		x_coord_d0 = 0;

		y_coord_d3 = y_coord_d2;
		y_coord_d2 = y_coord_d1;
		y_coord_d1 = y_coord_d0;
		if(k_y == IMG_HEIGH-1){
			y_coord_d0=0;
		}else{
			y_coord_d0++;
		}
		l3=l2;
		l2=l1;
		l1=1;

	}else{
		x_coord_d0++;
	}
	*x_coord = x_coord_d4;
	*y_coord = y_coord_d7;
	*coord_valid = p3;
}

static
void cache_img_lines(uint8_t pixel,uint16_t col_index,	cache_colmn_t *curr_colmn){
#pragma HLS INLINE

	static uint8_t line1_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line1_i core=RAM_T2P_BRAM
	static uint8_t line2_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line2_i core=RAM_T2P_BRAM
	static uint8_t line3_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line3_i core=RAM_T2P_BRAM
	static uint8_t line4_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line4_i core=RAM_T2P_BRAM
	static uint8_t line5_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line5_i core=RAM_T2P_BRAM
	static uint8_t line6_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line6_i core=RAM_T2P_BRAM

		//output column
		curr_colmn->line6_pixel = line6_i[col_index];
		curr_colmn->line5_pixel = line5_i[col_index];
		curr_colmn->line4_pixel = line4_i[col_index];
		curr_colmn->line3_pixel = line3_i[col_index];
		curr_colmn->line2_pixel = line2_i[col_index];
		curr_colmn->line1_pixel = line1_i[col_index];
		curr_colmn->line0_pixel = pixel;
		//Forward pixels between lines
		line6_i[col_index] = line5_i[col_index];
		line5_i[col_index] = line4_i[col_index];
		line4_i[col_index] = line3_i[col_index];
		line3_i[col_index] = line2_i[col_index];
		line2_i[col_index] = line1_i[col_index];
		line1_i[col_index] = pixel;
}

static
void pixel_matrix(cache_colmn_t *p_current_col,
		bhrm_circle_t *circles){
#pragma HLS INLINE


	static uint8_t c0_0,c0_1,c0_2,c0_3,c0_4,c0_5,c0_6;
	static uint8_t c1_0,c1_1,c1_2,c1_3,c1_4,c1_5,c1_6;
	static uint8_t c2_0,c2_1,c2_2,c2_3,c2_4,c2_5,c2_6;
	static uint8_t c3_0,c3_1,c3_2,c3_3,c3_4,c3_5,c3_6;
	static uint8_t c4_0,c4_1,c4_2,c4_3,c4_4,c4_5,c4_6;
	static uint8_t c5_1,c5_2,c5_3,c5_4,c5_5;//,c5_6;c5_0,
	static uint8_t c6_2,c6_3,c6_4;//,c6_5;//,c6_6;c6_0,c6_1


			//line0
			//c6_0 = c5_0;
			//c5_0 = c4_0;
			c4_0 = c3_0;
			c3_0 = c2_0;
			c2_0 = c1_0;
			c1_0 = c0_0;
			c0_0 = p_current_col->line0_pixel;



			//line1

			//c6_1 = c5_1;
			c5_1 = c4_1;
			c4_1 = c3_1;
			c3_1 = c2_1;
			c2_1 = c1_1;
			c1_1 = c0_1;
			c0_1 = p_current_col->line1_pixel;


			//line2

			c6_2 = c5_2;
			c5_2 = c4_2;
			c4_2 = c3_2;
			c3_2 = c2_2;
			c2_2 = c1_2;
			c1_2 = c0_2;
			c0_2 = p_current_col->line2_pixel;

			//line3

			c6_3 = c5_3;
			c5_3 = c4_3;
			c4_3 = c3_3;
			c3_3 = c2_3;
			c2_3 = c1_3;
			c1_3 = c0_3;
			c0_3 = p_current_col->line3_pixel;

			//line4

			c6_4 = c5_4;
			c5_4 = c4_4;
			c4_4 = c3_4;
			c3_4 = c2_4;
			c2_4 = c1_4;
			c1_4 = c0_4;
			c0_4 = p_current_col->line4_pixel;

			//line5

			//c6_5 = c5_5;
			c5_5 = c4_5;
			c4_5 = c3_5;
			c3_5 = c2_5;
			c2_5 = c1_5;
			c1_5 = c0_5;
			c0_5 = p_current_col->line5_pixel;

			//line6

			//c6_6 = c5_6;
			//c5_6 = c4_6;
			c4_6 = c3_6;
			c3_6 = c2_6;
			c2_6 = c1_6;
			c1_6 = c0_6;
			c0_6 = p_current_col->line6_pixel;

			circles->pixel_0 = c2_0;
			circles->pixel_1 = c3_0;
			circles->pixel_2 = c4_0;
			circles->pixel_3 = c5_1;
			circles->pixel_4 = c6_2;
			circles->pixel_5 = c6_3;
			circles->pixel_6 = c6_4;
			circles->pixel_7 = c5_5;
			circles->pixel_8 = c4_6;
			circles->pixel_9 = c3_6;
			circles->pixel_10 = c2_6;
			circles->pixel_11 = c1_5;
			circles->pixel_12 = c0_4;
			circles->pixel_13 = c0_3;
			circles->pixel_14 = c0_2;
			circles->pixel_15 = c1_1;
			circles->center = c3_3;
}

static
void classifier(bhrm_circle_t *circle,  uint8_t treshold, classfr_data_t * p_class_bus){
#pragma HLS INLINE
#pragma HLS INTERFACE ap_none port=treshold



	//PIXEL_0
	p_class_bus->dark.pre_score_0 = circle->center - circle->pixel_0;
	p_class_bus->dark.pre_score_0 -=treshold;

	p_class_bus->bright.pre_score_0 = circle->pixel_0 - circle->center;
	p_class_bus->bright.pre_score_0 -= treshold;


	//pixel 1
	p_class_bus->dark.pre_score_1 = circle->center - circle->pixel_1;
	p_class_bus->dark.pre_score_1 -= treshold;

	p_class_bus->bright.pre_score_1 = circle->pixel_1 - circle->center;
	p_class_bus->bright.pre_score_1 -= treshold;


	//pixel 2
	p_class_bus->dark.pre_score_2 = circle->center - circle->pixel_2;
	p_class_bus->dark.pre_score_2 -= treshold;

	p_class_bus->bright.pre_score_2 = circle->pixel_2 - circle->center;
	p_class_bus->bright.pre_score_2 -= treshold;

	//pixel 3
	p_class_bus->dark.pre_score_3 = circle->center - circle->pixel_3;
	p_class_bus->dark.pre_score_3 -= treshold;

	p_class_bus->bright.pre_score_3 = circle->pixel_3 - circle->center;
	p_class_bus->bright.pre_score_3 -= treshold;

	//pixel 4
	p_class_bus->dark.pre_score_4 = circle->center - circle->pixel_4;
	p_class_bus->dark.pre_score_4 -= treshold;

	p_class_bus->bright.pre_score_4 = circle->pixel_4 - circle->center;
	p_class_bus->bright.pre_score_4 -= treshold;


	//pixel 5
	p_class_bus->dark.pre_score_5 = circle->center - circle->pixel_5;
	p_class_bus->dark.pre_score_5 -= treshold;

	p_class_bus->bright.pre_score_5 = circle->pixel_5 - circle->center;
	p_class_bus->bright.pre_score_5 -= treshold;


	//pixel 6
	p_class_bus->dark.pre_score_6 = circle->center - circle->pixel_6;
	p_class_bus->dark.pre_score_6 -= treshold;

	p_class_bus->bright.pre_score_6 = circle->pixel_6 - circle->center;
	p_class_bus->bright.pre_score_6 -= treshold;



	//pixel 7
	p_class_bus->dark.pre_score_7 = circle->center - circle->pixel_7;
	p_class_bus->dark.pre_score_7 -= treshold;

	p_class_bus->bright.pre_score_7 = circle->pixel_7 - circle->center;
	p_class_bus->bright.pre_score_7 -= treshold;

	//pixel 8
	p_class_bus->dark.pre_score_8 = circle->center - circle->pixel_8;
	p_class_bus->dark.pre_score_8 -= treshold;

	p_class_bus->bright.pre_score_8 = circle->pixel_8 - circle->center;
	p_class_bus->bright.pre_score_8 -= treshold;



	//pixel 9
	p_class_bus->dark.pre_score_9 = circle->center - circle->pixel_9;
	p_class_bus->dark.pre_score_9 -= treshold;

	p_class_bus->bright.pre_score_9 = circle->pixel_9 - circle->center;
	p_class_bus->bright.pre_score_9 -= treshold;




	//pixel 10
	p_class_bus->dark.pre_score_10 = circle->center - circle->pixel_10;
	p_class_bus->dark.pre_score_10 -= treshold;


	p_class_bus->bright.pre_score_10 = circle->pixel_10 - circle->center;
	p_class_bus->bright.pre_score_10 -= treshold;



	//pixel 11
	p_class_bus->dark.pre_score_11 = circle->center - circle->pixel_11;
	p_class_bus->dark.pre_score_11 -= treshold;


	p_class_bus->bright.pre_score_11 = circle->pixel_11 - circle->center;
	p_class_bus->bright.pre_score_11 -= treshold;



	//pixel 12
	p_class_bus->dark.pre_score_12 = circle->center - circle->pixel_12;
	p_class_bus->dark.pre_score_12 -= treshold;

	p_class_bus->bright.pre_score_12 = circle->pixel_12 - circle->center;
	p_class_bus->bright.pre_score_12 -= treshold;




	//pixel 13
	p_class_bus->dark.pre_score_13 = circle->center - circle->pixel_13;
	p_class_bus->dark.pre_score_13 -= treshold;


	p_class_bus->bright.pre_score_13 = circle->pixel_13 - circle->center;
	p_class_bus->bright.pre_score_13 -= treshold;




	//pixel 14
	p_class_bus->dark.pre_score_14 = circle->center - circle->pixel_14;
	p_class_bus->dark.pre_score_14 -= treshold;


	p_class_bus->bright.pre_score_14 = circle->pixel_14 - circle->center;
	p_class_bus->bright.pre_score_14 -= treshold;



	//pixel 15
	p_class_bus->dark.pre_score_15 = circle->center - circle->pixel_15;
	p_class_bus->dark.pre_score_15 -= treshold;


	p_class_bus->bright.pre_score_15 = circle->pixel_15 - circle->center;
	p_class_bus->bright.pre_score_15 -= treshold;


}

static
void filter(classfr_data_t * p_class_bus, classfr_prescores_t * p_scores, classfr_flags_t * p_flags){
#pragma HLS INLINE

	//PIXEL_0

	if (p_class_bus->dark.pre_score_0 <= 0) {
		p_scores->dark.pre_score_0 = 0;
		p_flags->dark.f0= 0;
	}else{
		p_scores->dark.pre_score_0 = p_class_bus->dark.pre_score_0;
		p_flags->dark.f0=1;
	}

	if (p_class_bus->bright.pre_score_0 <= 0) {
		p_scores->bright.pre_score_0 = 0;
		p_flags->bright.f0 = 0;
	}else{
		p_scores->bright.pre_score_0 = p_class_bus->bright.pre_score_0;
		p_flags->bright.f0 =1;
	}

	//pixel 1

	if (p_class_bus->dark.pre_score_1 <= 0) {
		p_scores->dark.pre_score_1 = 0;
		p_flags->dark.f1= 0;
	}else{
		p_scores->dark.pre_score_1 = p_class_bus->dark.pre_score_1;
		p_flags->dark.f1=1;
	}

	if (p_class_bus->bright.pre_score_1 <= 0) {
		p_scores->bright.pre_score_1 = 0;
		p_flags->bright.f1 = 0;
	}else{
		p_scores->bright.pre_score_1 = p_class_bus->bright.pre_score_1;
		p_flags->bright.f1 =1;

	}


	//pixel 2
	if (p_class_bus->dark.pre_score_2 <= 0) {
		p_scores->dark.pre_score_2= 0;
		p_flags->dark.f2= 0;
	}else{
		p_scores->dark.pre_score_2 = p_class_bus->dark.pre_score_2;
		p_flags->dark.f2=1;

	}

	if (p_class_bus->bright.pre_score_2 <= 0) {
		p_scores->bright.pre_score_2 = 0;
		p_flags->bright.f2 = 0;
	}else{
		p_scores->bright.pre_score_2 = p_class_bus->bright.pre_score_2;
		p_flags->bright.f2 =1;
	}



	//pixel 3
	if (p_class_bus->dark.pre_score_3 <= 0) {
		p_scores->dark.pre_score_3 = 0;
		p_flags->dark.f3= 0;
	}else{
		p_scores->dark.pre_score_3 = p_class_bus->dark.pre_score_3;
		p_flags->dark.f3=1;
	}

	if (p_class_bus->bright.pre_score_3 <= 0) {
		p_scores->bright.pre_score_3 = 0;
		p_flags->bright.f3 = 0;
	}else{
		p_scores->bright.pre_score_3 = p_class_bus->bright.pre_score_3;
		p_flags->bright.f3 =1;
	}


	//pixel 4
	if (p_class_bus->dark.pre_score_4 <= 0) {
		p_scores->dark.pre_score_4 = 0;
		p_flags->dark.f4= 0;
	}else{
		p_scores->dark.pre_score_4 = p_class_bus->dark.pre_score_4;
		p_flags->dark.f4=1;
	}

	if (p_class_bus->bright.pre_score_4 <= 0) {
		p_scores->bright.pre_score_4 = 0;
		p_flags->bright.f4 = 0;
	}else{
		p_scores->bright.pre_score_4 = p_class_bus->bright.pre_score_4;
		p_flags->bright.f4 =1;
	}


	//pixel 5
	if (p_class_bus->dark.pre_score_5 <= 0) {
		p_scores->dark.pre_score_5 = 0;
		p_flags->dark.f5= 0;
	}else{
		p_scores->dark.pre_score_5 = p_class_bus->dark.pre_score_5;
		p_flags->dark.f5=1;
	}

	if (p_class_bus->bright.pre_score_5 <= 0) {
		p_scores->bright.pre_score_5 = 0;
		p_flags->bright.f5 = 0;
	}else{
		p_scores->bright.pre_score_5 = p_class_bus->bright.pre_score_5;
		p_flags->bright.f5 =1;
	}


	//pixel 6
	if (p_class_bus->dark.pre_score_6 <= 0) {
		p_scores->dark.pre_score_6 = 0;
		p_flags->dark.f6= 0;
	}else{
		p_scores->dark.pre_score_6 = p_class_bus->dark.pre_score_6;
		p_flags->dark.f6=1;
	}

	if (p_class_bus->bright.pre_score_6 <= 0) {
		p_scores->bright.pre_score_6 = 0;
		p_flags->bright.f6 = 0;
	}else{
		p_scores->bright.pre_score_6 = p_class_bus->bright.pre_score_6;
		p_flags->bright.f6 =1;
	}

	//pixel 7
	if (p_class_bus->dark.pre_score_7 <= 0) {
		p_scores->dark.pre_score_7 = 0;
		p_flags->dark.f7= 0;
	}else{
		p_scores->dark.pre_score_7 = p_class_bus->dark.pre_score_7;
		p_flags->dark.f7=1;
	}

	if (p_class_bus->bright.pre_score_7 <= 0) {
		p_scores->bright.pre_score_7= 0;
		p_flags->bright.f7 = 0;
	}else{
		p_scores->bright.pre_score_7 = p_class_bus->bright.pre_score_7;
		p_flags->bright.f7 =1;
	}


	//pixel 8
	if (p_class_bus->dark.pre_score_8 <= 0) {
		p_scores->dark.pre_score_8 = 0;
		p_flags->dark.f8= 0;
	}else{
		p_scores->dark.pre_score_8 = p_class_bus->dark.pre_score_8;
		p_flags->dark.f8=1;
	}

	if (p_class_bus->bright.pre_score_8 <= 0) {
		p_scores->bright.pre_score_8 = 0;
		p_flags->bright.f8 = 0;
	}else{
		p_scores->bright.pre_score_8 = p_class_bus->bright.pre_score_8;
		p_flags->bright.f8 =1;
	}



	//pixel 9
	if (p_class_bus->dark.pre_score_9 <= 0) {
		p_scores->dark.pre_score_9 = 0;
		p_flags->dark.f9= 0;
	}else{
		p_scores->dark.pre_score_9 = p_class_bus->dark.pre_score_9;
		p_flags->dark.f9=1;
	}

	if (p_class_bus->bright.pre_score_9 <= 0) {
		p_scores->bright.pre_score_9 = 0;
		p_flags->bright.f9 = 0;
	}else{
		p_scores->bright.pre_score_9 = p_class_bus->bright.pre_score_9;
		p_flags->bright.f9 =1;
	}


	//pixel 10
	if (p_class_bus->dark.pre_score_10 <= 0) {
		p_scores->dark.pre_score_10= 0;
		p_flags->dark.f10= 0;
	}else{
		p_scores->dark.pre_score_10 = p_class_bus->dark.pre_score_10;
		p_flags->dark.f10=1;
	}

	if (p_class_bus->bright.pre_score_10 <= 0) {
		p_scores->bright.pre_score_10 = 0;
		p_flags->bright.f10 = 0;
	}else{
		p_scores->bright.pre_score_10 = p_class_bus->bright.pre_score_10;
		p_flags->bright.f10 =1;
	}


	//pixel 11
	if (p_class_bus->dark.pre_score_11 <= 0) {
		p_scores->dark.pre_score_11 = 0;
		p_flags->dark.f11= 0;
	}else{
		p_scores->dark.pre_score_11 = p_class_bus->dark.pre_score_11;
		p_flags->dark.f11=1;
	}

	if (p_class_bus->bright.pre_score_11 <= 0) {
		p_scores->bright.pre_score_11 = 0;
		p_flags->bright.f11 = 0;
	}else{
		p_scores->bright.pre_score_11 = p_class_bus->bright.pre_score_11;
		p_flags->bright.f11 =1;
	}

	//pixel 12
	if (p_class_bus->dark.pre_score_12 <= 0) {
		p_scores->dark.pre_score_12 = 0;
		p_flags->dark.f12= 0;
	}else{
		p_scores->dark.pre_score_12 = p_class_bus->dark.pre_score_12;
		p_flags->dark.f12=1;
	}

	if (p_class_bus->bright.pre_score_12 <= 0) {
		p_scores->bright.pre_score_12= 0;
		p_flags->bright.f12 = 0;
	}else{
		p_scores->bright.pre_score_12 = p_class_bus->bright.pre_score_12;
		p_flags->bright.f12 =1;
	}


	//pixel 13
	if (p_class_bus->dark.pre_score_13 <= 0) {
		p_scores->dark.pre_score_13 = 0;
		p_flags->dark.f13= 0;
	}else{
		p_scores->dark.pre_score_13 = p_class_bus->dark.pre_score_13;
		p_flags->dark.f13=1;
	}

	if (p_class_bus->bright.pre_score_13 <= 0) {
		p_scores->bright.pre_score_13 = 0;
		p_flags->bright.f13 = 0;
	}else{
		p_scores->bright.pre_score_13 = p_class_bus->bright.pre_score_13;
		p_flags->bright.f13 =1;
	}


	//pixel 14
	if (p_class_bus->dark.pre_score_14 <= 0) {
		p_scores->dark.pre_score_14 = 0;
		p_flags->dark.f14= 0;
	}else{
		p_scores->dark.pre_score_14 = p_class_bus->dark.pre_score_14;
		p_flags->dark.f14=1;
	}

	if (p_class_bus->bright.pre_score_14 <= 0) {
		p_scores->bright.pre_score_14= 0;
		p_flags->bright.f14 = 0;
	}else{
		p_scores->bright.pre_score_14 = p_class_bus->bright.pre_score_14;
		p_flags->bright.f14 =1;
	}


	//pixel 15
	if (p_class_bus->dark.pre_score_15 <= 0) {
		p_scores->dark.pre_score_15 = 0;
		p_flags->dark.f15= 0;
	}else{
		p_scores->dark.pre_score_15 = p_class_bus->dark.pre_score_15;
		p_flags->dark.f15=1;
	}

	if (p_class_bus->bright.pre_score_15 <= 0) {
		p_scores->bright.pre_score_15 = 0;
		p_flags->bright.f15 = 0;
	}else{
		p_scores->bright.pre_score_15 = p_class_bus->bright.pre_score_15;
		p_flags->bright.f15 =1;
	}



}

static
void contiguity(classfr_flags_t* p_flags,uint1 *p_is_darker, uint1 *p_is_brighter){
#pragma HLS INLINE
	uint8_t i;
	uint1 is_darker, is_brighter;
	const uint32_t mask = ((1<<9)-1);
	uint32_t rot_mask = ((1 << 9) - 1);
	uint32_t dark_bus_i,bright_bus_i;

	dark_bus_i = (p_flags->dark.f0) |
				 (p_flags->dark.f1 <<1) |
				 (p_flags->dark.f2 <<2) |
				 (p_flags->dark.f3 <<3) |
				 (p_flags->dark.f4 <<4) |
				 (p_flags->dark.f5 <<5) |
				 (p_flags->dark.f6 <<6) |
				 (p_flags->dark.f7 <<7) |
				 (p_flags->dark.f8 <<8) |
				 (p_flags->dark.f9 <<9) |
				 (p_flags->dark.f10 <<10) |
				 (p_flags->dark.f11 <<11) |
				 (p_flags->dark.f12 <<12) |
				 (p_flags->dark.f13 <<13) |
				 (p_flags->dark.f14 <<14) |
				 (p_flags->dark.f15 <<15)|
				 (p_flags->dark.f0 <<16) |
				 (p_flags->dark.f1 <<17) |
				 (p_flags->dark.f2 <<18) |
				 (p_flags->dark.f3 <<19) |
				 (p_flags->dark.f4 <<20) |
				 (p_flags->dark.f5 <<21) |
				 (p_flags->dark.f6 <<22) |
				 (p_flags->dark.f7 <<23) |
				 (p_flags->dark.f8 <<24);




	bright_bus_i = (p_flags->bright.f0)|
				(p_flags->bright.f1<<1)|
				(p_flags->bright.f2<<2)|
				(p_flags->bright.f3<<3)|
				(p_flags->bright.f4<<4)|
				(p_flags->bright.f5<<5)|
				(p_flags->bright.f6<<6)|
				(p_flags->bright.f7<<7)|
				(p_flags->bright.f8<<8)|
				(p_flags->bright.f9<<9)|
				(p_flags->bright.f10<<10)|
				(p_flags->bright.f11<<11)|
				(p_flags->bright.f12<<12)|
				(p_flags->bright.f13<<13)|
				(p_flags->bright.f14<<14)|
				(p_flags->bright.f15<<15)|
				(p_flags->bright.f0<<16)|
				(p_flags->bright.f1<<17)|
				(p_flags->bright.f2<<18)|
				(p_flags->bright.f3<<19)|
				(p_flags->bright.f4<<20)|
				(p_flags->bright.f5<<21)|
				(p_flags->bright.f6<<22)|
				(p_flags->bright.f7<<23)|
				(p_flags->bright.f8<<24);


			is_darker = 0;
			is_brighter = 0;
			for (i = 0; i < 15; i++,rot_mask <<= 1) {
			#pragma HLS UNROLL
				is_darker 	|= (dark_bus_i & rot_mask) == rot_mask;
				is_brighter |= (bright_bus_i & rot_mask)== rot_mask;

			}


			is_darker 		|= (dark_bus_i & rot_mask) == rot_mask;
			is_brighter 	|= (bright_bus_i & rot_mask)== rot_mask;


			*p_is_brighter = is_brighter & ~ is_darker;
			*p_is_darker = ~is_brighter &  is_darker;

}

static
void scoring(classfr_prescores_t* p_scores, uint16_t *p_dark_score, uint16_t *p_bright_score){
#pragma HLS INLINE

	uint16_t temp_dark_score_0_0,temp_dark_score_0_1,temp_dark_score_0_2,temp_dark_score_0_3,temp_dark_score_0_4,temp_dark_score_0_5,temp_dark_score_0_6,temp_dark_score_0_7;
	uint16_t temp_dark_score_1_0,temp_dark_score_1_1,temp_dark_score_1_2,temp_dark_score_1_3;
	uint16_t temp_dark_score_2_0,temp_dark_score_2_1;

	uint16_t temp_bright_score_0_0,temp_bright_score_0_1,temp_bright_score_0_2,temp_bright_score_0_3,temp_bright_score_0_4,temp_bright_score_0_5,temp_bright_score_0_6,temp_bright_score_0_7;
	uint16_t temp_bright_score_1_0,temp_bright_score_1_1,temp_bright_score_1_2,temp_bright_score_1_3;
	uint16_t temp_bright_score_2_0,temp_bright_score_2_1;


	temp_dark_score_0_0 = p_scores->dark.pre_score_0 + p_scores->dark.pre_score_1;
	temp_dark_score_0_1 = p_scores->dark.pre_score_2 + p_scores->dark.pre_score_3;
	temp_dark_score_0_2 = p_scores->dark.pre_score_4 + p_scores->dark.pre_score_5;
	temp_dark_score_0_3 = p_scores->dark.pre_score_6 + p_scores->dark.pre_score_7;
	temp_dark_score_0_4 = p_scores->dark.pre_score_8 + p_scores->dark.pre_score_9;
	temp_dark_score_0_5 = p_scores->dark.pre_score_10 + p_scores->dark.pre_score_11;
	temp_dark_score_0_6 = p_scores->dark.pre_score_12 + p_scores->dark.pre_score_13;
	temp_dark_score_0_7 = p_scores->dark.pre_score_14 + p_scores->dark.pre_score_15;



	temp_bright_score_0_0 = p_scores->bright.pre_score_0 + p_scores->bright.pre_score_1;
	temp_bright_score_0_1 = p_scores->bright.pre_score_2 + p_scores->bright.pre_score_3;
	temp_bright_score_0_2 = p_scores->bright.pre_score_4 + p_scores->bright.pre_score_5;
	temp_bright_score_0_3 = p_scores->bright.pre_score_6 + p_scores->bright.pre_score_7;
	temp_bright_score_0_4 = p_scores->bright.pre_score_8 + p_scores->bright.pre_score_9;
	temp_bright_score_0_5 = p_scores->bright.pre_score_10 + p_scores->bright.pre_score_11;
	temp_bright_score_0_6 = p_scores->bright.pre_score_12 + p_scores->bright.pre_score_13;
	temp_bright_score_0_7 = p_scores->bright.pre_score_14 + p_scores->bright.pre_score_15;

	temp_dark_score_1_0 = temp_dark_score_0_0 + temp_dark_score_0_1;
	temp_dark_score_1_1 = temp_dark_score_0_2 + temp_dark_score_0_3;
	temp_dark_score_1_2 = temp_dark_score_0_4 + temp_dark_score_0_5;
	temp_dark_score_1_3 = temp_dark_score_0_6 + temp_dark_score_0_7;


	temp_bright_score_1_0 = temp_bright_score_0_0 + temp_bright_score_0_1;
	temp_bright_score_1_1 = temp_bright_score_0_2 + temp_bright_score_0_3;
	temp_bright_score_1_2 = temp_bright_score_0_4 + temp_bright_score_0_5;
	temp_bright_score_1_3 = temp_bright_score_0_6 + temp_bright_score_0_7;

	temp_dark_score_2_0 = temp_dark_score_1_0 + temp_dark_score_1_1;
	temp_dark_score_2_1 = temp_dark_score_1_2 + temp_dark_score_1_3;

	temp_bright_score_2_0 = temp_bright_score_1_0 + temp_bright_score_1_1;
	temp_bright_score_2_1 = temp_bright_score_1_2 + temp_bright_score_1_3;

	*p_dark_score = temp_dark_score_2_0+temp_dark_score_2_1;
	*p_bright_score = temp_bright_score_2_0+temp_bright_score_2_1;

}

static
void score_selection(
		uint16_t dark_score,
		uint16_t bright_score,
		uint1 is_darker, uint1 is_brighter,
		uint1 *is_keypoint, uint16_t *score){
#pragma HLS INLINE

	if(is_darker == 1)
		*score = dark_score;
	else if (is_brighter == 1)
		*score = bright_score;
	else
		*score = 0;
	*is_keypoint = is_darker|is_brighter;

}


void fast9_16(uint8_t i_pixel[IMG_HEIGH*IMG_WIDTH],
		uint8_t threshold, uint1 * o_is_keypoint,
		uint16_t o_score[IMG_HEIGH*IMG_WIDTH], uint1* o_score_valid,
		uint16_t* x_coord, uint16_t* y_coord ){
#pragma HLS INTERFACE ap_fifo port=i_pixel
#pragma HLS INTERFACE ap_fifo port=o_score
cache_colmn_t colmn;bhrm_circle_t circle;
classfr_data_t class_data;classfr_prescores_t prescores;
classfr_flags_t flags; uint1 is_darker, is_brighter;
int i,k_y,k_x; uint16_t dark_score, bright_score;
 for( i=0,k_y=0; k_y < IMG_HEIGH; k_y++){
  for(k_x=0; k_x < IMG_WIDTH; k_x++, i++ ){
  #pragma HLS PIPELINE rewind
    cache_img_lines(i_pixel[i],k_x,&colmn);
   	 pixel_matrix(&colmn, &circle);
	  classifier(&circle, threshold, &class_data);
	   filter(&class_data,&prescores, &flags);
	    scoring(&prescores,&dark_score,&bright_score);
		 contiguity(&flags,&is_darker,&is_brighter);
	      score_selection(dark_score, bright_score, is_darker, 
				is_brighter, o_is_keypoint, &(o_score[i]));
	compute_coordinates(k_x,k_y, x_coord, y_coord,o_score_valid);
}}}
