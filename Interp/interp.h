#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 100
#define MAXNUMTOKENS 1000
#define MAXTOKENSIZE 50
#define RESWIDTH 51
#define RESHEIGHT 33
#define strsame(A,B) (strcmp(A, B)==0)

typedef struct prog{
   char words[MAXNUMTOKENS][MAXTOKENSIZE];
   int curword; // Current Word
} Program;

typedef struct Turtle{
    char matrix[RESHEIGHT][RESWIDTH];
    int row, col;
}

bool interp_file(FILE* file, Turtle* res);
void strip_new_line(char* str);
void init_turtle(Turtle* res);
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
void copy_word_from_str(char* word, char* str);