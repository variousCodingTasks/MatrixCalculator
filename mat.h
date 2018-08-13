#ifndef MAT_H
#define MAT_H

    typedef float **matrix;

    typedef struct mat {
        char *name;
        matrix data;
    } mat;
    
    /*
     * parameters:
     * func_selection: the index of the selected function
     * scalar_input: the floating point number supplied by the user
     * elements: array that stores the matrix elements
     * mat_selection: an array which contains the the indexes
     * of the selected matrices: 0 and 1 holds the indexes
     * of the input matrices and 2 holds the index of the desired
     * output matrix.
     * matrices: the array of 6 matrices, created when the program
     * is initialized.
     */
    typedef struct parameters {
        int func_selection;
        float scalar_input;
        float *elements;
        int *mat_selection;
        mat *matrices;
    } parameters;
    
    matrix create_matrix(int);
    void free_matrix(matrix, int);
    void print_matrix(parameters* , int);
    void mul_matrix(parameters*, int);
    void add_matrix(parameters*, int);
    void sub_matrix(parameters*, int);
    void mul_scalar(parameters*, int);
    void trans_matrix(parameters*, int);

#endif