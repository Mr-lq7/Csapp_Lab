/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "bovik1",              /* Team name */

    "Harry Q. Bovik1",     /* First member full name */
    "bovik@nowhere.edu1",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";


// need to optimize
void rotate(int dim, pixel *src, pixel *dst) 
{
    // 1.行列交换
    int i, j;
    // for (j = 0; j < dim; j++)
	// for (i = 0; i < dim; i++)
	//     dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

    int s;
    int t;
    //经测试, 分的块数为16的时候效果最好
    //2.矩阵分块 15.9
    int block = 16;
    for (i = 0; i < dim; i += block) {
	    for (j = 0; j < dim; j += block) {
            for (s = i; s < i + block; s++) {
                for (t = j; t < j + block; t++) {
                    dst[RIDX(dim-1-s, t, dim)] = src[RIDX(t, s, dim)];
                }
            }
        }
    }


    //3.矩阵分块+内层循环展开 15.3
    // int s;
    // int block = 16;
    // for (i = 0; i < dim; i += block) {
	//     for (j = 0; j < dim; j += block) {
    //         for (s = i; s < i + block; s++) {
    //             dst[RIDX(dim-1-j, s, dim)] = src[RIDX(s, j, dim)];
    //             dst[RIDX(dim-2-j, s, dim)] = src[RIDX(s, j+1, dim)];
    //             dst[RIDX(dim-3-j, s, dim)] = src[RIDX(s, j+2, dim)];
    //             dst[RIDX(dim-4-j, s, dim)] = src[RIDX(s, j+3, dim)];  
    //             dst[RIDX(dim-5-j, s, dim)] = src[RIDX(s, j+4, dim)];
    //             dst[RIDX(dim-6-j, s, dim)] = src[RIDX(s, j+5, dim)];  
    //             dst[RIDX(dim-7-j, s, dim)] = src[RIDX(s, j+6, dim)];
    //             dst[RIDX(dim-8-j, s, dim)] = src[RIDX(s, j+7, dim)];  
    //             dst[RIDX(dim-9-j, s, dim)] = src[RIDX(s, j+8, dim)];
    //             dst[RIDX(dim-10-j, s, dim)] = src[RIDX(s, j+9, dim)];  
    //             dst[RIDX(dim-11-j, s, dim)] = src[RIDX(s, j+10, dim)];
    //             dst[RIDX(dim-12-j, s, dim)] = src[RIDX(s, j+11, dim)];   
    //             dst[RIDX(dim-13-j, s, dim)] = src[RIDX(s, j+12, dim)];
    //             dst[RIDX(dim-14-j, s, dim)] = src[RIDX(s, j+13, dim)];
    //             dst[RIDX(dim-15-j, s, dim)] = src[RIDX(s, j+14, dim)];
    //             dst[RIDX(dim-16-j, s, dim)] = src[RIDX(s, j+15, dim)];  
                 
    //         }

    //     }
    // }

}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   
    /* ... Register additional test functions here */
}


/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(ii = max(i-1, 0); ii <= min(i+1, dim-1); ii++) 
	for(jj = max(j-1, 0); jj <= min(j+1, dim-1); jj++) 
	    accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth. 
 * IMPORTANT: This is the version you will be graded on
 */
char smooth_descr[] = "smooth: Current working version";

// 四个角
void set_corner(int cc, pixel *src, pixel *dst, int a1, int a2, int a3){
    dst[cc].blue = (src[cc].blue+src[a1].blue+src[a2].blue+src[a3].blue) >> 2;
    dst[cc].green = (src[cc].green+src[a1].green+src[a2].green+src[a3].green) >> 2;
    dst[cc].red = (src[cc].red+src[a1].red+src[a2].red+src[a3].red) >> 2;
}

void set_top(int dim, pixel *src, pixel *dst, int j){
    dst[j].blue = (src[j].blue+src[j+dim].blue+src[j-1].blue+src[j+1].blue+src[j+dim-1].blue+src[j+dim+1].blue)/6;
    dst[j].green = (src[j].green+src[j+dim].green+src[j-1].green+src[j+1].green+src[j+dim-1].green+src[j+dim+1].green)/6;
    dst[j].red = (src[j].red+src[j+dim].red+src[j-1].red+src[j+1].red+src[j+dim-1].red+src[j+dim+1].red)/6;
}

void set_bottom(int dim, pixel *src, pixel *dst, int j){
    dst[j].blue = (src[j].blue+src[j-dim].blue+src[j-1].blue+src[j+1].blue+src[j-dim-1].blue+src[j-dim+1].blue)/6;
    dst[j].green = (src[j].green+src[j-dim].green+src[j-1].green+src[j+1].green+src[j-dim-1].green+src[j-dim+1].green)/6;
    dst[j].red = (src[j].red+src[j-dim].red+src[j-1].red+src[j+1].red+src[j-dim-1].red+src[j-dim+1].red)/6;
}
void set_left(int dim, pixel *src, pixel *dst, int i){
    dst[i].blue = (src[i].blue+src[i-dim].blue+src[i-dim+1].blue+src[i+1].blue+src[i+dim].blue+src[i+dim+1].blue)/6;
    dst[i].green = (src[i].green+src[i-dim].green+src[i-dim+1].green+src[i+1].green+src[i+dim].green+src[i+dim+1].green)/6;
    dst[i].red = (src[i].red+src[i-dim].red+src[i-dim+1].red+src[i+1].red+src[i+dim].red+src[i+dim+1].red)/6;
}
void set_right(int dim, pixel *src, pixel *dst, int i){
    dst[i].blue = (src[i].blue+src[i-dim].blue+src[i-dim-1].blue+src[i-1].blue+src[i+dim].blue+src[i+dim-1].blue)/6;
    dst[i].green = (src[i].green+src[i-dim].green+src[i-dim-1].green+src[i-1].green+src[i+dim].green+src[i+dim-1].green)/6;
    dst[i].red = (src[i].red+src[i-dim].red+src[i-dim-1].red+src[i-1].red+src[i+dim].red+src[i+dim-1].red)/6;
}

void set_innner(int dim, pixel *src, pixel *dst, int k){
    dst[k].blue = (src[k].blue+src[k-1].blue+src[k+1].blue+src[k+dim-1].blue+src[k+dim].blue+src[k+dim+1].blue+src[k-dim-1].blue+src[k-dim].blue+src[k-dim+1].blue)/9;
    dst[k].green = (src[k].green+src[k-1].green+src[k+1].green+src[k+dim-1].green+src[k+dim].green+src[k+dim+1].green+src[k-dim-1].green+src[k-dim].green+src[k-dim+1].green)/9;
    dst[k].red = (src[k].red+src[k-1].red+src[k+1].red+src[k+dim-1].red+src[k+dim].red+src[k+dim+1].red+src[k-dim-1].red+src[k-dim].red+src[k-dim+1].red)/9;
}

// need to optimize
void smooth(int dim, pixel *src, pixel *dst) 
{

    //先对四个角进行赋值
    //左上
    set_corner(0, src, dst, 1, dim, dim+1); //[i][j] -> i*dim+j
    //右上
    set_corner(dim-1, src, dst, dim-2, dim+dim-1, dim+dim-2);
    //左下
    set_corner(RIDX(dim-1, 0, dim), src, dst, RIDX(dim-2, 0, dim), RIDX(dim-2, 1, dim), RIDX(dim-1, 1, dim));
    //右下
    set_corner(RIDX(dim-1, dim-1, dim), src, dst, RIDX(dim-1, dim-2, dim), RIDX(dim-2, dim-2, dim), RIDX(dim-2, dim-1, dim));

    int i, j;
    //四条边
    for(i = 1; i <= dim-2; i++){
        set_top(dim, src, dst, i);
        set_bottom(dim, src, dst, dim*dim-dim+i);
        set_left(dim, src, dst, i*dim);
        set_right(dim, src, dst, i*dim+dim-1);
    }
    //内部
    for(i = 1; i <= dim-2; i++){
        for(j = 1; j <= dim-2; j++){
            set_innner(dim, src, dst, i*dim+j);
        }
    }   
}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&smooth, smooth_descr);
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    /* ... Register additional test functions here */
}

