#ifndef INTERP
#define INTERP


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "../neillsimplescreen.h"

#define PI 3.14159
#define STACKSIZE 1000
#define INITIALPS_ANGLE 180
#define MAX_LINE_LENGTH 100
#define MAXNUMTOKENS 1000
#define MAXTOKENSIZE 50
#define COMMAND_LEN 200
#define RESWIDTH 51
#define RESHEIGHT 33
#define PSWIDTH 30
#define PSHEIGHT 40
#define TERMINALHEIGHT 33
#define TERMINALWIDTH 51
#define TERMINALANGLE 180
#define strsame(A,B) (strcmp(A, B)==0)
#define NUM_VARS 27
#define INITIAL_ANGLE 0
#define tc_enter_alt_screen() puts("\033[?1049h\033[H")
#define tc_exit_alt_screen() puts("\033[?1049l")


typedef enum FILETYPE{
    TEXT_FILE,
    NO_FILE,
    POSTSCRIPT_FILE
} FILETYPE;

typedef enum VARTYPE{
    STRING,
    DOUBLE
} VARTYPE;

typedef struct VAR{
    VARTYPE vartype;
    char strval[MAXTOKENSIZE];
    double numval;
} VAR;

typedef struct LOOPLIST{
    VAR list[MAXNUMTOKENS];
    int size;
    int curr_index;
} LOOPLIST;

typedef struct coll {
   // Underlying array
   VAR a[STACKSIZE];
   int size;
} coll;

typedef struct prog{
   char words[MAXNUMTOKENS][MAXTOKENSIZE];
   int curword; // Current Word
   VAR variables[NUM_VARS];
   bool is_var_used[NUM_VARS];
   coll* stack;
} Program;

typedef struct Turtle{
    char matrix[RESHEIGHT][RESWIDTH];
    double row, col;
    double maxrow, maxcol;
    FILE* file;
    double angle;
    FILETYPE filetype;
    neillcol colour;
} Turtle;

bool interp_file(FILE* file, Turtle* res);
void processFile(bool is_valid, int argc, char** argv, Turtle* res);
bool isnumber(char* str);
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
bool check_varnum(Program* prog, VAR* num);
bool check_word(Program* prog);
bool check_var(Program* prog, VAR* num);
bool check_pfix(Program* prog, Turtle* res, VAR* val);
bool check_ltr(Program* prog, int index);
bool check_lst(Program* prog, Turtle* res, LOOPLIST* loop_lst);
bool check_num(Program* prog, VAR* num);
bool check_op(Program* prog, char* op);
bool check_items(Program* prog, Turtle* res, LOOPLIST* loop_lst);
bool check_item(Program* prog, LOOPLIST* loop_lst);
void get_file_extension(char* file_name, char* extension);
bool print_to_file(Turtle* res, double num);
void free_prog(Program* prog);

void process_rgt(Turtle* res, int angle);
char convert_colour_to_char(neillcol colour);
void write_to_file(Turtle* res, char* file_name);
bool fetch_colour(char* colour, neillcol* val);
void process_colour(Turtle* res, neillcol colour);
char str_to_var(char* str);
void set_var(Program* prog, char var_name, VAR* val);
bool fetch_colour_var(VAR* var, neillcol* val);
bool update_stack(coll* stack, char op);
void add_to_looplist(LOOPLIST* looplst, VAR d);
void write_to_ps(Turtle* res, double x1, double x2, double y1, double y2);
void getrgbcolor(neillcol colour, double* r, double* g, double* b);
void get_command(char* command, char* input_file);
void free_turtle(Turtle* res);
void initializescreen(Turtle* res);
void print_to_screen(int x, int y, char colour);
bool is_validcoord(int r, int c, int m, int n);

// STACK FUNCTIONS
coll* coll_init(void);
int coll_size(coll* c);
void coll_add(coll* c, VAR  d);
bool coll_free(coll* c);
bool coll_pop(coll* c, VAR* res);
Program* get_program(FILE* file);

// TESTING FUNCTIONS

void test(void);
void test_check_ltr(void);
void test_check_var(void);
void test_check_num(void);
void test_check_word(void);
void test_check_op(void);
void test_check_varnum(void);
void test_fwd(void);
void test_rgt(void);
void test_col(void);
void test_set(void);
void test_item(void);
void test_items(void);
void test_lst(void);
void test_loop(void);
void test_ins(void);
void test_inslst(void);
void test_process_rgt(void);
void test_convert_colour_to_char(void);
void test_fetch_colour(void);
void test_process_colour(void);
void test_str_to_var(void);
void test_is_validcoord(void);
void test_getrgbcolour(void);
void test_update_stack(void);
void test_add_to_looplist(void);

#endif

