#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define PI 3.14159
#define MAX_LINE_LENGTH 100
#define MAXNUMTOKENS 1000
#define MAXTOKENSIZE 50
#define RESWIDTH 51
#define RESHEIGHT 33
#define TERMINALHEIGHT 80
#define TERMINALWIDTH 100
#define strsame(A,B) (strcmp(A, B)==0)
#define NUM_VARS 26
#define INITIAL_ANGLE 180

typedef enum FILETYPE{
    TEXT_FILE,
    NO_FILE,
    POSTSCRIPT_FILE
} FILETYPE;

typedef enum COLOUR{
    WHITE,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN
} COLOUR;

typedef struct prog{
   char words[MAXNUMTOKENS][MAXTOKENSIZE];
   int curword; // Current Word
   int variable[NUM_VARS];
   bool is_var_used[NUM_VARS];
} Program;

typedef struct Turtle{
    char matrix[RESHEIGHT][RESWIDTH];
    double row, col;
    FILE* file;
    double angle;
    FILETYPE filetype;
    COLOUR colour;
} Turtle;

bool interp_file(FILE* file, Turtle* res);
void strip_new_line(char* str);
Turtle* init_turtle(char* file_name);
bool check_prog(Program* prog, Turtle* res);
bool check_inslst(Program* prog, Turtle* res);
bool check_ins(Program* prog, Turtle* res);
bool check_fwd(Program* prog, Turtle* res);
bool check_rgt(Program* prog, Turtle* res);
bool check_col(Program* prog, Turtle* res);
bool check_loop(Program* prog, Turtle* res);
bool check_set(Program* prog, Turtle* res);
bool check_varnum(Program* prog, Turtle* res);
bool check_word(Program* prog, Turtle* res);
bool check_var(Program* prog, Turtle* res);
bool check_pfix(Program* prog, Turtle* res);
bool check_ltr(Program* prog, int index, Turtle* res);
bool check_lst(Program* prog, Turtle* res);
bool check_num(Program* prog, Turtle* res);
bool check_op(Program* prog, Turtle* res);
bool check_items(Program* prog, Turtle* res);
bool check_item(Program* prog, Turtle* res);
void get_file_extension(char* file_name, char* extension);
void print_to_file(Program* prog, Turtle* res, int num);
int fetch_num(Program* prog, int step_pos, Turtle* res);
void process_rgt(Turtle* res, int angle);
char convert_colour_to_char(COLOUR colour);
void write_to_file(Turtle* res, char* file_name);