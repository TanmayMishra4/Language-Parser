#ifndef PARSER
#define PARSER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_LINE_LENGTH 100
#define MAXNUMTOKENS 1000
#define MAXTOKENSIZE 50
#define strsame(A,B) (strcmp(A, B)==0)

typedef struct prog{
   char words[MAXNUMTOKENS][MAXTOKENSIZE];
   int curword; // Current Word
} Program;

bool parse_file(FILE* file);
void strip_new_line(char* str);
bool check_prog(Program* prog);
bool check_inslst(Program* prog);
bool check_ins(Program* prog);
bool check_fwd(Program* prog);
bool check_rgt(Program* prog);
bool check_col(Program* prog);
bool check_loop(Program* prog);
bool check_set(Program* prog);
bool check_varnum(Program* prog);
bool check_word(Program* prog);
bool check_var(Program* prog);
bool check_pfix(Program* prog);
bool check_ltr(Program* prog, int index);
bool check_lst(Program* prog);
bool check_num(Program* prog);
bool check_op(Program* prog);
bool check_items(Program* prog);
bool check_item(Program* prog);
void copy_word_from_str(char* word, char* str);
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
void test_pfix(void);
void test_set(void);
void test_item(void);
void test_items(void);
void test_lst(void);
void test_loop(void);
void test_ins(void);
void test_inslst(void);
#endif

