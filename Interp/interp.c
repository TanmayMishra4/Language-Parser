#include "interp.h"
// TODO check out of bounds for turtle
int main(int argc, char** argv){
    if(argc > 3 || argc < 2){
        fprintf(stderr, "Wrong number of params: Correct usage - ./parse [input file] [output file](optional)\n");
        exit(EXIT_FAILURE);
    }
    Turtle* res;
    if(argc == 2){
        res = init_turtle(NULL);
    }
    else if(argc ==  3){
        res = init_turtle(argv[2]);
    }
    char* file_name = argv[1];
    FILE* input_file = fopen(file_name, "r");
    if(input_file == NULL){
        fprintf(stderr, "Cannot open file\n");
        exit(EXIT_FAILURE);
    }
    bool is_valid = interp_file(input_file, res);
    // fclose(input_file);
    if(is_valid){
        if(argc == 3){ // output file case
            if(res->file){
                write_to_file(res, argv[2]);
                fclose(res->file);
            }
            free(res);
        }
        else if(argc == 2){ // no output file case
            if(res->file){
                fclose(res->file);
            }
            free(res);
        }
        return 0;
    }
    else{
        if(res->file){
            fclose(res->file);
        }
        free(res);
        return 1;
    }
}

Turtle* init_turtle(char* file_name){
    Turtle* res = (Turtle*)calloc(1, sizeof(Turtle));
    res->angle = INITIAL_ANGLE;
    res->colour = WHITE;
    if(file_name != NULL){
        char extension[10] = {0};
        get_file_extension(file_name, extension);
        if(strsame(extension, ".ps")){
            res->filetype = POSTSCRIPT_FILE;
            res->row = (double)RESHEIGHT/2;
            res->col = (double)RESWIDTH/2;
        }
        else{
            res->filetype = TEXT_FILE;
            res->row = (double)RESHEIGHT/2;
            res->col = (double)RESWIDTH/2;
        }
        res->file = fopen(file_name, "w");
    }
    else{
        res->filetype = NO_FILE;
        res->row = (double)TERMINALHEIGHT/2;
        res->col = (double)TERMINALWIDTH/2;
    }
    return res;
}

bool interp_file(FILE* file, Turtle* res){
    Program* prog = (Program*)calloc(1, sizeof(Program));
    int i = 0;
    while(fscanf(file, "%s", prog->words[i++])==1){
        strip_new_line(prog->words[i]);
    }
    fclose(file);
    bool is_valid = check_prog(prog, res);
    free(prog);
    return is_valid;
}

void strip_new_line(char* str){
    int len = strlen(str);
    int ind = len-1;
    while(ind >= 0 && str[ind] == '\n'){
        str[ind] = '\0';
        ind--;
    }
}

bool check_prog(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    if(!strsame(prog->words[curword], "START")){
        return false;
    }
    prog->curword++;
    bool is_inslst = check_inslst(prog, res);
    if(is_inslst){
        return true;
    }
    else{
        prog->curword = original_curword;
        return false;
    }
}

bool check_inslst(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    printf("inside inslst curword = %i, word = ", curword);
    puts(prog->words[curword]);
    if(strsame(prog->words[curword], "END")){
        prog->curword++;
        return true;
    }
    else{
        bool is_ins = check_ins(prog, res);
        if(is_ins){
            bool is_inslst = check_inslst(prog, res);
            if(is_inslst){
                return true;
            }
            else{
                prog->curword = original_curword;
                return false;
            }
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
}

bool check_ins(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = prog->curword;
    printf("inside ins curword = %i, word = ", curword);
    puts(prog->words[curword]);
    if(check_fwd(prog, res)){
        return true;
    }
    prog->curword = original_curword;
    if(check_rgt(prog, res)){
        return true;
    }
    prog->curword = original_curword;
    if(check_col(prog, res)){
        return true;
    }
    prog->curword = original_curword;
    if(check_loop(prog, res)){
        return true;
    }
    prog->curword = original_curword;
    if(check_set(prog, res)){
        return true;
    }
    else{
        prog->curword = original_curword;
        return false;
    }
}
//<FWD> ::= "FORWARD" <VARNUM>
bool check_fwd(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "FORWARD")){
        prog->curword++;
        int step_pos = prog->curword;
        bool is_valid = check_varnum(prog, res);
        if(is_valid){
            // double steps = extract_num(prog)
            int num = fetch_num(prog, step_pos, res);
            print_to_file(prog, res, num);
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    else{
        return false;
    }
}

bool check_rgt(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "RIGHT")){
        prog->curword++;
        int step_pos = prog->curword;
        bool is_valid = check_varnum(prog, res);
        if(is_valid){
            int num = fetch_num(prog, step_pos, res);
            process_rgt(res, num);
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    else{
        return false;
    }
}
// TODO handlew variable colour value and NOTCOLOUR
bool check_col(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "COLOUR")){
        prog->curword++;
        if(check_var(prog, res)){
            return true;
        }
        prog->curword = original_curword + 1;
        printf("inside col, word = ");
        puts(prog->words[prog->curword]);
        if(check_word(prog, res)){
            COLOUR colour = fetch_colour(prog, original_curword+1);
            process_colour(res, colour);
            return true;
        }
        prog->curword = original_curword;
        return false;
    }
    else{
        return false;
    }
}

bool check_loop(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "LOOP")){
        prog->curword++;
        if(!check_ltr(prog, 0, res)){
            prog->curword = original_curword;
            return false;
        }
        // prog->curword++;
        curword =  prog->curword;
        printf("after letter word = ");
        puts(prog->words[prog->curword]);
        if(!strsame(prog->words[curword], "OVER")){
            prog->curword = original_curword;
            return false;
        }
        prog->curword++;
        if(!check_lst(prog, res)){
            prog->curword = original_curword;
            return false;
        }
        // prog-
        if(!check_inslst(prog, res)){
            prog->curword = original_curword;
            return false;
        }
        return true;
    }
    else{
        return false;
    }
}

bool check_set(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    printf("inside set curword = %i, word = ", curword);
    puts(prog->words[curword]);
    if(strsame(prog->words[curword], "SET")){
        prog->curword++;
        if(!check_ltr(prog, 0, res)){
            prog->curword = original_curword;
            return false;
        }
        curword = prog->curword;
        if(!strsame(prog->words[curword], "(")){
            prog->curword = original_curword;
            return false;
        }
        prog->curword++;
        if(!check_pfix(prog, res)){
            prog->curword = original_curword;
            return false;
        }
        return true;
    }
    else{
        return false;
    }
}

bool check_varnum(Program* prog, Turtle* res){
    int original_curword = prog->curword;
    bool is_var = check_var(prog, res);
    if(is_var){
        return true;
    }
    prog->curword = original_curword;
    bool is_num = check_num(prog, res);
    if(is_num){
        return true;
    }
    prog->curword = original_curword;
    return false;
}
// TODO: Complete fully, only partially working now
bool check_word(Program* prog, Turtle* res){
    int curword = prog->curword;
    printf("inside word, word = ");
    puts(prog->words[curword]);
    // int original_curword = curword;
    int len = strlen(prog->words[curword]);
    if(prog->words[curword][0] != '"' && prog->words[curword][len-1] != '"'){
        return false;
    }
    for(int i=1;i<len-1;i++){
        if(prog->words[curword][i] == '"' || prog->words[curword][i] == ' ' || prog->words[curword][i] == '\n' || prog->words[curword][i] == '\t' || prog->words[curword][i] == '\f' || prog->words[curword][i] == '\v'){
            return false;
        }
    }
    // copy_word_from_str(word, prog->words[curword]);
    // if(strsame(word, "RED")){
    //     printf("matched\n");
    //     prog->curword++;
    //     return true;
    // }
    // else if(strsame(word, "GREEN")){
    //     prog->curword++;
    //     return true;
    // }
    // else if(strsame(word, "YELLOW")){
    //     prog->curword++;
    //     return true;
    // }
    // else if(strsame(word, "CYAN")){
    //     prog->curword++;
    //     return true;
    // }
    // else if(strsame(word, "MAGENTA")){
    //     prog->curword++;
    //     return true;
    // }
    // printf("should not be here\n");
    // prog->curword = original_curword;
    // return false;
    prog->curword++;
    return true;
}

bool check_var(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    if(prog->words[curword][0] == '$'){
        bool is_ltr = check_ltr(prog, 1, res);
        if(is_ltr){
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    prog->curword = original_curword;
    return false;
}

bool check_pfix(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], ")")){
        prog->curword++;
        return true;
    }
    else if(check_op(prog, res)){
        bool is_valid = check_pfix(prog, res);
        if(is_valid){
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    else if(check_varnum(prog, res)){
        bool is_valid = check_pfix(prog, res);
        if(is_valid){
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    else{
        return false;
    }
}

bool check_ltr(Program* prog, int index, Turtle* res){
    int curword = prog->curword;
    int len = strlen(prog->words[curword]);
    printf("inside ltr curword = %i, word = ", curword);
    puts(prog->words[curword]);
    if(len < 1 || len > 2){
        return false;
    }
    char letter = prog->words[curword][index];
    if(letter >=  'A' && letter <= 'Z'){
        prog->curword++;
        printf("here\n");
        return true;
    }
    return false;

}

bool check_lst(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    printf("inside lst curword = %i, word = ", curword);
    puts(prog->words[curword]);
    if(strsame(prog->words[curword], "{")){
        prog->curword++;
        bool is_valid = check_items(prog, res);
        if(is_valid){
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    else{
        return false;
    }
}

bool check_num(Program* prog, Turtle* res){
    int curword = prog->curword;
    double num;
    int num_vars = sscanf(prog->words[curword], "%lf", &num);
    if(num_vars != 1){
        return false;
    }
    prog->curword++;
    return  true;
}

bool check_op(Program* prog, Turtle* res){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "+")){
        prog->curword++;
        return true;
    }
    else if(strsame(prog->words[curword], "-")){
        prog->curword++;
        return true;
    }
    else if(strsame(prog->words[curword], "*")){
        prog->curword++;
        return true;
    }
    else if(strsame(prog->words[curword], "/")){
        prog->curword++;
        return true;
    }
    else{
        return false;
    }
}

bool check_items(Program* prog, Turtle* res){
    int curword = prog->curword;
    int original_curword = curword;
    printf("inside items curword = %i, word = ", curword);
    puts(prog->words[curword]);
    if(strsame(prog->words[curword], "}")){
        prog->curword++;
        return true;
    }
    bool is_valid = check_item(prog, res);
    if(is_valid){
        bool is_items = check_items(prog, res);
        if(is_items){
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    else{
        prog->curword = original_curword;
        return false;
    }
}

bool check_item(Program* prog, Turtle* res){
    int original_curword = prog->curword;
    printf("inside item, word = ");
    puts(prog->words[original_curword]);
    bool is_varnum = check_varnum(prog, res);
    if(is_varnum){
        return true;
    }
    prog->curword = original_curword;
    printf("inside item, word = ");
    puts(prog->words[prog->curword]);
    bool is_word = check_word(prog, res);
    if(is_word){
        return true;
    }
    prog->curword = original_curword;
    return false;
}

void get_file_extension(char* file_name, char* extension){
    char* ext = strchr(file_name, '.');
    if(!ext){
        return;
    }
    else{
        strcpy(extension, ext+1); 
    }
}
// TODO: handle variable case and decide if num is to be int or double!!
int fetch_num(Program* prog, int step_pos, Turtle* res){
    double num;
    if((sscanf(prog->words[step_pos], "%lf", &num) == 1)){
        return num;
    }
    // else{
    //     sscanf(prog->words[step_pos], );
    // }
    return 1;
}
// TODO check for out of bounds
void print_to_file(Program* prog, Turtle* res, int num){
    double angle = ((double)res->angle*PI)/180;
    if(res->filetype == NO_FILE){ // terminal case

    }
    else if(res->filetype == TEXT_FILE){ // TEXT FILE CASE
        double target_y = res->row + num*cos(angle);
        double target_x = res->col + num*sin(angle);
        char colour = convert_colour_to_char(res->colour);
        for(int i=0;i<num;i++){
            int x, y;
            printf("angle = %.2lf, cos(angle) = %.2lf, sine(angle) = %.2lf\n", angle, cos(angle), sin(angle));
            y = (int)(res->row);
            x = (int)(res->col);
            res->row = res->row + cos(angle);
            res->col = res->col + sin(angle);
            // res->row = y;
            // res->col = x;
            printf("coordinates = %i, %i\n", y, x);
            res->matrix[y][x] = colour;
        }
        // res->row = res->row - cos(angle);
        // res->col = res->col - sin(angle);
    }
    else{ // Post Script FILE case

    }
}

char convert_colour_to_char(COLOUR colour){
    char val;
    if(colour == WHITE){
        val = 'W';
    }
    else if(colour == BLACK){
        val = 'K';
    }
    else if(colour == RED){
        val = 'R';
    }
    else if(colour == GREEN){
        val = 'G';
    }
    else if(colour == YELLOW){
        val = 'Y';
    }
    else if(colour == BLUE){
        val = 'B';
    }
    else if(colour == MAGENTA){
        val = 'M';
    }
    else{
        val = 'C';
    }
    return val;
}
// TODO: handle case for PS file
void write_to_file(Turtle* res, char* file_name){
    if(res->filetype == TEXT_FILE){
        FILE* file = fopen(file_name, "w");
        for(int row=0;row<RESHEIGHT;row++){
            for(int col=0;col<RESWIDTH;col++){
                char c = res->matrix[row][col];
                c = (c != '\0'?c:' ');
                printf("%c", c);
                fprintf(file, "%c", c);
            }
            printf("\n");
            fprintf(file, "%c", '\n');
        }
        fclose(file);
    }
}
// TODO check if angle shoule be double or int
// TODO check if %360 is possible or not
void process_rgt(Turtle* res, int angle){
    res->angle = res->angle - angle;
    printf("angle = %.2lf\n", res->angle);
}

COLOUR fetch_colour(Program* prog, int curword){
    char* colour = prog->words[curword];
    COLOUR val;
    if(strsame(colour, "\"WHITE\"")){
        val = WHITE;
    }
    else if(strsame(colour, "\"BLACK\"")){
        val = BLACK;
    }
    else if(strsame(colour, "\"RED\"")){
        val = RED;
    }
    else if(strsame(colour, "\"GREEN\"")){
        val = GREEN;
    }
    else if(strsame(colour, "\"YELLOW\"")){
        val = YELLOW;
    }
    else if(strsame(colour, "\"BLUE\"")){
        val = BLUE;
    }
    else if(strsame(colour, "\"CYAN\"")){
        val = CYAN;
    }
    else{
        val = MAGENTA;
    }
    return val;
}

void process_colour(Turtle* res, COLOUR colour){
    res->colour = colour;
}


