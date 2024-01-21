#ifndef EXTENSION_H
#define EXTENSION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "../neillsimplescreen.h"

#define PI 3.14159
#define STACKSIZE 1000
#define INITIALPS_ANGLE 0
#define MAX_LINE_LENGTH 100
#define MAXNUMTOKENS 1000
#define MAXTOKENSIZE 50
#define COMMAND_LEN 200
#define NUMDIRS 8
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

int dirr[NUMDIRS] = {1, 1, 0, -1, -1, -1, 0, 1};
int dirc[NUMDIRS] = {0, 1, 1, 1, 0, -1, -1, -1};

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

typedef struct Pair{
    int index;
    int size;
    int arr[RESHEIGHT*RESWIDTH][2];
} Pair;

typedef struct Turtle{
    int row, col;
    char matrix[RESHEIGHT+5][RESWIDTH*2];
    bool visited[RESHEIGHT][RESWIDTH];
    int maxrow, maxcol;
    FILE* op;
    int angle;
    int numcells;
    neillcol colour;
} Turtle;

void processFile(char* ipFileName, char* opFileName);
Turtle* initTurtle(FILE* op);
void convertToMatrix(Turtle* turtle, FILE* ip);
void strip_new_line(char* str);
void printInvalidMessage(char* str);
void startConversion(Turtle* turtle);
void freeTurtle(Turtle* turtle);
int countAlpha(char* str, Turtle* turtle);
void dfs(Turtle* turtle, int initRow, int initCol, int numCells, Pair* pairList);
static inline bool isValid(int row, int col);
void processMoves(Turtle* turtle, Pair* pairList);
void writeMoves(int prevR, int prevC, int nextR, int nextC, Turtle* turtle);
int getAngle(int prevR, int prevC, int nextR, int nextC);
void checkColour(int prevR, int prevC, int nextR, int nextC, Turtle* turtle);
void getColour(char ch, char colour[10]);

// TESTING FUNCTIONS

void test(void);
void test_initTurtle(void);
void testCountAlpha(void);
void testIsValid(void);
void testGetAngle(void);
void testGetColour(void);
void testCheckColour(void);

#endif

