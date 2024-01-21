#include "extension.h"

int main(int argc, char** argv){
    if(argc != 3){
        fprintf(stderr, "Wrong number of params: Correct usage - ./extension [input file] [output file]\n");
        exit(EXIT_FAILURE);
    }
    char* inputFileName = argv[1];
    char* outputFileName = argv[2];
    processFile(inputFileName, outputFileName);
    return 0;
}  

void processFile(char* ipFileName, char* opFileName){
    FILE* ip = fopen(ipFileName, "r");
    if(ip == NULL){
        fprintf(stderr, "Cannot open input file\n");
        exit(EXIT_FAILURE);
    }
    FILE* op = fopen(opFileName, "w");
    if(op == NULL){
        fprintf(stderr, "Cannot open output file\n");
        exit(EXIT_FAILURE);
    }
    Turtle* turtle = initTurtle(op);
    convertToMatrix(turtle, ip);
    fclose(ip);
    startConversion(turtle);
    freeTurtle(turtle);
}

Turtle* initTurtle(FILE* op){
    Turtle* res = (Turtle*)calloc(1, sizeof(Turtle));
    res->angle = INITIAL_ANGLE;
    res->colour = white;
    res->op = op;
    res->numcells = 0;
    res->row = RESHEIGHT/2;
    res->col = RESWIDTH/2;
    res->maxrow = RESHEIGHT;
    res->maxcol = RESWIDTH;
    for(int i=0;i<RESHEIGHT+5;i++){
        for(int j=0;j<RESWIDTH*2;j++){
            res->matrix[i][j] = ' ';
        }
    }

    return res;
}
void convertToMatrix(Turtle* turtle, FILE* ip){
    int res = 0;
    int i = 0;
    while(i <= RESHEIGHT && fgets(turtle->matrix[RESHEIGHT - i++], RESWIDTH*2, ip)){
        strip_new_line(turtle->matrix[RESHEIGHT - i + 1]);
        res += countAlpha(turtle->matrix[RESHEIGHT - i + 1], turtle);
    }
    turtle->numcells = res;
}

void strip_new_line(char* str){
    int len = strlen(str);
    int ind = len-1;
    while(ind >= 0 && str[ind] == '\n'){
        str[ind] = '\0';
        ind--;
    }
}

void freeTurtle(Turtle* turtle){
    if(turtle == NULL){
        return;
    }
    if(turtle->op != NULL){
        fprintf(turtle->op, "END\n");
        fclose(turtle->op);
        turtle->op = NULL;
    }
    free(turtle);
    turtle = NULL;
}

void printInvalidMessage(char* str){
    fprintf(stderr, "%s", str);
    exit(EXIT_FAILURE);
}

void startConversion(Turtle* turtle){
    int initialRow = RESHEIGHT - RESHEIGHT/2;
    int initialCol = RESWIDTH/2;
    printf("initial row = %i, col = %i\n", initialRow, initialCol);
    printMatrix(turtle);
    if(turtle->matrix[initialRow][initialCol] == ' '){
        char str[MAX_LINE_LENGTH];
        sprintf(str, "Initial starting point of row = %i, col = %i is empty\n", initialRow, initialCol);
        puts(str);
        freeTurtle(turtle);
        printInvalidMessage(str);
    }
    static Pair pairList;
    pairList.index = 0;
    pairList.size = 0;
    turtle->visited[initialRow][initialCol] = true;
    dfs(turtle, initialRow, initialCol, 1, &pairList);
}

int countAlpha(char* str, Turtle* turtle){
    int len = strlen(str);
    int res = 0;
    for(int i=0;i<len;i++){
        char ch = str[i];
        switch(ch){
            case 'W':
            case 'B':
            case 'K':
            case 'R':
            case 'G':
            case 'Y':
            case 'C':
            case 'M':
                res++;
                break;
            case ' ':
                break;
            default:
                freeTurtle(turtle);
                printInvalidMessage("Invalid characters present in the file\n");
        }
    }
    return res;
}

void dfs(Turtle* turtle, int initRow, int initCol, int numCells, Pair* pairList){
    if(numCells == turtle->numcells){
        processMoves(turtle, pairList);
        freeTurtle(turtle);
        exit(EXIT_SUCCESS);
    }
    for(int i=0;i<NUMDIRS;i++){
        int newR = initRow + dirr[i];
        int newC = initCol + dirc[i];
        if(isValid(newR, newC) && !turtle->visited[newR][newC] && turtle->matrix[newR][newC] != ' '){
            int size = pairList->size;
            pairList->arr[size][0] = newR;
            pairList->arr[size][1] = newC;
            pairList->size++;
            turtle->visited[newR][newC] = true;
            dfs(turtle, newR, newC, numCells+1, pairList);
            pairList->size = size;
            turtle->visited[newR][newC] = false;
        }
    }
}

static inline bool isValid(int row, int col){
    if(row >= RESHEIGHT || col >= RESWIDTH || row < 0 || col < 0){
        return false;
    }
    return true;
}

void processMoves(Turtle* turtle, Pair* pairList){
    int prevR = RESHEIGHT/2;
    int prevC = RESWIDTH/2;
    fprintf(turtle->op, "START\n");
    for(int i=0;i<pairList->size;i++){
        int nextR = pairList->arr[i][0];
        int nextC = pairList->arr[i][1];
        checkColour(prevR, prevC, nextR, nextC, turtle);
        writeMoves(prevR, prevC, nextR, nextC, turtle);
        prevR = nextR;
        prevC = nextC;
    }
}

void writeMoves(int prevR, int prevC, int nextR, int nextC, Turtle* turtle){
    int angle = getAngle(prevR, prevC, nextR, nextC);
    if(angle != turtle->angle){
        fprintf(turtle->op, "RIGHT %i\n", turtle->angle - angle);
        turtle->angle = angle;
    }
    fprintf(turtle->op, "FORWARD 1\n");
}

int getAngle(int prevR, int prevC, int nextR, int nextC){
    int diffy = nextR - prevR;
    int diffx = nextC - prevC;
    if(diffx == 0 && diffy == 1){
        return 0;
    }
    if(diffx == 1 && diffy == 1){
        return 45;
    }
    if(diffx == 1 && diffy == 0){
        return 90;
    }
    if(diffx == 1 && diffy == -1){
        return 135;
    }
    if(diffx == 0 && diffy == -1){
        return 180;
    }
    if(diffx == -1 && diffy == -1){
        return 225;
    }
    if(diffx == -1 && diffy == 0){
        return 270;
    }
    else{
        return 315;
    }
}

void checkColour(int prevR, int prevC, int nextR, int nextC, Turtle* turtle){
    if(turtle->matrix[prevR][prevC] != turtle->matrix[nextR][nextC]){
        char newColour[10];
        getColour(turtle->matrix[nextR][nextC], newColour);
        fprintf(turtle->op, "COLOUR \"%s\"\n", newColour);
    }
}

void getColour(char ch, char colour[10]){
    switch(ch){
        case 'W':
            strcpy(colour, "WHITE");
            break;
        case 'K':
            strcpy(colour, "BLACK");
            break;
        case 'R':
            strcpy(colour, "RED");
            break;
        case 'G':
            strcpy(colour, "GREEN");
            break;
        case 'B':
            strcpy(colour, "BLUE");
            break;
        case 'Y':
            strcpy(colour, "YELLOW");
            break;
        case 'M':
            strcpy(colour, "MAGENTA");
            break;
        case 'C':
            strcpy(colour, "CYAN");
            break;
    }
}

void printMatrix(Turtle* turtle){
    for(int i=0;i<RESHEIGHT;i++){
        for(int j=0;j<RESWIDTH;j++){
            printf("%c", turtle->matrix[i][j]);
        }
        printf("\n");
    }
}




