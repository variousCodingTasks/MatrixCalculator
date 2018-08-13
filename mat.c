#include <stdio.h>
#include <stdlib.h>
#include "mat.h"

/*
 * create_matrix:
 * creates a square matrix of size N, returns a pointer
 * to pointer of float. "calloc" is used, to make sure all
 * elements are initialized to zeros.
 */
matrix create_matrix(int N){
    int i;
    matrix array;
    array = (float**)malloc(N * sizeof(float*));
    for(i = 0; i < N; i++){
        array[i] = (float*)calloc(N, sizeof(float));
    }
    return array;
}

/*
 * free_matrix:
 * takes a pointer to a pointer of floats, frees the allocated
 * memory of each pointer to float, then frees the memory allocated
 * to the pointer to pointer of float.
 */
void free_matrix(matrix xx, int N){
    int i;
    for(i =0; i < N; i++){
        free(*(xx + i));
    }
    free(xx);
}

/*
 * matrix_data:
 * takes a parameters structure as input and an index of the
 * selected matrix (of type "mat"), and returns its data member,
 * which is a pointer to pointer of float (typedef matrix).
 */
static float **matrix_data(parameters *params, int index){
    return (params->matrices)[(params->mat_selection)[index]].data;
}

/*
 * print_matrix:
 * takes a parameters structure and size, and prints the members
 * of the mat selected by the user (the input mat).
 */
void print_matrix(parameters *params, int N){
    int i, j;
    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            printf("%-9.2f\t", ((params->matrices)[(params->mat_selection)[0]].data)[i][j]);
        }
        puts("");
    }
}

/*
 * mult_row_column:
 * an auxiliary function
 * takes 2d arrays, a row index, a column index and size, returns
 * the sum product, this is the i,j element in the product of the
 * matrices.
 */
static float mult_row_column(matrix xx, matrix yy, int i, int j, int size){
    int k;
    float result = 0;    
    for(k = 0; k < size; k++){
        result += xx[i][k] * yy[k][j];
    }
    return result;
}

/*
 * mul_matrix:
 * takes a "parameters" structure and size, creates a new matrix
 * to save the result, accesses the matrix parts of the user selected
 * matrices, multiplies the matrices, saves the result in temp,
 * then frees whatever matrix is in the output matrix, selected by the
 * user, then replaces it with the temp matrix.
 */
void mul_matrix(parameters *params, int size){
    int i, j;
    matrix temp_matrix = create_matrix(size);
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            temp_matrix[i][j] = mult_row_column(matrix_data(params, 0), matrix_data(params, 1), i, j, size);
        }
    }
    free_matrix(matrix_data(params, 2), size);
    (params->matrices)[(params->mat_selection)[2]].data = temp_matrix;
}

/*
 * add_matrix:
 * works like "mul_matrix", performs simple matrix addition.
 */
void add_matrix(parameters *params, int size){
    int i, j;
    matrix temp_matrix = create_matrix(size);
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            temp_matrix[i][j] = (matrix_data(params, 0))[i][j] + (matrix_data(params, 1))[i][j];
        }
    }
    free_matrix(matrix_data(params, 2), size);
    (params->matrices)[(params->mat_selection)[2]].data = temp_matrix;
}

/*
 * sub_matrix:
 * works like "mul_matrix", performs simple matrix subtraction.
 */
void sub_matrix(parameters *params, int size){
    int i, j;
    matrix temp_matrix = create_matrix(size);
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            temp_matrix[i][j] = (matrix_data(params, 0))[i][j] - (matrix_data(params, 1))[i][j];
        }
    }
    free_matrix(matrix_data(params, 2), size);
    (params->matrices)[(params->mat_selection)[2]].data = temp_matrix;
}

/*
 * sub_matrix:
 * works like "mul_matrix", performs matrix multiplication,
 * the scalar is supplied by the user.
 */
void mul_scalar(parameters *params, int size){
    int i, j;
    matrix temp_matrix = create_matrix(size);
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            temp_matrix[i][j] = params->scalar_input * (matrix_data(params, 0))[i][j];
        }
    }
    free_matrix(matrix_data(params, 2), size);
    (params->matrices)[(params->mat_selection)[2]].data = temp_matrix;
}

/*
 * sub_matrix:
 * works like "mul_matrix", transposes selected input matrix
 * and saves the result in the selected output matrix.
 */
void trans_matrix(parameters *params, int size){
    int i, j;
    matrix temp_matrix = create_matrix(size);
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            temp_matrix[i][j] = (matrix_data(params, 0))[j][i];
        }
    }
    free_matrix(matrix_data(params, 2), size);
    (params->matrices)[(params->mat_selection)[2]].data = temp_matrix;
}