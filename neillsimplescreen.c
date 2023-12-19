/*
   Simple attempt to use colour, clear screen etc.
   Will only work on ANSI compliant terminals :
   https://en.wikipedia.org/wiki/ANSI_escape_code
   such as xterm, WSL etc.
*/

#include "neillsimplescreen.h"

/* Issue ANSI Codes to clear screen */
void neillclrscrn(void)
{
   printf("\033[2J");
}

/* Issue ANSI Codes to move cursor home */
void neillcursorhome(void)
{
   printf("\033[H");
}

/* Issue ANSI Codes change foreground colour */
void neillfgcol(const neillcol c)
{
   printf("\033[%dm", c);
}

/* Issue ANSI Codes to change background colour */
void neillbgcol(const neillcol c)
{
   neillfgcol(c+BACKGROUND);
}

/* Issue ANSI Codes to reset text/colour styles */
/* Equivalent to running the command 'reset' in the terminal */
void neillreset(void)
{
   printf("\033[0m");
}

void neillmoverowsdown(int n){
   printf("\033[%iA", n);
}

void neillmovecolsright(int n){
   printf("\033[%iC", n);
}

void neillmoverowsup(int n){
   printf("\033[%iA", n);
}

void neillmovecolsleft(int n){
   printf("\033[%iD", n);
}

void neillmovecursortopos(int r, int c){
   // FILE* fp = fopen("/dev/ttys003", "w");
   // fprintf(fp, "r = %d, c = %i\n", r, c);
   printf("\033[%d;%dH", r, c);
   // fclose(fp);
}


/* May the coding Gods forgive this code
   You really want to give time to other
   processes, rather than sit in a busy loop */
void neillbusywait(const double secs)
{
   clock_t t2;
   const clock_t t1 = clock();
   do{
      t2 = clock();
   }while((t2-t1) < (clock_t)((double)CLOCKS_PER_SEC*secs));
}
