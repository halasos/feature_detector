#include "fast_dual.h"

void compute_coordinates_double(uint16_t k_x, uint16_t* x_coord1,  uint16_t* x_coord0, uint16_t *y_coord1, uint16_t *y_coord0,uint1* coord_valid){
#pragma HLS INLINE


	static int16_t x_coord_d0,x_coord_d1,x_coord_d2,x_coord_d3,x_coord3_p;
	static int16_t y_coord_d0=-3,y_coord_d1,y_coord_d2,y_coord_d3,y_coord_d4;//,y_coord_d5;
	static int1 l0,l1,l2,l3,p0,p1,p2,p3;

	p3=p2;
	//delay k0
	p2=p1;
	p1=p0;
	p0=l3;

	x_coord3_p = x_coord_d2+1;
	x_coord_d3 = x_coord_d2;
	x_coord_d2 = x_coord_d1;
	x_coord_d1 = x_coord_d0;
	x_coord_d0 = k_x;



	y_coord_d4 = y_coord_d3;
	y_coord_d3 = y_coord_d2;
	y_coord_d2 = y_coord_d1;
	y_coord_d1 = y_coord_d0;

	if(k_x==(IMG_WIDTH-2) ){
		if(y_coord_d0 < IMG_HEIGH-1){
			y_coord_d0++;
		}else{
			y_coord_d0=0;
		}
		l3=l2;
		l2=l1;
		l1=1;

	}

	*x_coord0 = x_coord_d3;
	*y_coord0 = y_coord_d4;

	*x_coord1 = x_coord3_p;
	*y_coord1 = y_coord_d4;


	*coord_valid = p3;

}

void cache_img_lines_double(uint8_t pixel1,uint8_t pixel0,uint16_t col_index,	cache_colmn_t *col1,cache_colmn_t *col0){
#pragma HLS INLINE

	static uint16_t line1_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line1_i core=RAM_T2P_BRAM
	static uint16_t line2_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line2_i core=RAM_T2P_BRAM
	static uint16_t line3_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line3_i core=RAM_T2P_BRAM
	static uint16_t line4_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line4_i core=RAM_T2P_BRAM
	static uint16_t line5_i[RAM_LINE];
	#pragma HLS RESOURCE variable=line5_i core=RAM_T2P_BRAM
	static uint16_t line6_i[RAM_LINE];
	#pragma dual_pixel_t RESOURCE variable=line6_i core=RAM_T2P_BRAM

		//output column
		col0->line6_pixel = (uint8_t)line6_i[col_index];
		col0->line5_pixel = (uint8_t)line5_i[col_index];
		col0->line4_pixel = (uint8_t)line4_i[col_index];
		col0->line3_pixel = (uint8_t)line3_i[col_index];
		col0->line2_pixel = (uint8_t)line2_i[col_index];
		col0->line1_pixel = (uint8_t)line1_i[col_index];
		col0->line0_pixel = pixel0;

		col1->line6_pixel = (uint8_t)(line6_i[col_index]>>8);
		col1->line5_pixel = (uint8_t)(line5_i[col_index]>>8);
		col1->line4_pixel = (uint8_t)(line4_i[col_index]>>8);
		col1->line3_pixel = (uint8_t)(line3_i[col_index]>>8);
		col1->line2_pixel = (uint8_t)(line2_i[col_index]>>8);
		col1->line1_pixel = (uint8_t)(line1_i[col_index]>>8);
		col1->line0_pixel = pixel1;
		//Forward pixels between lines
		line6_i[col_index] = line5_i[col_index];
		line5_i[col_index] = line4_i[col_index];
		line4_i[col_index] = line3_i[col_index];
		line3_i[col_index] = line2_i[col_index];
		line2_i[col_index] = line1_i[col_index];
		line1_i[col_index] = pixel1<<8 | pixel0;
}

void pixel_matrix_double (cache_colmn_t *c1, cache_colmn_t *c0,
		bhrm_circle_t *bc1,bhrm_circle_t *bc0){
#pragma HLS INLINE


	static uint8_t c0_0, c0_1, c0_2, c0_3, c0_4, c0_5, c0_6;
	static uint8_t c1_0, c1_1, c1_2, c1_3, c1_4, c1_5, c1_6;
	static uint8_t c2_0, c2_1, c2_2, c2_3, c2_4, c2_5, c2_6;
	static uint8_t c3_0, c3_1, c3_2, c3_3, c3_4, c3_5, c3_6;
	static uint8_t c4_0, c4_1, c4_2, c4_3, c4_4, c4_5, c4_6;
	static uint8_t c5_0, c5_1, c5_2, c5_3, c5_4, c5_5, c5_6;
	static uint8_t c6_0, c6_1, c6_2, c6_3, c6_4, c6_5 ,c6_6;
	static uint8_t c7_0, c7_1, c7_2, c7_3, c7_4, c7_5 ,c7_6;


	bc0->pixel_0 = c2_0;
	bc0->pixel_1 = c3_0;
	bc0->pixel_2 = c4_0;
	bc0->pixel_3 = c5_1;
	bc0->pixel_4 = c6_2;
	bc0->pixel_5 = c6_3;
	bc0->pixel_6 = c6_4;
	bc0->pixel_7 = c5_5;
	bc0->pixel_8 = c4_6;
	bc0->pixel_9 = c3_6;
	bc0->pixel_10 = c2_6;
	bc0->pixel_11 = c1_5;
	bc0->pixel_12 = c0_4;
	bc0->pixel_13 = c0_3;
	bc0->pixel_14 = c0_2;
	bc0->pixel_15 = c1_1;
	bc0->center = c3_3;


	bc1->pixel_0 = c3_0;
	bc1->pixel_1 = c4_0;
	bc1->pixel_2 = c5_0;
	bc1->pixel_3 = c6_1;
	bc1->pixel_4 = c7_2;
	bc1->pixel_5 = c7_3;
	bc1->pixel_6 = c7_4;
	bc1->pixel_7 = c6_5;
	bc1->pixel_8 = c5_6;
	bc1->pixel_9 = c4_6;
	bc1->pixel_10 = c3_6;
	bc1->pixel_11 = c2_5;
	bc1->pixel_12 = c1_4;
	bc1->pixel_13 = c1_3;
	bc1->pixel_14 = c1_2;
	bc1->pixel_15 = c2_1;
	bc1->center = c4_3;


			//line0
			c7_0 = c5_0;
			c6_0 = c4_0;
			c5_0 = c3_0;
			c4_0 = c2_0;
			c3_0 = c1_0;
			c2_0 = c0_0;
			c1_0 = c0->line0_pixel;
			c0_0 = c1->line0_pixel;



			//line1
			c7_1 = c5_1;
			c6_1 = c4_1;
			c5_1 = c3_1;
			c4_1 = c2_1;
			c3_1 = c1_1;
			c2_1 = c0_1;
			c1_1 = c0->line1_pixel;
			c0_1 = c1->line1_pixel;


			//line2

			c7_2 = c5_2;
			c6_2 = c4_2;
			c5_2 = c3_2;
			c4_2 = c2_2;
			c3_2 = c1_2;
			c2_2 = c0_2;
			c1_2 = c0->line2_pixel;
			c0_2 = c1->line2_pixel;
			//line3

			c7_3 = c5_3;
			c6_3 = c4_3;
			c5_3 = c3_3;
			c4_3 = c2_3;
			c3_3 = c1_3;
			c2_3 = c0_3;
			c1_3 = c0->line3_pixel;
			c0_3 = c1->line3_pixel;

			//line4

			c7_4 = c5_4;
			c6_4 = c4_4;
			c5_4 = c3_4;
			c4_4 = c2_4;
			c3_4 = c1_4;
			c2_4 = c0_4;
			c1_4 = c0->line4_pixel;
			c0_4 = c1->line4_pixel;

			//line5

			c7_5 = c5_5;
			c6_5 = c4_5;
			c5_5 = c3_5;
			c4_5 = c2_5;
			c3_5 = c1_5;
			c2_5 = c0_5;
			c1_5 = c0->line5_pixel;
			c0_5 = c1->line5_pixel;

			//line6

			c7_6 = c5_6;
			c6_6 = c4_6;
			c5_6 = c3_6;
			c4_6 = c2_6;
			c3_6 = c1_6;
			c2_6 = c0_6;
			c1_6 = c0->line6_pixel;
			c0_6 = c1->line6_pixel;


}

static
void classifier(bhrm_circle_t *circle,  uint8_t treshold, classifilter_data_t * p_class_bus){
#pragma HLS INLINE
#pragma HLS INTERFACE ap_none port=treshold



	//PIXEL_0
	p_class_bus->dark_prscrs.pre_score_0 = circle->center - circle->pixel_0;
	p_class_bus->dark_prscrs.pre_score_0 -=treshold;

	p_class_bus->bright_prscrs.pre_score_0 = circle->pixel_0 - circle->center;
	p_class_bus->bright_prscrs.pre_score_0 -= treshold;


	//pixel 1
	p_class_bus->dark_prscrs.pre_score_1 = circle->center - circle->pixel_1;
	p_class_bus->dark_prscrs.pre_score_1 -= treshold;

	p_class_bus->bright_prscrs.pre_score_1 = circle->pixel_1 - circle->center;
	p_class_bus->bright_prscrs.pre_score_1 -= treshold;


	//pixel 2
	p_class_bus->dark_prscrs.pre_score_2 = circle->center - circle->pixel_2;
	p_class_bus->dark_prscrs.pre_score_2 -= treshold;

	p_class_bus->bright_prscrs.pre_score_2 = circle->pixel_2 - circle->center;
	p_class_bus->bright_prscrs.pre_score_2 -= treshold;

	//pixel 3
	p_class_bus->dark_prscrs.pre_score_3 = circle->center - circle->pixel_3;
	p_class_bus->dark_prscrs.pre_score_3 -= treshold;

	p_class_bus->bright_prscrs.pre_score_3 = circle->pixel_3 - circle->center;
	p_class_bus->bright_prscrs.pre_score_3 -= treshold;

	//pixel 4
	p_class_bus->dark_prscrs.pre_score_4 = circle->center - circle->pixel_4;
	p_class_bus->dark_prscrs.pre_score_4 -= treshold;

	p_class_bus->bright_prscrs.pre_score_4 = circle->pixel_4 - circle->center;
	p_class_bus->bright_prscrs.pre_score_4 -= treshold;


	//pixel 5
	p_class_bus->dark_prscrs.pre_score_5 = circle->center - circle->pixel_5;
	p_class_bus->dark_prscrs.pre_score_5 -= treshold;

	p_class_bus->bright_prscrs.pre_score_5 = circle->pixel_5 - circle->center;
	p_class_bus->bright_prscrs.pre_score_5 -= treshold;


	//pixel 6
	p_class_bus->dark_prscrs.pre_score_6 = circle->center - circle->pixel_6;
	p_class_bus->dark_prscrs.pre_score_6 -= treshold;

	p_class_bus->bright_prscrs.pre_score_6 = circle->pixel_6 - circle->center;
	p_class_bus->bright_prscrs.pre_score_6 -= treshold;



	//pixel 7
	p_class_bus->dark_prscrs.pre_score_7 = circle->center - circle->pixel_7;
	p_class_bus->dark_prscrs.pre_score_7 -= treshold;

	p_class_bus->bright_prscrs.pre_score_7 = circle->pixel_7 - circle->center;
	p_class_bus->bright_prscrs.pre_score_7 -= treshold;

	//pixel 8
	p_class_bus->dark_prscrs.pre_score_8 = circle->center - circle->pixel_8;
	p_class_bus->dark_prscrs.pre_score_8 -= treshold;

	p_class_bus->bright_prscrs.pre_score_8 = circle->pixel_8 - circle->center;
	p_class_bus->bright_prscrs.pre_score_8 -= treshold;



	//pixel 9
	p_class_bus->dark_prscrs.pre_score_9 = circle->center - circle->pixel_9;
	p_class_bus->dark_prscrs.pre_score_9 -= treshold;

	p_class_bus->bright_prscrs.pre_score_9 = circle->pixel_9 - circle->center;
	p_class_bus->bright_prscrs.pre_score_9 -= treshold;




	//pixel 10
	p_class_bus->dark_prscrs.pre_score_10 = circle->center - circle->pixel_10;
	p_class_bus->dark_prscrs.pre_score_10 -= treshold;


	p_class_bus->bright_prscrs.pre_score_10 = circle->pixel_10 - circle->center;
	p_class_bus->bright_prscrs.pre_score_10 -= treshold;



	//pixel 11
	p_class_bus->dark_prscrs.pre_score_11 = circle->center - circle->pixel_11;
	p_class_bus->dark_prscrs.pre_score_11 -= treshold;


	p_class_bus->bright_prscrs.pre_score_11 = circle->pixel_11 - circle->center;
	p_class_bus->bright_prscrs.pre_score_11 -= treshold;



	//pixel 12
	p_class_bus->dark_prscrs.pre_score_12 = circle->center - circle->pixel_12;
	p_class_bus->dark_prscrs.pre_score_12 -= treshold;

	p_class_bus->bright_prscrs.pre_score_12 = circle->pixel_12 - circle->center;
	p_class_bus->bright_prscrs.pre_score_12 -= treshold;




	//pixel 13
	p_class_bus->dark_prscrs.pre_score_13 = circle->center - circle->pixel_13;
	p_class_bus->dark_prscrs.pre_score_13 -= treshold;


	p_class_bus->bright_prscrs.pre_score_13 = circle->pixel_13 - circle->center;
	p_class_bus->bright_prscrs.pre_score_13 -= treshold;




	//pixel 14
	p_class_bus->dark_prscrs.pre_score_14 = circle->center - circle->pixel_14;
	p_class_bus->dark_prscrs.pre_score_14 -= treshold;


	p_class_bus->bright_prscrs.pre_score_14 = circle->pixel_14 - circle->center;
	p_class_bus->bright_prscrs.pre_score_14 -= treshold;



	//pixel 15
	p_class_bus->dark_prscrs.pre_score_15 = circle->center - circle->pixel_15;
	p_class_bus->dark_prscrs.pre_score_15 -= treshold;


	p_class_bus->bright_prscrs.pre_score_15 = circle->pixel_15 - circle->center;
	p_class_bus->bright_prscrs.pre_score_15 -= treshold;

	p_class_bus->center = circle->center;

}

static
void filter(classifilter_data_t * p_class_bus, classifilter_data_t *p_class){
#pragma HLS INLINE

	//PIXEL_0

	if (p_class_bus->dark_prscrs.pre_score_0 <= 0) {
		p_class->dark_prscrs.pre_score_0 = 0;
		p_class->dark_flags.f0= 0;
	}else{
		p_class->dark_prscrs.pre_score_0 = p_class_bus->dark_prscrs.pre_score_0;
		p_class->dark_flags.f0=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_0 <= 0) {
		p_class->bright_prscrs.pre_score_0 = 0;
		p_class->bright_flags.f0 = 0;
	}else{
		p_class->bright_prscrs.pre_score_0 = p_class_bus->bright_prscrs.pre_score_0;
		p_class->bright_flags.f0 =1;
	}

	//pixel 1

	if (p_class_bus->dark_prscrs.pre_score_1 <= 0) {
		p_class->dark_prscrs.pre_score_1 = 0;
		p_class->dark_flags.f1= 0;
	}else{
		p_class->dark_prscrs.pre_score_1 = p_class_bus->dark_prscrs.pre_score_1;
		p_class->dark_flags.f1=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_1 <= 0) {
		p_class->bright_prscrs.pre_score_1 = 0;
		p_class->bright_flags.f1 = 0;
	}else{
		p_class->bright_prscrs.pre_score_1 = p_class_bus->bright_prscrs.pre_score_1;
		p_class->bright_flags.f1 =1;

	}


	//pixel 2
	if (p_class_bus->dark_prscrs.pre_score_2 <= 0) {
		p_class->dark_prscrs.pre_score_2= 0;
		p_class->dark_flags.f2= 0;
	}else{
		p_class->dark_prscrs.pre_score_2 = p_class_bus->dark_prscrs.pre_score_2;
		p_class->dark_flags.f2=1;

	}

	if (p_class_bus->bright_prscrs.pre_score_2 <= 0) {
		p_class->bright_prscrs.pre_score_2 = 0;
		p_class->bright_flags.f2 = 0;
	}else{
		p_class->bright_prscrs.pre_score_2 = p_class_bus->bright_prscrs.pre_score_2;
		p_class->bright_flags.f2 =1;
	}



	//pixel 3
	if (p_class_bus->dark_prscrs.pre_score_3 <= 0) {
		p_class->dark_prscrs.pre_score_3 = 0;
		p_class->dark_flags.f3= 0;
	}else{
		p_class->dark_prscrs.pre_score_3 = p_class_bus->dark_prscrs.pre_score_3;
		p_class->dark_flags.f3=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_3 <= 0) {
		p_class->bright_prscrs.pre_score_3 = 0;
		p_class->bright_flags.f3 = 0;
	}else{
		p_class->bright_prscrs.pre_score_3 = p_class_bus->bright_prscrs.pre_score_3;
		p_class->bright_flags.f3 =1;
	}


	//pixel 4
	if (p_class_bus->dark_prscrs.pre_score_4 <= 0) {
		p_class->dark_prscrs.pre_score_4 = 0;
		p_class->dark_flags.f4= 0;
	}else{
		p_class->dark_prscrs.pre_score_4 = p_class_bus->dark_prscrs.pre_score_4;
		p_class->dark_flags.f4=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_4 <= 0) {
		p_class->bright_prscrs.pre_score_4 = 0;
		p_class->bright_flags.f4 = 0;
	}else{
		p_class->bright_prscrs.pre_score_4 = p_class_bus->bright_prscrs.pre_score_4;
		p_class->bright_flags.f4 =1;
	}


	//pixel 5
	if (p_class_bus->dark_prscrs.pre_score_5 <= 0) {
		p_class->dark_prscrs.pre_score_5 = 0;
		p_class->dark_flags.f5= 0;
	}else{
		p_class->dark_prscrs.pre_score_5 = p_class_bus->dark_prscrs.pre_score_5;
		p_class->dark_flags.f5=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_5 <= 0) {
		p_class->bright_prscrs.pre_score_5 = 0;
		p_class->bright_flags.f5 = 0;
	}else{
		p_class->bright_prscrs.pre_score_5 = p_class_bus->bright_prscrs.pre_score_5;
		p_class->bright_flags.f5 =1;
	}


	//pixel 6
	if (p_class_bus->dark_prscrs.pre_score_6 <= 0) {
		p_class->dark_prscrs.pre_score_6 = 0;
		p_class->dark_flags.f6= 0;
	}else{
		p_class->dark_prscrs.pre_score_6 = p_class_bus->dark_prscrs.pre_score_6;
		p_class->dark_flags.f6=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_6 <= 0) {
		p_class->bright_prscrs.pre_score_6 = 0;
		p_class->bright_flags.f6 = 0;
	}else{
		p_class->bright_prscrs.pre_score_6 = p_class_bus->bright_prscrs.pre_score_6;
		p_class->bright_flags.f6 =1;
	}

	//pixel 7
	if (p_class_bus->dark_prscrs.pre_score_7 <= 0) {
		p_class->dark_prscrs.pre_score_7 = 0;
		p_class->dark_flags.f7= 0;
	}else{
		p_class->dark_prscrs.pre_score_7 = p_class_bus->dark_prscrs.pre_score_7;
		p_class->dark_flags.f7=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_7 <= 0) {
		p_class->bright_prscrs.pre_score_7= 0;
		p_class->bright_flags.f7 = 0;
	}else{
		p_class->bright_prscrs.pre_score_7 = p_class_bus->bright_prscrs.pre_score_7;
		p_class->bright_flags.f7 =1;
	}


	//pixel 8
	if (p_class_bus->dark_prscrs.pre_score_8 <= 0) {
		p_class->dark_prscrs.pre_score_8 = 0;
		p_class->dark_flags.f8= 0;
	}else{
		p_class->dark_prscrs.pre_score_8 = p_class_bus->dark_prscrs.pre_score_8;
		p_class->dark_flags.f8=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_8 <= 0) {
		p_class->bright_prscrs.pre_score_8 = 0;
		p_class->bright_flags.f8 = 0;
	}else{
		p_class->bright_prscrs.pre_score_8 = p_class_bus->bright_prscrs.pre_score_8;
		p_class->bright_flags.f8 =1;
	}



	//pixel 9
	if (p_class_bus->dark_prscrs.pre_score_9 <= 0) {
		p_class->dark_prscrs.pre_score_9 = 0;
		p_class->dark_flags.f9= 0;
	}else{
		p_class->dark_prscrs.pre_score_9 = p_class_bus->dark_prscrs.pre_score_9;
		p_class->dark_flags.f9=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_9 <= 0) {
		p_class->bright_prscrs.pre_score_9 = 0;
		p_class->bright_flags.f9 = 0;
	}else{
		p_class->bright_prscrs.pre_score_9 = p_class_bus->bright_prscrs.pre_score_9;
		p_class->bright_flags.f9 =1;
	}


	//pixel 10
	if (p_class_bus->dark_prscrs.pre_score_10 <= 0) {
		p_class->dark_prscrs.pre_score_10= 0;
		p_class->dark_flags.f10= 0;
	}else{
		p_class->dark_prscrs.pre_score_10 = p_class_bus->dark_prscrs.pre_score_10;
		p_class->dark_flags.f10=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_10 <= 0) {
		p_class->bright_prscrs.pre_score_10 = 0;
		p_class->bright_flags.f10 = 0;
	}else{
		p_class->bright_prscrs.pre_score_10 = p_class_bus->bright_prscrs.pre_score_10;
		p_class->bright_flags.f10 =1;
	}


	//pixel 11
	if (p_class_bus->dark_prscrs.pre_score_11 <= 0) {
		p_class->dark_prscrs.pre_score_11 = 0;
		p_class->dark_flags.f11= 0;
	}else{
		p_class->dark_prscrs.pre_score_11 = p_class_bus->dark_prscrs.pre_score_11;
		p_class->dark_flags.f11=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_11 <= 0) {
		p_class->bright_prscrs.pre_score_11 = 0;
		p_class->bright_flags.f11 = 0;
	}else{
		p_class->bright_prscrs.pre_score_11 = p_class_bus->bright_prscrs.pre_score_11;
		p_class->bright_flags.f11 =1;
	}

	//pixel 12
	if (p_class_bus->dark_prscrs.pre_score_12 <= 0) {
		p_class->dark_prscrs.pre_score_12 = 0;
		p_class->dark_flags.f12= 0;
	}else{
		p_class->dark_prscrs.pre_score_12 = p_class_bus->dark_prscrs.pre_score_12;
		p_class->dark_flags.f12=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_12 <= 0) {
		p_class->bright_prscrs.pre_score_12= 0;
		p_class->bright_flags.f12 = 0;
	}else{
		p_class->bright_prscrs.pre_score_12 = p_class_bus->bright_prscrs.pre_score_12;
		p_class->bright_flags.f12 =1;
	}


	//pixel 13
	if (p_class_bus->dark_prscrs.pre_score_13 <= 0) {
		p_class->dark_prscrs.pre_score_13 = 0;
		p_class->dark_flags.f13= 0;
	}else{
		p_class->dark_prscrs.pre_score_13 = p_class_bus->dark_prscrs.pre_score_13;
		p_class->dark_flags.f13=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_13 <= 0) {
		p_class->bright_prscrs.pre_score_13 = 0;
		p_class->bright_flags.f13 = 0;
	}else{
		p_class->bright_prscrs.pre_score_13 = p_class_bus->bright_prscrs.pre_score_13;
		p_class->bright_flags.f13 =1;
	}


	//pixel 14
	if (p_class_bus->dark_prscrs.pre_score_14 <= 0) {
		p_class->dark_prscrs.pre_score_14 = 0;
		p_class->dark_flags.f14= 0;
	}else{
		p_class->dark_prscrs.pre_score_14 = p_class_bus->dark_prscrs.pre_score_14;
		p_class->dark_flags.f14=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_14 <= 0) {
		p_class->bright_prscrs.pre_score_14= 0;
		p_class->bright_flags.f14 = 0;
	}else{
		p_class->bright_prscrs.pre_score_14 = p_class_bus->bright_prscrs.pre_score_14;
		p_class->bright_flags.f14 =1;
	}


	//pixel 15
	if (p_class_bus->dark_prscrs.pre_score_15 <= 0) {
		p_class->dark_prscrs.pre_score_15 = 0;
		p_class->dark_flags.f15= 0;
	}else{
		p_class->dark_prscrs.pre_score_15 = p_class_bus->dark_prscrs.pre_score_15;
		p_class->dark_flags.f15=1;
	}

	if (p_class_bus->bright_prscrs.pre_score_15 <= 0) {
		p_class->bright_prscrs.pre_score_15 = 0;
		p_class->bright_flags.f15 = 0;
	}else{
		p_class->bright_prscrs.pre_score_15 = p_class_bus->bright_prscrs.pre_score_15;
		p_class->bright_flags.f15 =1;
	}



	p_class->center = p_class_bus->center;
}

static
void contiguity(uint8_t center, flags_t *dark_contig_bus,flags_t *bright_contig_bus, uint1 *p_is_darker, uint1 *p_is_brighter,uint8_t *p_center){
#pragma HLS INLINE
	uint8_t i;
	uint1 is_darker, is_brighter;
	const uint32_t mask = ((1<<9)-1);
	uint32_t rot_mask = ((1 << 9) - 1);
	uint32_t dark_bus_i,bright_bus_i;

	dark_bus_i = (dark_contig_bus->f0) |
				 (dark_contig_bus->f1 <<1) |
				 (dark_contig_bus->f2 <<2) |
				 (dark_contig_bus->f3 <<3) |
				 (dark_contig_bus->f4 <<4) |
				 (dark_contig_bus->f5 <<5) |
				 (dark_contig_bus->f6 <<6) |
				 (dark_contig_bus->f7 <<7) |
				 (dark_contig_bus->f8 <<8) |
				 (dark_contig_bus->f9 <<9) |
				 (dark_contig_bus->f10 <<10) |
				 (dark_contig_bus->f11 <<11) |
				 (dark_contig_bus->f12 <<12) |
				 (dark_contig_bus->f13 <<13) |
				 (dark_contig_bus->f14 <<14) |
				 (dark_contig_bus->f15 <<15)|
				 (dark_contig_bus->f0 <<16) |
				 (dark_contig_bus->f1 <<17) |
				 (dark_contig_bus->f2 <<18) |
				 (dark_contig_bus->f3 <<19) |
				 (dark_contig_bus->f4 <<20) |
				 (dark_contig_bus->f5 <<21) |
				 (dark_contig_bus->f6 <<22) |
				 (dark_contig_bus->f7 <<23) |
				 (dark_contig_bus->f8 <<24);




	bright_bus_i = (bright_contig_bus->f0)|
				(bright_contig_bus->f1<<1)|
				(bright_contig_bus->f2<<2)|
				(bright_contig_bus->f3<<3)|
				(bright_contig_bus->f4<<4)|
				(bright_contig_bus->f5<<5)|
				(bright_contig_bus->f6<<6)|
				(bright_contig_bus->f7<<7)|
				(bright_contig_bus->f8<<8)|
				(bright_contig_bus->f9<<9)|
				(bright_contig_bus->f10<<10)|
				(bright_contig_bus->f11<<11)|
				(bright_contig_bus->f12<<12)|
				(bright_contig_bus->f13<<13)|
				(bright_contig_bus->f14<<14)|
				(bright_contig_bus->f15<<15)|
				(bright_contig_bus->f0<<16)|
				(bright_contig_bus->f1<<17)|
				(bright_contig_bus->f2<<18)|
				(bright_contig_bus->f3<<19)|
				(bright_contig_bus->f4<<20)|
				(bright_contig_bus->f5<<21)|
				(bright_contig_bus->f6<<22)|
				(bright_contig_bus->f7<<23)|
				(bright_contig_bus->f8<<24);


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

			*p_center = center;
}

static
void scoring(pre_scores16_t *p_array_dark_coefs, pre_scores16_t *p_array_bright_coefs, uint16_t *p_dark_score, uint16_t *p_bright_score){
#pragma HLS INLINE

	uint16_t temp_dark_score_0_0,temp_dark_score_0_1,temp_dark_score_0_2,temp_dark_score_0_3,temp_dark_score_0_4,temp_dark_score_0_5,temp_dark_score_0_6,temp_dark_score_0_7;
	uint16_t temp_dark_score_1_0,temp_dark_score_1_1,temp_dark_score_1_2,temp_dark_score_1_3;
	uint16_t temp_dark_score_2_0,temp_dark_score_2_1;

	uint16_t temp_bright_score_0_0,temp_bright_score_0_1,temp_bright_score_0_2,temp_bright_score_0_3,temp_bright_score_0_4,temp_bright_score_0_5,temp_bright_score_0_6,temp_bright_score_0_7;
	uint16_t temp_bright_score_1_0,temp_bright_score_1_1,temp_bright_score_1_2,temp_bright_score_1_3;
	uint16_t temp_bright_score_2_0,temp_bright_score_2_1;


	temp_dark_score_0_0 = p_array_dark_coefs->pre_score_0 + p_array_dark_coefs->pre_score_1;
	temp_dark_score_0_1 = p_array_dark_coefs->pre_score_2 + p_array_dark_coefs->pre_score_3;
	temp_dark_score_0_2 = p_array_dark_coefs->pre_score_4 + p_array_dark_coefs->pre_score_5;
	temp_dark_score_0_3 = p_array_dark_coefs->pre_score_6 + p_array_dark_coefs->pre_score_7;
	temp_dark_score_0_4 = p_array_dark_coefs->pre_score_8 + p_array_dark_coefs->pre_score_9;
	temp_dark_score_0_5 = p_array_dark_coefs->pre_score_10 + p_array_dark_coefs->pre_score_11;
	temp_dark_score_0_6 = p_array_dark_coefs->pre_score_12 + p_array_dark_coefs->pre_score_13;
	temp_dark_score_0_7 = p_array_dark_coefs->pre_score_14 + p_array_dark_coefs->pre_score_15;



	temp_bright_score_0_0 = p_array_bright_coefs->pre_score_0 + p_array_bright_coefs->pre_score_1;
	temp_bright_score_0_1 = p_array_bright_coefs->pre_score_2 + p_array_bright_coefs->pre_score_3;
	temp_bright_score_0_2 = p_array_bright_coefs->pre_score_4 + p_array_bright_coefs->pre_score_5;
	temp_bright_score_0_3 = p_array_bright_coefs->pre_score_6 + p_array_bright_coefs->pre_score_7;
	temp_bright_score_0_4 = p_array_bright_coefs->pre_score_8 + p_array_bright_coefs->pre_score_9;
	temp_bright_score_0_5 = p_array_bright_coefs->pre_score_10 + p_array_bright_coefs->pre_score_11;
	temp_bright_score_0_6 = p_array_bright_coefs->pre_score_12 + p_array_bright_coefs->pre_score_13;
	temp_bright_score_0_7 = p_array_bright_coefs->pre_score_14 + p_array_bright_coefs->pre_score_15;

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
		uint8_t center_i,
		uint16_t dark_score,
		uint16_t bright_score,
		uint1 is_darker, uint1 is_brighter,
		uint1 *is_keypoint, uint16_t *score,
		uint8_t *p_center){

#pragma HLS INLINE

	if(is_darker == 1)
		*score = dark_score;
	else if (is_brighter == 1)
		*score = bright_score;
	else
		*score = 0;
	*is_keypoint = is_darker|is_brighter;

	*p_center = center_i;

}

void delay_kp(uint16_t dscore, uint16_t bscore, uint8_t center, uint1 is_brighter, uint1 is_darker,
		uint16_t *p_dscore,uint16_t *p_bscore, uint8_t *p_center, uint1 *p_is_brighter, uint1 *p_is_darker){
#pragma HLS INLINE


	static uint16_t dscore_i;
	static uint16_t bscore_i;
	static uint8_t center_i;
	static uint1 is_brighter_i;
	static uint1 is_darker_i;

	*p_dscore = dscore_i;
	*p_bscore = bscore_i;
	*p_center=center_i;
	*p_is_brighter = is_brighter_i;
	*p_is_darker = is_darker_i;

	dscore_i=dscore;
	bscore_i=bscore;
	center_i=center;
	is_brighter_i=is_brighter;
	is_darker_i=is_darker;
}

void fast9_16_double(uint8_t pixel1[IMG_HEIGH*IMG_WIDTH],uint8_t pixel0[IMG_HEIGH*IMG_WIDTH],
		uint8_t threshold, uint1 * is_keypoint1, uint1 *is_keypoint0,
		uint16_t score1[IMG_HEIGH*IMG_WIDTH],uint16_t score0[IMG_HEIGH*IMG_WIDTH],
		uint1* scores_valid,
		uint8_t *center0, uint8_t *center1,
		uint16_t* x_coord1, uint16_t* x_coord0, uint16_t* y_coord0,uint16_t* y_coord1){

	#pragma HLS INTERFACE ap_fifo port=pixel0
	#pragma HLS INTERFACE ap_fifo port=pixel1
	#pragma HLS INTERFACE ap_fifo port=score0
	#pragma HLS INTERFACE ap_fifo port=score1

				cache_colmn_t clmn1_i,clmn0_i;
				bhrm_circle_t bc1, bc0;

				classifilter_data_t cls_data1,cls_data0;
				classifilter_data_t fltr_data1,fltr_data0;

				uint1 is_darker1, is_brighter1,is_darker0, is_brighter0;
				uint16_t dark_score1, bright_score1;
				uint16_t dark_score0, bright_score0;
				uint16_t dark_score0_d1, bright_score0_d1;
				uint1 is_darker0_d1,is_brighter0_d1;
				uint16_t i,k_y,k_x,coord_x0;
				uint8_t center0_i,center0_d1_i;
				uint8_t center1_i;

				 for( i=0,k_y=0; k_y < IMG_HEIGH; k_y++){
				  for(k_x=0,coord_x0=0; k_x < IMG_WIDTH/2; k_x++, i++,coord_x0+=2 ){
				  #pragma HLS PIPELINE rewind
					  compute_coordinates_double(coord_x0,x_coord1,x_coord0,y_coord1,y_coord0,scores_valid);

					  cache_img_lines_double(pixel1[i],pixel0[i],k_x,&clmn1_i,&clmn0_i);
					   pixel_matrix_double(&clmn1_i, &clmn0_i, &bc1,&bc0);

						classifier(&bc0, threshold, &cls_data0);
 	 	 	 	 	    classifier(&bc1, threshold, &cls_data1);

						   filter(&cls_data0, &fltr_data0);
						   filter(&cls_data1, &fltr_data1);

					   	   scoring(&(fltr_data0.dark_prscrs),&(fltr_data0.bright_prscrs),&dark_score0,&bright_score0);
					   	   scoring(&(fltr_data1.dark_prscrs),&(fltr_data1.bright_prscrs),&dark_score1,&bright_score1);

					  		contiguity(fltr_data0.center,&(fltr_data0.dark_flags), &(fltr_data0.bright_flags), &is_darker0, &is_brighter0,&center0_i);
					  		contiguity(fltr_data1.center,&(fltr_data1.dark_flags), &(fltr_data1.bright_flags), &is_darker1, &is_brighter1,&center1_i);

					  		delay_kp(dark_score0, bright_score0,center0_i, is_brighter0, is_darker0,
					  				&dark_score0_d1, &bright_score0_d1,	&center0_d1_i, &is_brighter0_d1, &is_darker0_d1);

					  	      score_selection(center0_d1_i,dark_score0_d1, bright_score0_d1,is_darker0_d1 , is_brighter0_d1, is_keypoint0, &(score0[i]),center0);

					   		  score_selection(center1_i,dark_score1, bright_score1, is_darker1, is_brighter1, is_keypoint1, &(score1[i]),center1);


				  }}
}
