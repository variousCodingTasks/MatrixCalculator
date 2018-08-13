#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mat.h"

#define DEFAULT_SIZE 4
#define MAX_BUFFER_SIZE 100
#define MAX_LINE_SIZE 2048
#define MATRIX_COUNT 6
#define FUNCTIONS_COUNT 8

/*
 * func:
 * a structure which contains some function meta data,
 * like its name (represented by a string), how many
 * of each type of input it takes, and a pointer to it.
 * this is used only in this source file, so it's not included
 * in the header "mat.h".
 */
typedef struct func {
        char *name;
        int mat_input;
        unsigned int takes_scalar : 1;
        unsigned int has_output : 1;
        unsigned int reads_floats : 1;
        int parameters_count;
        void (*func)(parameters*, int);
    } func;

/*
 * functions_list:
 * an array containing "func" structures, relating to some of the
 * functions in the "mat.c" file.
 */
const func functions_list[] = {
                            {"read_mat", 0, 0, 1, 1, 2, NULL},
                            {"print_mat", 1, 0, 0, 0, 1, print_matrix},
                            {"add_mat", 2, 0, 1, 0, 3, add_matrix},
                            {"sub_mat", 2, 0, 1, 0, 3, sub_matrix},
                            {"mul_mat", 2, 0, 1, 0, 3, mul_matrix},
                            {"mul_scalar", 1, 1, 1, 0, 3, mul_scalar},
                            {"trans_mat", 1, 0, 1, 0, 2, trans_matrix},
                            {"stop", 0, 0, 0, 0, 0, NULL}};

parameters pack_parameters(int, float, float*, int*, mat*);
int is_legal_mat_char(int);
int read_next_mat_string(char*);
int read_float(float*);
int skip_whites(void);
void skip_line(void);
int peek_next_char(void);
int select_function(char*);
void read_command(char*);
void read_mat_parameter_error_check(int, int, char*, int, int, int*);
int read_mat_parameter(mat*, int, int*);
void read_scalar_parameter_error_check(int, int, int*);
void read_scalar_parameter(float*, int*);
void read_mat_elements_error_check(int, int, int, int);
void read_mat_elements(float*);
void stop(int*);
int check_comma_error(void);
int read_parameters(int, int*, float*, float*, mat*);
void read_mat(int, float*, mat*);
void call_function(parameters*, int*);
int pre_process_line(int*);
void process_line(mat*, int*);
void mat_calculator(void);

/*
 * main function calls "mat_calculator", which calls the main processing
 * functions.
 */
int main(int argc, char** argv) {
    
    puts("This is the simple matrix calculator program.\n"
         "Please enter your input line by line, each line\n"
         "must be terminated with a line break. the marker\n"
         "\">>>\", marks a new line where you can enter your\n"
         "next input. Each line must start with a command followed\n"
         "by any number of spaces or tabs, then by the desired parameters\n"
         "separated by commas (and any number of spaces or tabs).\n"
         "When you're done, please terminate the program by calling\n"
         "the \"stop\" command.");
    
    mat_calculator();

    return (EXIT_SUCCESS);
}

/*
 * pack_parameters:
 * this function takes the various data structures read from the input,
 * and packs them into a parameter structure, to be passed to the calculating
 * functions selected by the user. 
 */
parameters pack_parameters(int func_selection, float scalar_input, float *elements,
                            int *mat_selection, mat *matrices){
    parameters result;
    result.func_selection = func_selection;
    result.scalar_input = scalar_input;
    result.elements = elements;
    result.mat_selection = mat_selection;
    result.matrices = matrices;
    return result;
}
/*
 * is_legal_mat_char:
 * takes a char and decides if it's a legal matrix char, matrix names
 * consist of upper case letters and under scores only.
 */
int is_legal_mat_char(int c){
    if (('A' <= c && 'Z' >= c) || c == '_')
        return 1;
    else
        return 0;
}

/*
 * read_next_mat_string:
 * takes a char array in which it saves the matrix name read from stdin,
 * then adds a '\0', returns the last non upper case nor under score
 * character to the caller, also returns it to the buffer, so it can
 * be processed by the next function.
 */
int read_next_mat_string(char *string){
    int c, i;
    for(i= 0; is_legal_mat_char((c = getc(stdin))); ){
        string[i++] = c;
    }
    string[i] = '\0';
    ungetc(c, stdin);
    return c;
}

/*
 * read_float:
 * reads a float number and saves it in "result", returns the
 * number of digits it read: this is useful in case only - or .
 * were supplied with no digits, which could have been counted as 0,
 * but actually is an illegal character when no digits are present in
 * the right places. this function is used instead of "atof" from the
 * standard library.
 */
int read_float(float *result){
    float val, power = 1.0;
    int i, sign = 1, c = getc(stdin), digits_count = 0;
    if (c == '-'){
        sign = -1;
        c = getc(stdin);
    }
    while(isdigit(c)){
        val = 10.0 * val + (c - '0');
        c = getc(stdin);
        digits_count++;
    }
    if (c == '.')
        for (power = 1.0; isdigit((c = getc(stdin))); i++, digits_count++) {
            val = 10.0 * val + (c - '0');
            power *= 10.0;
        }
    ungetc(c, stdin);
    *result = sign * val / power;
    return digits_count;
}

/*
 * skip_whites:
 * skips spaces and tabs, returns the first non space nor tab char
 * it reads to the caller and to the stdin.
 */
int skip_whites(void){
    int c;
    while((c = getc(stdin)) == '\t' || c == ' ')
        ;
    ungetc(c, stdin);
    return c;
}

/*
 * skip_line:
 * it skips and consumes all the characters until a line break is
 * detected (which is also consumed), EOF is not used here since the 
 * function "pre_process_line" makes sure that the program is stopped
 * before processing a line terminated with EOF.
 */
void skip_line(void){
    int c;
    while((c = getc(stdin)) != '\n' && c != EOF)
        ;
    if (c == EOF)
        ungetc(c, stdin);
}

/*
 * peek_next_char:
 * skips spaces and tabs, then reads the next char, returns it to the
 * user and the buffer (stdin).
 */
int peek_next_char(void){
    int c;
    skip_whites();
    c = getc(stdin);
    ungetc(c, stdin);
    return c;
}

/*
 * select_function:
 * finds the index of the supplied function string in the "functions_list"
 * array and returns it to the caller, if not found, the index returned is
 * larger than the maximum index in the array.
 */ 
int select_function(char *command){
    int i;
    for(i = 0; i < FUNCTIONS_COUNT; i++){
        if (!strcmp(command, functions_list[i].name))
            break;
    }
    return i;
}

/*
 * read_command:
 * read the first string in a line, saves it in the command pointer.
 * since it uses "fscanf", it skips all the whites, including line breaks, until it reads 
 * a string.
 */
void read_command(char *command){
    skip_whites();
    fscanf(stdin,"%s",command);
    skip_whites();
}

/*
 * read_mat_parameter_error_check:
 * takes some parameters set by "read_mat_parameter", which is the caller
 * of this function, and determines the error. it sets status to 0,
 * to prevent "read_parameters" from reading any more parameters and
 * skips the line after determining the error type, which can be only
 * one. the order of the errors is set in a way which prints the first
 * relevant error message and doesn't report any additional errors.
 */
void read_mat_parameter_error_check(int index, int p_count, char *mat_name, int parameter_length, int next_char, int *status){
    int c = peek_next_char();
    *status = 0;
    if (p_count == 1 && c != '\n' && index < 6)
        printf("Error: extraneous text at end of command\n");
    else if (p_count > 1 && is_legal_mat_char(c) && index < 6)
        printf("Error: missing comma\n");
    else if (c == '\n' && (parameter_length == 0 || (p_count > 1 && index < 6)))
        printf("Error: too few arguments\n");
    else if (parameter_length == 0 && c == ',')
        printf("Error: multiple consecutive commas\n");
    else if (!is_legal_mat_char(c) &&  index < 6)
        printf("Error: illegal char \"%c\" following matrix name\n", c);
    else if (index > 5 && (next_char == '\n' || next_char == ' ' || next_char == '\t' || next_char == ','))
        printf("Error: unknown matrix \"%s\"\n",mat_name);
    else if (!is_legal_mat_char(c))
        printf("Error: matrix name should only contain upper case letters and underscores\n");
    skip_line();
}

/*
 * read_mat_parameter:
 * tries to read the next "mat" parameter supplied by the user, read
 * from stdin, and determines if the name is a valid matrix name, and
 * if it's followed by the right character: line break, if it's the last parameter
 * to be read or a comma if there are still additional parameters to be
 * read. if the matrix name is not correct or any other illegal characters
 * present the error checking function is called with the calculated parameters.
 * the result (selection) is saved in "matrices" array, which contains three
 * places: 0 and 1 for the input matrices, and 2 for the output. this
 * is the maximum number of input and output matrices any function uses.
 * p_count is the number of remaining parameters to be read and status
 * is a flag parameter, 1 means that everything is OK, 0 otherwise.
 */
int read_mat_parameter(mat *matrices, int p_count, int *status){
    int i, next_char;
    char mat_name[MAX_BUFFER_SIZE];
    next_char = read_next_mat_string(mat_name);
    for(i = 0; i < MATRIX_COUNT; i++){
        if (!strcmp(mat_name, matrices[i].name))
            break;
    }
    skip_whites();
    if (p_count == 1 && i < 6 && peek_next_char() == '\n')
        skip_line();
    else if (p_count > 1 && i < 6 && peek_next_char() == ','){
        getc(stdin);
        skip_whites();
    }
    else
        read_mat_parameter_error_check(i, p_count, mat_name, strlen(mat_name), next_char, status);
    return i;
}

/*
 * read_scalar_parameter_error_check:
 * checks for the cause of error detected by "read_scalar_parameter"
 * in the correct order, and reports it, also sets status to 0, to stop
 * parameter reading and skips line. the input parameters are calculated
 * by the scalar reading function, which is also the caller.
 */
void read_scalar_parameter_error_check(int digits_count, int c, int *status){
    int next_char;
    *status = 0;
    next_char = peek_next_char();
    if ((c == '.' || c == '-') && !digits_count)
        printf("Error: illegal char \'%c\'\n",c);
    else if (!digits_count && next_char == ',')
        printf("Error: multiple consecutive commas\n");
    else if (next_char == '\n')
        printf("Error: too few arguments\n");
    else if (digits_count)
        printf("Error: illegal char \'%c\' following scalar\n",c);
    else
        printf("Error: illegal char \'%c\'\n",c);
    skip_line();
}

/*
 * read_scalar_parameter:
 * reads scalar parameter from stdin and stores it, in case everything
 * is OK, in result. only "mul_scalar" function uses this type of parameter,
 * so it must be followed by a comma, any other case which is not a floating
 * point number followed by a comma triggers the error checking function
 * to determine the source of the error.
 */
void read_scalar_parameter(float *result, int *status){
    int c, digits_count;
    float temp;
    c = peek_next_char();
    digits_count = read_float(&temp);
    if (digits_count && ((c = peek_next_char()) == ',')){
        getc(stdin);
        skip_whites();
        *result = temp;
    }
    else {
        read_scalar_parameter_error_check(digits_count, c, status);
    }
}

/*
 * read_mat_elements_error_check:
 * this is an error checking function, called by "read_mat_elements",
 * which also calculates the input for this function. it determines
 * the source of the error when reading matrix elements, in case "read_mat"
 * was selected by the user.
 */
void read_mat_elements_error_check(int c, int count, int prefix, int success){
    if (count < 16){
        if ((prefix == '.' || prefix == '-') && success == 0)
            printf("Error: illegal char \'%c\', only %d elements read\n", prefix, count);
        else if (isdigit(c) || c == '.' || c== '-')
            printf("Error: matrix elements should be comma separated, only %d elements were read\n", count);
        else if (c == '\n')
            printf("Warning: too few elements, only %d elements read\n", count);
        else if (c == ',')
            printf("Warning: multiple consecutive commas, only %d elements read\n", count);
        else
            printf("Error: illegal char \'%c\', only %d elements read\n", c, count);
    }
}

/*
 * read_mat_elements:
 * this function reads the floating pont numbers supplied by the user
 * from stdin, up to the point it detects an error or reads the maximum
 * number of elements allowed. if more input is present, the function
 * ignores it and skips to the next line.  if any error is detected before
 * the max number of elements is read, the values are stored in the
 * matrix selected by the user anyway and the error checking function
 * is called.
 */
void read_mat_elements(float *elements){
    int i = 0, prefix, digits_count;
    float temp;
    prefix = peek_next_char();
    while(i < DEFAULT_SIZE * DEFAULT_SIZE && (digits_count = read_float(&temp))){
        elements[i++] = temp;
        if (peek_next_char() == ','){
            getc(stdin);
            prefix = peek_next_char();
        }
        else{
            break;
        }
    }
    read_mat_elements_error_check(peek_next_char(), i, prefix,digits_count);
    skip_line();
}

/*
 * stop:
 * sets the supplied stop flag to 0.
 */
void stop(int *stop_flag){
    *stop_flag = 1;
}

/*
 * check_comma_error:
 * checks if a command is followed by a comma character and stops
 * the line processing and skips to the next line.
 */
int check_comma_error(void){
    int c, status = 1;
    skip_whites();
    c = peek_next_char();
    if (c == ','){
        status = 0;
        skip_line();
        printf("Error: invalid comma after command, skipping line\n");
    }
    return status;
}

/*
 * read_parameters:
 * takes different data structures and calls the parameter reading functions
 * which stores their readings in the input parameters (passed to them),
 * if any error is detected by any o the parameter reading functions,
 * the process stops, also, this function keeps track of the number 
 * of parameters to be read, using "p_notify",to notify the functions it calls if the
 * parameter to be read is the last (for error checking purposes). the function calls
 * are performed in the proper order. the function uses the meta data
 * from the function list to determine how many and what type of parameters
 * need to be processed.
 */
int read_parameters(int selection, int *mat_selection, float *scalar_input, float *elements, mat *matrices){
    int status = check_comma_error();
    int p_count = functions_list[selection].parameters_count;
    if (status){
        if (functions_list[selection].mat_input)
            mat_selection[0] = read_mat_parameter(matrices, p_count--, &status);
        if (functions_list[selection].mat_input == 2 && status)
            mat_selection[1] = read_mat_parameter(matrices, p_count--, &status);
        if (functions_list[selection].takes_scalar && status){
            read_scalar_parameter(scalar_input, &status);
            p_count--;
        }
        if (functions_list[selection].has_output && status){
            mat_selection[2] = read_mat_parameter(matrices, p_count--, &status);
        }
        if (functions_list[selection].reads_floats && status)
            read_mat_elements(elements);
    }
    return status;
}

/*
 * read_mat:
 * takes the mat selection as input, where it stores the floats from
 * the elements array supplied and saves them into the "matrix" that belongs
 * to the relevant "mat" selected by the user.
 */
void read_mat(int mat_selected, float *elements, mat *matrices){
    int i, j, k = 0;
    matrix dest_mat = matrices[mat_selected].data;
    for (i = 0; i < DEFAULT_SIZE; i++){
        for (j = 0; j < DEFAULT_SIZE; j++){
            dest_mat[i][j] = elements[k];
            k++;
        }
    }
}

/*
 * call_function:
 * calls the selected function with the parameters structure, using
 * the pointer stored in the "functions_list" array.
 */
void call_function(parameters *params, int *stop_flag){
    switch(params->func_selection){
        case 0:
            read_mat((params->mat_selection)[2], params->elements, params->matrices);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            (functions_list[params->func_selection].func)(params, DEFAULT_SIZE);
            break;
        case 7:
            stop(stop_flag);
            break;
    }
}

/*
 * pre_process_line:
 * stop flag is set 1 if an error is detected and the status is 1 when
 * everything is fine, 0 otherwise. this functions reads the whole line
 * up to the defined buffer size: if the the number of characters is less
 * than the max and the line is terminated with a line break, then its printed
 * to stdout and is unread back to the buffer, so process line can perform
 * its work, other wise (in case buffer is maxed or EOF is detected),
 * it stops the program. the printing part and EOF detection is better
 * done here, otherwise, it could cause the code to be less readable
 * or more complicated, so I'd rather its done here.
 */
int pre_process_line(int *stop_flag){
    char line[MAX_LINE_SIZE];
    int c, j, i = 0;
    while(i < MAX_LINE_SIZE - 1 && (c = getc(stdin)) != '\n' && c != EOF)
                line[i++] = c;
    line[i++] = '\0';
    puts(line);
    if (c == '\n'){
        ungetc('\n', stdin);
        for(j = i - 2; 0 <= j; j--)
            ungetc(line[j], stdin);
    }
    else {
        *stop_flag = 1;
        if (c == EOF)
            puts("Error: End of File character detected, terminating...");
        else
            printf("Error: line should be up to %d chars, terminating...\n", MAX_LINE_SIZE);
    }
    return *stop_flag ? 0 : 1;
}

/*
 * process_line:
 * takes the matrices array, defines several data structures to hold
 * the reading functions output and allocates (later frees) the memory
 * needed for their operation, reads the command, if no errors,
 * calls "read_parameters" (which returns its status), if no errors,
 * calls the function "call_function", to call the selected function
 * using the read parameters as input.
 */
void process_line(mat *matrices, int *stop_flag){
    float scalar_input, *elements;
    int func_selection, mat_selection[3];
    char command[MAX_BUFFER_SIZE];
    parameters params;
    elements = calloc(DEFAULT_SIZE * DEFAULT_SIZE, sizeof(float));
    read_command(command);
    func_selection = select_function(command);
    if (func_selection >= FUNCTIONS_COUNT){
         printf("Error: unknown command \"%s\"\n",command);
         skip_line();
    }
    else if (read_parameters(func_selection, mat_selection, &scalar_input, elements, matrices)){
        params = pack_parameters(func_selection, scalar_input, elements, mat_selection, matrices);
        call_function(&params ,stop_flag);
    }
    free(elements);
}

/*
 * mat_calculator:
 * creates 6 matrices, places them in an array, initializes them,
 * then processes each line: ">>>" marks the beginning of a new line,
 * each iteration the line is pre-processed, if everything goes well,
 * the line is processed. when something is "wrong" detected by
 * any function called down the way, the flag is set to 1, and the loop
 * terminates, stopping the program, after freeing the allocated memory.
 */
void mat_calculator(void){
    int i, stop_flag = 0;
    mat matrices[] = { {"MAT_A", NULL}, {"MAT_B", NULL}, {"MAT_C", NULL},
                        {"MAT_D", NULL}, {"MAT_E", NULL}, {"MAT_F", NULL}};
    for(i = 0; i < MATRIX_COUNT; i++){
        matrices[i].data = create_matrix(DEFAULT_SIZE);
    }
    while(!stop_flag){
        printf(">>> ");
        if (pre_process_line(&stop_flag))
            process_line(matrices, &stop_flag);
    }
    for(i = 0; i < MATRIX_COUNT; i++){
        free_matrix(matrices[i].data, DEFAULT_SIZE);
    }
}
