#include "interp.h"
// TODO check out of bounds for turtle
int main(int argc, char** argv){
    test();
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
        free_turtle(res);
        // printf("closing file at line 20\n");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }
    bool is_valid = interp_file(input_file, res);
    // fclose(input_file);
    if(is_valid){
        if(argc == 3){ // output file case
            write_to_file(res, argv[2]);
            // if(res->file){
            //     printf("closing file at line 30\n");
            //     fclose(res->file);
            //     res->file = NULL;
            // }
            free_turtle(res);
        }
        else if(argc == 2){ // no output file case
            // if(res->file){
            //     printf("closing file at line 723\n");
            //     // fclose(res->file);
            //     // res->file = NULL;
            // }
            // fclose(input_file);
            // TODO figure out the end position of the terminal cursor
            // neillmovecursortopos(TERMINALHEIGHT+1, 0);
            char x = getchar();
            x++;
            tc_exit_alt_screen();
            free_turtle(res);
        }
        return 0;
    }
    else{
        if(res->filetype != NO_FILE && res->file){
            fclose(res->file);
            res->file = NULL;
        }
        free_turtle(res);
        return 1;
    }
}

Turtle* init_turtle(char* file_name){
    Turtle* res = (Turtle*)calloc(1, sizeof(Turtle));
    res->colour = white;
    res->file = NULL;
    if(file_name != NULL){
        char extension[10] = {0};
        get_file_extension(file_name, extension);
        // printf("extension is %s\n", extension);
        if(strsame(extension, "ps")){
            res->file = fopen(file_name, "w");
            // printf("file is postscript file\n");
            res->filetype = POSTSCRIPT_FILE;
            res->row = PSHEIGHT;
            res->angle = INITIALPS_ANGLE;
            res->col = PSWIDTH;
            res->maxrow = PSHEIGHT*2;
            res->maxcol = PSWIDTH*2;
            fprintf(res->file, "%s %s\n", "0.2", "setlinewidth");
            fprintf(res->file, "%s %s %s\n", "10", "10", "scale");
        }
        else{
            res->angle = INITIAL_ANGLE;
            res->filetype = TEXT_FILE;
            res->row = (double)RESHEIGHT/2;
            res->col = (double)RESWIDTH/2;
            res->maxrow = (double)(RESHEIGHT);
            res->maxcol = (double)(RESWIDTH);
        }
    }
    else{
        struct winsize size;
        ioctl(1, TIOCGWINSZ, &size);
        res->angle = TERMINALANGLE;
        res->filetype = NO_FILE;
        tc_enter_alt_screen();
        // printf("colsize = %ui, rowsize = %ui\n", size.ws_col, size.ws_row);
        res->maxrow = (double)(size.ws_row);
        res->maxcol = (double)(size.ws_col);
        res->row = (double)(size.ws_row/2);
        res->col = (double)(size.ws_col/2);
        // initializescreen(res);
    }
    return res;
}

bool interp_file(FILE* file, Turtle* res){
    Program* prog = (Program*)calloc(1, sizeof(Program));
    prog->stack = coll_init();
    int i = 0;
    while(fscanf(file, "%s", prog->words[i++])==1){
        strip_new_line(prog->words[i]);
    }
    fclose(file);
    bool is_valid = check_prog(prog, res);
    coll_free(prog->stack);
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
    // printf("inside inslst curword = %i, word = ", curword);
    // puts(prog->words[curword]);
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
    // int curword = prog->curword;
    int original_curword = prog->curword;
    // printf("inside ins curword = %i, word = ", curword);
    // puts(prog->words[curword]);
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
        VAR var;
        strcpy(var.strval, "\0");
        bool is_valid = check_varnum(prog, &var);
        if(is_valid){
            // int num = fetch_num(prog, step_pos, res);
            double num = var.numval;
            bool is_val = print_to_file(res, num);
            if(is_val == false){
                return false;
            }
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
        VAR var;
        strcpy(var.strval, "\0");
        bool is_valid = check_varnum(prog, &var);
        if(is_valid){
            int num = (int)var.numval;
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
        VAR var;
        strcpy(var.strval, "\0");
        if(check_var(prog, &var)){
            neillcol colour;
            bool is_valid = fetch_colour_var(&var, &colour);
            if(!is_valid){
                return false;
            }
            process_colour(res, colour);
            return true;
        }
        prog->curword = original_curword + 1;
        // printf("inside col, word = ");
        // puts(prog->words[prog->curword]);
        if(check_word(prog)){
            neillcol colour;
            bool is_valid = fetch_colour(prog->words[original_curword+1], &colour);
            if(!is_valid){
                return false;
            }
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
        if(!check_ltr(prog, 0)){
            prog->curword = original_curword;
            return false;
        }
        // prog->curword++;
        curword =  prog->curword;
        // printf("after letter word = ");
        // puts(prog->words[prog->curword]);
        char var_name = str_to_var(prog->words[curword-1]);
        // printf("loop variable = %c\n", var_name);
        if(!strsame(prog->words[curword], "OVER")){
            prog->curword = original_curword;
            return false;
        }
        prog->curword++;
        LOOPLIST loop_lst;
        loop_lst.size = 0;
        loop_lst.curr_index = 0;
        if(!check_lst(prog, res, &loop_lst)){
            prog->curword = original_curword;
            return false;
        }
        // printf("loop items are\n");
        // for(int i=0;i<loop_lst.size;i++){
        //     VAR v = loop_lst.list[i];
        //     if(v.vartype == STRING){
        //         // printf("%s ", v.strval);
        //     }
        //     else{
        //         // printf("%lf ", v.numval);
        //     }
        // }
        // printf("\n");
        // prog-
        // printf("before instlst curwords = %s\n", prog->words[prog->curword]);
        int pos_before = prog->curword;
        for(int i=0;i<loop_lst.size;i++){
            prog->curword = pos_before;
            VAR cur_val = loop_lst.list[i];
            set_var(prog, var_name, &cur_val);
            if(!check_inslst(prog, res)){
                prog->curword = original_curword;
                return false;
            }
            // prog->curword = prog->curword
            // printf("after instlst curwords = %s\n", prog->words[prog->curword]);
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
    if(strsame(prog->words[curword], "SET")){
        prog->curword++;
        if(!check_ltr(prog, 0)){
            prog->curword = original_curword;
            return false;
        }
        // printf("variable name = %s\n", prog->words[prog->curword-1]);
        char var_name = str_to_var(prog->words[prog->curword-1]);
        prog->is_var_used[var_name-'A'] = true;
        curword = prog->curword;
        if(!strsame(prog->words[curword], "(")){
            prog->curword = original_curword;
            return false;
        }
        prog->curword++;
        VAR val;
        strcpy(val.strval, "\0");
        if(!check_pfix(prog, res, &val)){
            prog->curword = original_curword;
            return false;
        }
        set_var(prog, var_name, &val);
        return true;
    }
    else{
        return false;
    }
}

bool check_varnum(Program* prog, VAR* num){
    int original_curword = prog->curword;
    bool is_var = check_var(prog, num);
    if(is_var){
        return true;
    }
    prog->curword = original_curword;
    bool is_num = check_num(prog, num);
    if(is_num){
        return true;
    }
    prog->curword = original_curword;
    return false;
}
// TODO: Complete fully, only partially working now
bool check_word(Program* prog){
    int curword = prog->curword;
    // printf("inside word, word = ");
    // puts(prog->words[curword]);
    // int original_curword = curword;
    int len = strlen(prog->words[curword]);
    if(len <= 2){
        return false;
    }
    if(prog->words[curword][0] != '"' || prog->words[curword][len-1] != '"'){
        return false;
    }
    for(int i=1;i<len-1;i++){
        if(prog->words[curword][i] == '"' || prog->words[curword][i] == ' ' || prog->words[curword][i] == '\n' || prog->words[curword][i] == '\t' || prog->words[curword][i] == '\f' || prog->words[curword][i] == '\v'){
            return false;
        }
    }

    prog->curword++;
    return true;
}

bool check_var(Program* prog, VAR* var){
    int curword = prog->curword;
    int original_curword = curword;
    if(prog->words[curword][0] == '$'){
        bool is_ltr = check_ltr(prog, 1);
        if(is_ltr){
            char var_name = str_to_var(prog->words[curword]);
            int pos = var_name - 'A';
            // check if variable has been set before
            if(prog->is_var_used[pos]){
                var->vartype = prog->variables[pos].vartype;
                var->numval = prog->variables[pos].numval;
                strcpy(var->strval, prog->variables[pos].strval);
            }
            else{
                return false;
            }
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

bool check_pfix(Program* prog, Turtle* res, VAR* var){
    int curword = prog->curword;
    int original_curword = curword;
    char op;
    if(strsame(prog->words[curword], ")")){
        coll_pop(prog->stack, var);
        prog->curword++;
        return true;
    }
    if(check_op(prog, &op)){
        if(prog->stack->size <= 1){ //if stack is empty
            return false;
        }
        bool did_update = update_stack(prog->stack, op);
        if(!did_update){ // check for divide by zero error
            return false;
        }
        bool is_valid = check_pfix(prog, res, var);
        if(is_valid){
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    // printf("curword = %s is not operator\n", prog->words[prog->curword]);
    if(check_varnum(prog, var)){
        // printf("inside pfix, added %lf to stack\n", var->numval);
        coll_add(prog->stack, *var);
        bool is_valid = check_pfix(prog, res, var);
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

bool check_ltr(Program* prog, int index){
    int curword = prog->curword;
    int len = strlen(prog->words[curword]);
    if(len < 1 || len > 2){
        return false;
    }
    char letter = prog->words[curword][index];
    if(letter >=  'A' && letter <= 'Z'){
        prog->curword++;
        return true;
    }
    return false;

}

bool check_lst(Program* prog, Turtle* res, LOOPLIST* loop_lst){
    int curword = prog->curword;
    int original_curword = curword;

    if(strsame(prog->words[curword], "{")){
        prog->curword++;
        bool is_valid = check_items(prog, res, loop_lst);
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

bool check_num(Program* prog, VAR* var){
    int curword = prog->curword;
    double num;
    int num_vars = sscanf(prog->words[curword], "%lf", &num);
    if(num_vars != 1){
        return false;
    }
    var->vartype = DOUBLE;
    var->numval = num;
    strcpy(var->strval, "\0");
    prog->curword++;
    return  true;
}

bool check_op(Program* prog, char* op){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "+")){
        prog->curword++;
        op[0] = '+';
        return true;
    }
    else if(strsame(prog->words[curword], "-")){
        prog->curword++;
        op[0] = '-';
        return true;
    }
    else if(strsame(prog->words[curword], "*")){
        prog->curword++;
        op[0] = '*';
        return true;
    }
    else if(strsame(prog->words[curword], "/")){
        prog->curword++;
        op[0] = '/';
        return true;
    }
    else{
        return false;
    }
}

bool check_items(Program* prog, Turtle* res, LOOPLIST* loop_lst){
    int curword = prog->curword;
    int original_curword = curword;
    // printf("inside items curword = %i, word = ", curword);
    // puts(prog->words[curword]);
    if(strsame(prog->words[curword], "}")){
        prog->curword++;
        return true;
    }
    bool is_valid = check_item(prog, loop_lst);
    if(is_valid){
        bool is_items = check_items(prog, res, loop_lst);
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

bool check_item(Program* prog, LOOPLIST* loop_lst){
    int original_curword = prog->curword;
    // printf("inside item, word = ");
    // puts(prog->words[original_curword]);
    VAR var;
    strcpy(var.strval, "\0");
    bool is_varnum = check_varnum(prog, &var);
    if(is_varnum){
        add_to_looplist(loop_lst, var);
        return true;
    }
    prog->curword = original_curword;
    // printf("inside item, word = ");
    // puts(prog->words[prog->curword]);
    var.vartype = STRING;
    strcpy(var.strval, prog->words[prog->curword]);
    bool is_word = check_word(prog);
    if(is_word){
        add_to_looplist(loop_lst, var);
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

void print_stack(coll* c){
    // printf("PRINTING STACK left to right\n");
    for(int i=0;i<c->size;i++){
        // printf("%0.4f  ", c->a[i].numval);
    }
    // printf("\n");
}
// TODO: handle variable case and decide if num is to be int or double!!
// int fetch_num(Program* prog, int step_pos, Turtle* res){
//     double num;
//     if((sscanf(prog->words[step_pos], "%lf", &num) == 1)){
//         return num;
//     }
//     // else{
//     //     sscanf(prog->words[step_pos], );
//     // }
//     return 1;
// }
// TODO check for out of bounds
bool print_to_file(Turtle* res, double num){
    double angle = ((double)res->angle*PI)/180;
    // printf("colour is %u\n", res->colour);
    // printf("colour char is %c\n", convert_colour_to_char(res->colour));
    if(res->filetype == NO_FILE){ // terminal case
        char colour = convert_colour_to_char(res->colour);
        double multiplier = (num >= 0?1.0:-1.0);
        num = fabs(num);
        // int prevx = (int)res->col;
        // int prevy = (int)res->row;
        neillfgcol(res->colour);
        for(int i=0;i<(int)num;i++){
            int x, y;
            // printf("angle = %.2lf, cos(angle) = %.2lf, sine(angle) = %.2lf\n", angle, cos(angle), sin(angle));   
            y = (int)(res->row);
            x = (int)(res->col);
            // int diffy = y - prevy;
            // int diffx = x - prevx;
            
            res->row = res->row + multiplier*cos(angle);
            res->col = res->col + multiplier*sin(angle);
            // printf("angle = %lf, rownext = %lf, colnext = %lf\n", res->angle, res->row, res->col);
            // res->row = y;
            // res->col = x;
            
            // printf("coordinates = %i, %i\n", y, x);
            if(!is_validcoord(y, x, res->maxrow, res->maxcol)){
                fprintf(stderr, "limits for row = %lf, col = %lf\n", res->maxrow, res->maxcol);
	            fprintf(stderr, "Out of bounds for values row = %i, col = %i\n", y, x);
                return false;
	        }
            print_to_screen(x, y, colour);
            fflush(stdout);
        }
        neillbusywait(1.0);
    }
    else if(res->filetype == TEXT_FILE){ // TEXT FILE CASE
        // printf("num = %lf\n", num);
        char colour = convert_colour_to_char(res->colour);
        double multiplier = (num >= 0?1.0:-1.0);
        num = fabs(num);
        for(int i=0;i<(int)num;i++){
            int x, y;
            // printf("angle = %.2lf, cos(angle) = %.2lf, sine(angle) = %.2lf\n", angle, cos(angle), sin(angle));   
            y = (int)(res->row);
            x = (int)(res->col);
            res->row = res->row - multiplier*cos(angle);
            res->col = res->col + multiplier*sin(angle);
            // res->row = y;
            // res->col = x;
            // printf("coordinates = %i, %i\n", y, x);
            if(!is_validcoord(y, x, res->maxrow, res->maxcol)){
                fprintf(stderr, "limits for row = %lf, col = %lf\n", res->maxrow, res->maxcol);
	            fprintf(stderr, "Out of bounds for values row = %i, col = %i\n", y, x);
                return false;
	        }
            res->matrix[y][x] = colour;
        }
        // res->row = res->row - multiplier*cos(angle);
        // res->col = res->col - multiplier*sin(angle);
        // res->row = res->row - cos(angle);
        // res->col = res->col - sin(angle); 
        // double x1, x2, y1, y2;
        // x1 = res->col;
        // y1 = res->row;
        // x2 = res->col + num*sin(angle);
        // y2 = res->row + num*cos(angle);
        // double diffx = x2-x1;
        // double diffy = y2-y1;
        // int n  = num;
        // int i = 0;
        // while(i < num){
        //     x1 = x1 + diffx/n;
        //     y1 = y1 + diffy/n;
        //     int y = (int)y1;
        //     int x = (int)x1;
        //     res->matrix[y][x] =colour;
        //     i++;
        // }
        // res->col = x2;
        // res->row = y2;
    }
    else{ // Post Script FILE case
        double x1, x2, y1, y2;
        x1 = res->col;
        y1 = res->row;
        x2 = res->col - (double)num*sin(angle);
        y2 = res->row + (double)num*cos(angle);
        res->col = x2;
        res->row = y2;
        write_to_ps(res, x1, x2, y1, y2);
    }
    return true;
}

char convert_colour_to_char(neillcol colour){
    char val;
    if(colour == white){
        val = 'W';
    }
    else if(colour == black){
        val = 'K';
    }
    else if(colour == red){
        val = 'R';
    }
    else if(colour == green){
        val = 'G';
    }
    else if(colour == yellow){
        val = 'Y';
    }
    else if(colour == blue){
        val = 'B';
    }
    else if(colour == magenta){
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
                // printf("%c", c);
                fprintf(file, "%c", c);
            }
            // printf("\n");
            fprintf(file, "%c", '\n');
        }
        // printf("closing file at line 719\n");
        fclose(file);
    }
    else if(res->filetype == POSTSCRIPT_FILE){
        fprintf(res->file, "%s", "showpage\n");
        // printf("closing file at line 724\n");
        fclose(res->file);
        char command[COMMAND_LEN] = {0};
        get_command(command, file_name);
        // printf("command = \n");
        puts(command);
        system(command);
    }
}
// TODO check if angle shoule be double or int
// TODO check if %360 is possible or not
void process_rgt(Turtle* res, int angle){
    if(res->filetype == TEXT_FILE){
        res->angle = res->angle + angle;
    }
    else{
        res->angle = res->angle - angle;
    }
    
    // printf("angle = %.2lf\n", res->angle);
}

bool fetch_colour(char* colour, neillcol* val){
    if(strsame(colour, "\"WHITE\"")){
        *val = white;
    }
    else if(strsame(colour, "\"BLACK\"")){
        *val = black;
    }
    else if(strsame(colour, "\"RED\"")){
        *val = red;
    }
    else if(strsame(colour, "\"GREEN\"")){
        *val = green;
    }
    else if(strsame(colour, "\"YELLOW\"")){
        *val = yellow;
    }
    else if(strsame(colour, "\"BLUE\"")){
        *val = blue;
    }
    else if(strsame(colour, "\"CYAN\"")){
        *val = cyan;
    }
    else if(strsame(colour, "\"MAGENTA\"")){
        *val = magenta;
    }
    else{
        return false;
    }
    return true;
}

void process_colour(Turtle* res, neillcol colour){
    res->colour = colour;
}

char str_to_var(char* str){
    int len = strlen(str);
    if(len == 1){
        return str[0];
    }
    else if(len == 2){
        return str[1];
    }
    else{
        // printf("invalid string passed to str_to_val\n");
        return 'A'+26;
    }
}

void set_var(Program* prog, char var_name, VAR* val){
    int pos = var_name - 'A';
    prog->variables[pos].numval = val->numval;
    prog->variables[pos].vartype = val->vartype;
    strcpy(prog->variables[pos].strval, val->strval);
    prog->is_var_used[pos] = true;
    // printf("var %c set to %lf\n", var_name, prog->variables[pos].numval);
    // puts(prog->variables[pos].strval);
}

bool fetch_colour_var(VAR* var, neillcol* val){
    if(var->vartype != STRING){
        return false;
    }
    bool is_val = fetch_colour(var->strval, val);
    if(is_val){
        return true;
    }
    return false;
}

bool isnumber(char* str){
    double val;
    int numvars = sscanf(str, "%lf", &val);
    if(numvars == 1){
        return true;
    }
    return false;
}

// STACK FUNCTIONS

coll* coll_init(void)
{
    coll* c = (coll*) calloc(1, sizeof(coll));
    c->size = 0;
    return c;
}

int coll_size(coll* c)
{
    if(c==NULL){
        return 0;
    }
    return c->size;
}

void coll_add(coll* c, VAR d)
{
    if(c){
        int size = c->size;
        if(size >= STACKSIZE){
            fprintf(stderr, "STACK COLLECTION OVERFLOW in stack.h\n");
            exit(EXIT_FAILURE);
        }
        c->a[size].vartype = d.vartype;
        c->a[size].numval = d.numval;
        if(d.vartype == STRING){
            strcpy(c->a[size].strval, d.strval);
        }
        // printf("stack top is %f\n", d.numval);
        c->size = c->size + 1;
        // printf("calling from coll_add\n");
        // print_stack(c);
    }
}

bool coll_pop(coll* c, VAR* res){
    if(!c || c->size == 0){
        return false;
    }
    int size = c->size;
    VAR top = c->a[size-1];
    res->vartype = top.vartype;
    res->numval = top.numval;
    strcpy(res->strval, top.strval);
    c->size = c->size-1;
    // printf("calling from coll_pop\n");
    // print_stack(c);
    return true;
}

bool coll_free(coll* c)
{
    if(c){
        free(c);
        c = NULL;
        return true;
    }
    return false;
}

bool update_stack(coll* stack, char op){
    VAR a;
    strcpy(a.strval, "\0");
    VAR b;
    strcpy(b.strval, "\0");
    // printf("values popped = %f, %f\n", b.numval, a.numval);
    coll_pop(stack, &a);
    coll_pop(stack, &b);
    // printf("values on stack = %lf, %lf\n", a.numval, b.numval);
    double res;
    if(op == '+'){
        res = b.numval + a.numval;
    }
    else if(op == '-'){
        res = b.numval - a.numval;
    }
    else if(op == '*'){
        res = b.numval * a.numval;
    }
    else{
        // if(a.numval == 0){
        //     fprintf(stderr, "divide by zero\n");
        //     return false;
        // }
        res = b.numval / a.numval;
    }
    VAR c;
    strcpy(c.strval, "\0");
    c.vartype = DOUBLE;
    c.numval = res;
    coll_add(stack, c);
    // printf("top of stack after op %c is %0.1f\n", op, c.numval);
    // printf("calling from update stack\n");
    // print_stack(stack);
    return true;
}

void add_to_looplist(LOOPLIST* looplst, VAR d){
    int size = looplst->size;
    looplst->list[size].vartype = d.vartype;
    looplst->list[size].numval = d.numval;
    strcpy(looplst->list[size].strval, d.strval);
    looplst->size = size + 1;
}

void write_to_ps(Turtle* res, double x1, double x2, double y1, double y2){
    fprintf(res->file, "newpath\n");
    fprintf(res->file, "%lf %lf moveto\n", x1, y1);
    fprintf(res->file, "%lf %lf lineto\n", x2, y2);
    double r, g, b;
    getrgbcolor(res->colour, &r, &g, &b);
    fprintf(res->file, "%.1lf %.1lf %.1lf setrgbcolor\n", r, g, b);
    fprintf(res->file, "stroke\n");
}

void getrgbcolor(neillcol colour, double* r, double* g, double* b){
    switch(colour){
        case black:
            *r = 0;
            *g = 0;
            *b = 0;
            return;
        case red:
            *r = 1;
            *g = 0;
            *b = 0;
            return;
        case blue:
            *r = 0;
            *g = 0;
            *b = 1;
            return;
        case green:
            *r = 0;
            *g = 1;
            *b = 0;
            return;
        case cyan:
            *r = 0;
            *g = 1;
            *b = 1;
            return;
        case yellow:
            *r = 1;
            *g = 1;
            *b = 0;
            return;
        case magenta:
            *r = 1;
            *g = 0;
            *b = 1;
            return;
        default:
            *r = 0.8;
            *g = 0.8;
            *b = 0.8;
            return;
    }
}

void get_command(char* command, char* file_name){
    strcpy(command, "ps2pdf ");
    char outputfile[COMMAND_LEN]= {0};
    strcpy(outputfile, file_name);
    strcat(command, outputfile);
    int len = strlen(outputfile);
    outputfile[len-1] = 'd';
    outputfile[len] = 'f';
    outputfile[len+1] = '\0';
    strcat(command, " ");
    strcat(command, outputfile);
    // TODO change output name to something relevant
}

void free_turtle(Turtle* res){
    if(res){
        free(res);
        res = NULL;
    }
}
// TODO decide defauly bg colour
void initializescreen(Turtle* res){
    int r = res->row;
    int c = res->col;
    neillclrscrn();
    neillmovecursortopos(r, c);
    // printf("hello world");
    // neillbgcol(BACKGROUND);
    // neillfgcol(res->colour);
}

void print_to_screen(int x, int y, char colour){
    neillmovecursortopos(y, x);
    printf("%c", colour);
}

bool is_validcoord(int r, int c, int m, int n){
    if(r < 0 || c < 0 || r >= m || c >= n){
        return false;
    }
    return true;
}

Program* get_program(FILE* file){
    Program* prog = (Program*)calloc(1, sizeof(Program));
    int i = 0;
    prog->stack = coll_init();
    while(fscanf(file, "%s", prog->words[i++])==1){
        strip_new_line(prog->words[i]);
    }
    return prog;
}

void free_prog(Program* prog){
    if(prog->stack != NULL){
        free(prog->stack);
    }
    free(prog);
}

// TESTING FUNCTIONS

void test(void){
    test_check_ltr();
    test_check_var();
    test_check_num();
    test_check_word();
    test_check_op();
    test_check_varnum();
    test_fwd();
    test_rgt();
    test_col();
    test_set();
    test_item();
    test_items();
    test_lst();
    test_loop();
    test_ins();
    test_inslst();
    // test_prog();
    test_process_rgt();
    test_convert_colour_to_char();
    test_fetch_colour();
    test_process_colour();
    test_str_to_var();
    test_is_validcoord();
    test_update_stack();
    test_add_to_looplist();
}

void test_check_ltr(void){
    FILE* file = fopen("Testing/LTR/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_ltr(prog, 0) == true);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/LTR/digit.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/LTR/smallcase.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/LTR/word.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);

    free_prog(prog);
}

void test_check_var(void){
    FILE* file = fopen("Testing/VAR/correct.ttl", "r");
    VAR var;
    Program* prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);
    free_prog(prog);


    file = fopen("Testing/VAR/digit.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);
    free_prog(prog);

    Turtle* res = init_turtle("test_file_for_tests.txt");
    file = fopen("Testing/VAR/setanduse.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog, res));
    assert(check_var(prog, &var) == true);
    assert(var.vartype == DOUBLE);
    assert(var.numval > 4.9 && var.numval < 5.1);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    res = init_turtle("test_file_for_tests.txt");
    file = fopen("Testing/VAR/setanduse2.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog, res));
    assert(check_var(prog, &var) == true);
    assert(var.vartype == DOUBLE);
    assert(var.numval > 11.9 && var.numval < 12.1);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/VAR/smallcase.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/VAR/word.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/VAR/nodollar.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);

    free_prog(prog);
}

void test_check_num(void){
    FILE* file = fopen("Testing/NUM/correct.ttl", "r");
    Program* prog = get_program(file);
    VAR var;
    assert(check_num(prog, &var) == true);
    assert(var.vartype == DOUBLE);
    assert(var.numval > 384.727 && var.numval < 384.729);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/NUM/negative.ttl", "r");
    prog = get_program(file);
    assert(check_num(prog, &var) == true);
    assert(var.vartype == DOUBLE);
    assert(var.numval > -32468 && var.numval < -32466);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/NUM/bignum.ttl", "r");
    prog = get_program(file);
    assert(check_num(prog, &var) == true);
    assert(var.vartype == DOUBLE);
    assert(var.numval > 2934928 && var.numval < 2934930);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/NUM/notnum.ttl", "r");
    prog = get_program(file);
    assert(check_num(prog, &var) == false);
    fclose(file);
    free_prog(prog);
}

void test_check_word(void){
    FILE* file = fopen("Testing/WORD/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_word(prog) == true);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/WORD/empty.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/WORD/incomp.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/WORD/notword.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/WORD/number.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == true);
    fclose(file);

    free_prog(prog);
}

void test_check_op(void){
    FILE* file = fopen("Testing/OP/add.ttl", "r");
    Program* prog = get_program(file);
    char op;
    assert(check_op(prog, &op) == true);
    assert(op == '+');
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/OP/subtract.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog, &op) == true);
    assert(op == '-');
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/OP/multiply.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog, &op) == true);
    assert(op == '*');
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/OP/divide.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog, &op) == true);
    assert(op == '/');
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/OP/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog, &op) == false);
    fclose(file);

    free_prog(prog);
}

void test_check_varnum(void){
    FILE* file = fopen("Testing/VARNUM/num.ttl", "r");
    Program* prog = get_program(file);
    VAR var;
    assert(check_varnum(prog, &var) == true);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/VARNUM/var.ttl", "r");
    prog = get_program(file);
    assert(check_varnum(prog, &var) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/VARNUM/var.ttl", "r");
    prog = get_program(file);
    prog->is_var_used['V'-'A'] = true;
    assert(check_varnum(prog, &var) == true);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/VARNUM/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_varnum(prog, &var) == false);
    fclose(file);
    free_prog(prog);

    file = fopen("Testing/VARNUM/incorrect2.ttl", "r");
    prog = get_program(file);
    assert(check_varnum(prog, &var) == false);
    fclose(file);

    free_prog(prog);
}

void test_fwd(void){
    FILE* file = fopen("Testing/FWD/correctnum.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    assert(check_fwd(prog, res) == true);
    assert(res->col > 24 && res->col < 27);
    assert(res->row > 5 && res->row < 7);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/FWD/correctvar.ttl", "r");
    prog = get_program(file);
    prog->is_var_used[3] = true;
    prog->variables[3].vartype = DOUBLE;
    prog->variables[3].numval = 4.0;
    res = init_turtle("test_file_for_tests.txt");
    assert(check_fwd(prog, res) == true);
    assert(res->col > 24 && res->col < 27);
    assert(res->row > 11 && res->row < 13);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/FWD/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_fwd(prog, res) == false);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/FWD/mispelled.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_fwd(prog, res) == false);
    fclose(file);
    free_turtle(res);

    free_prog(prog);
}

void test_rgt(void){
    FILE* file = fopen("Testing/RGT/correctnum.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    assert(check_rgt(prog, res) == true);
    assert(res->angle > 44 && res->angle < 46);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/RGT/correctvar.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    prog->is_var_used['X'-'A'] = true;
    prog->variables['X'-'A'].vartype = DOUBLE;
    prog->variables['X'-'A'].numval = 90.0;
    assert(check_rgt(prog, res) == true);
    assert(res->angle > 89 && res->angle < 91);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/RGT/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_rgt(prog, res) == false);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/RGT/mispelled.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_rgt(prog, res) == false);
    fclose(file);

    free_prog(prog);
    free_turtle(res);
}

void test_col(void){
    FILE* file = fopen("Testing/COL/correct.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    assert(check_col(prog, res) == true);
    assert(res->colour == yellow);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/COL/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_col(prog, res) == false);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/COL/invalidcol.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_col(prog, res) == false);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/COL/number.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_col(prog, res) == false);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/COL/varcol.ttl", "r");
    prog = get_program(file);
    prog->is_var_used[0] = true;
    prog->variables[0].vartype = STRING;
    strcpy(prog->variables[0].strval, "\"GREEN\"");
    res = init_turtle("test_file_for_tests.txt");
    assert(check_col(prog, res) == true);
    assert(res->colour == green);
    fclose(file);
    free_turtle(res);

    free_prog(prog);
}

void test_set(void){
    FILE* file = fopen("Testing/SET/correct.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests");
    assert(check_set(prog, res) == true);
    assert(prog->is_var_used[0]);
    assert(prog->variables[0].numval > 3.0 && prog->variables[0].numval < 5.0);
    assert(prog->variables[0].vartype == DOUBLE);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/SET/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests");
    assert(check_set(prog, res) == false);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/SET/incorrect2.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests");
    assert(check_set(prog, res) == false);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/SET/emptyvalue.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests");
    assert(check_set(prog, res) == true);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/SET/pfix.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests");
    assert(check_set(prog, res) == true);
    assert(prog->is_var_used[0]);
    assert(prog->variables[0].numval > 2.0 && prog->variables[0].numval < 4.0);
    assert(prog->variables[0].vartype == DOUBLE);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/SET/word.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests");
    assert(check_set(prog, res) == false);
    assert(prog->is_var_used[0]);
    fclose(file);
    free_turtle(res);

    free_prog(prog);
}

void test_item(void){
    // add test case for checking num where num = 563yt
    FILE* file = fopen("Testing/ITEM/correct.ttl", "r");
    Program* prog = get_program(file);
    LOOPLIST* looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_item(prog, looplst) == true);
    assert(looplst->size == 1);
    assert(looplst->list[0].vartype == STRING);
    assert(strcmp(looplst->list[0].strval, "\"SDKFNN\"") == 0);
    fclose(file);
    free(looplst);
    free_prog(prog);

    file = fopen("Testing/ITEM/incorrect.ttl", "r");
    prog = get_program(file);
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_item(prog, looplst) == false);
    fclose(file);
    free(looplst);
    free_prog(prog);

    file = fopen("Testing/ITEM/varnum.ttl", "r");
    prog = get_program(file);
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    prog->is_var_used[0] = true;
    prog->variables[0].numval = 5;
    prog->variables[0].vartype = DOUBLE;
    assert(check_item(prog, looplst) == true);
    assert(looplst->size == 1);
    assert(looplst->list[0].vartype == DOUBLE);
    assert(looplst->list[0].numval > 4.0 &&  looplst->list[0].numval < 6.0);
    fclose(file);
    free(looplst);
    free_prog(prog);

    file = fopen("Testing/ITEM/varnum2.ttl", "r");
    prog = get_program(file);
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_item(prog, looplst) == true);
    assert(looplst->size == 1);
    assert(looplst->list[0].vartype == DOUBLE);
    assert(looplst->list[0].numval > 4337.0 &&  looplst->list[0].numval < 4339.0);
    fclose(file);
    free(looplst);
    free_prog(prog);

    file = fopen("Testing/ITEM/smallcase.ttl", "r");
    prog = get_program(file);
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_item(prog, looplst) == true);
    assert(looplst->size == 1);
    assert(looplst->list[0].vartype == STRING);
    assert(strcmp(looplst->list[0].strval, "\"skdjfk\"") == 0);
    fclose(file);
    free(looplst);

    free_prog(prog);
}

void test_items(void){
    FILE* file = fopen("Testing/ITEMS/correct.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    LOOPLIST* looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_items(prog, res, looplst) == true);
    assert(looplst->size == 5);
    assert(looplst->list[2].vartype == DOUBLE);
    assert(looplst->list[2].numval > 5.0 &&  looplst->list[2].numval < 7.0);
    assert(looplst->list[3].vartype == DOUBLE);
    assert(looplst->list[3].numval > 6.0 &&  looplst->list[3].numval < 8.0);
    fclose(file);
    free_prog(prog);
    free_turtle(res);
    free(looplst);

    file = fopen("Testing/ITEMS/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_items(prog, res, looplst) == false);
    fclose(file);
    free_prog(prog);
    free_turtle(res);
    free(looplst);

    file = fopen("Testing/ITEMS/empty.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_items(prog, res, looplst) == true);
    fclose(file);
    free_prog(prog);
    free_turtle(res);
    free(looplst);

    file = fopen("Testing/ITEMS/words.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_items(prog, res, looplst) == true);
    assert(looplst->size == 4);
    assert(looplst->list[1].vartype == STRING);
    assert(strcmp(looplst->list[1].strval, "\"TWO\"") == 0);
    assert(looplst->list[3].vartype == STRING);
    assert(strcmp(looplst->list[3].strval, "\"3182\"") == 0);
    fclose(file);
    free(looplst);
    free_turtle(res);

    free_prog(prog);
}

void test_lst(void){
    FILE* file = fopen("Testing/LST/correct.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    LOOPLIST* looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    prog->is_var_used[0] = true;
    prog->variables[0].vartype = DOUBLE;
    prog->variables[0].numval = 0.56;
    assert(check_lst(prog, res, looplst) == true);
    assert(looplst->size == 3);
    assert(looplst->list[0].vartype == STRING);
    assert(strcmp(looplst->list[0].strval, "\"sjhf\"") == 0);
    assert(looplst->list[1].vartype == DOUBLE);
    assert(looplst->list[1].numval > 558 && looplst->list[1].numval < 560);
    fclose(file);
    free(looplst);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/LST/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_lst(prog, res, looplst) == false);
    fclose(file);
    free(looplst);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/LST/incorrect2.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_lst(prog, res, looplst) == false);
    fclose(file);
    free(looplst);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/LST/empty.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    assert(check_lst(prog, res, looplst) == true);
    assert(looplst->size == 0);
    fclose(file);
    free(looplst);
    free_turtle(res);

    free_prog(prog);
}

void test_loop(void){
    FILE* file = fopen("Testing/LOOP/correct.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    prog->is_var_used[1] = true;
    prog->variables[1].vartype = DOUBLE;
    prog->variables[1].numval = 0.9;
    assert(check_loop(prog, res) == true);
    assert(prog->is_var_used['M'-'A']);
    assert(prog->variables['M'-'A'].vartype == DOUBLE);
    assert(prog->variables['M'-'A'].numval > 0.8 && prog->variables['M'-'A'].numval < 1.0);
    free_prog(prog);
    free_turtle(res);
    fclose(file);

    file = fopen("Testing/LOOP/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_loop(prog, res) == false);
    free_prog(prog);
    free_turtle(res);
    fclose(file);

    file = fopen("Testing/LOOP/incorrect2.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_loop(prog, res) == false);
    free_prog(prog);
    free_turtle(res);
    fclose(file);

    file = fopen("Testing/LOOP/noloopbody.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    prog->is_var_used[1] = true;
    prog->variables[1].vartype = DOUBLE;
    prog->variables[1].numval = 0.9;
    assert(check_loop(prog, res) == true);
    assert(prog->is_var_used['M'-'A']);
    assert(prog->variables['M'-'A'].vartype == DOUBLE);
    assert(prog->variables['M'-'A'].numval > 0.8 && prog->variables['M'-'A'].numval < 1.0);
    free_prog(prog);
    free_turtle(res);
    fclose(file);

    file = fopen("Testing/LOOP/nested.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    prog->is_var_used[1] = true;
    prog->variables[1].vartype = DOUBLE;
    prog->variables[1].numval = 0.9;
    assert(check_loop(prog, res) == true);
    assert(prog->is_var_used['M'-'A']);
    assert(prog->is_var_used['C'-'A']);
    assert(prog->variables['M'-'A'].vartype == DOUBLE);
    assert(prog->variables['M'-'A'].numval > 0.8 && prog->variables['M'-'A'].numval < 1.0);
    assert(prog->variables['C'-'A'].vartype == DOUBLE);
    assert(prog->variables['C'-'A'].numval > 92738.0 && prog->variables['C'-'A'].numval < 92740.0);
    fclose(file);
    free_turtle(res);

    free_prog(prog);
}

void test_ins(void){
    FILE* file = fopen("Testing/INS/col.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    assert(check_ins(prog, res) == true);
    assert(res->colour == red);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/INS/fwd.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    prog->is_var_used['T'-'A'] = true;
    prog->variables['T'-'A'].vartype = DOUBLE;
    prog->variables['T'-'A'].numval = 8.0;
    assert(check_ins(prog, res) == true);
    assert(res->row > 7.0 && res->row < 9.0);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/INS/rgt.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_ins(prog, res) == true);
    assert(res->angle > 49.0 && res->angle < 51.0);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/INS/loop.ttl", "r");
    prog = get_program(file);
    prog->is_var_used['G'-'A'] = true;
    prog->variables['G'-'A'].vartype = DOUBLE;
    prog->variables['G'-'A'].numval = 8.0;
    res = init_turtle("test_file_for_tests.txt");

    assert(check_ins(prog, res) == true);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/INS/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_ins(prog, res) == false);
    fclose(file);
    free_prog(prog);
    free_turtle(res);

    file = fopen("Testing/INS/incorrect2.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_ins(prog, res) == false);
    fclose(file);

    free_turtle(res);
    free_prog(prog);
}

void test_inslst(void){
    FILE* file = fopen("Testing/INSLST/correct.ttl", "r");
    Program* prog = get_program(file);
    Turtle* res = init_turtle("test_file_for_tests.txt");
    assert(check_inslst(prog, res) == true);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/INSLST/incorrect.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_inslst(prog, res) == false);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/INSLST/emptyfile.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_inslst(prog, res) == false);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/INSLST/empty.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_inslst(prog, res) == true);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/INSLST/multiple.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    prog->is_var_used['L'-'A'] = true;
    prog->variables['L'-'A'].vartype = DOUBLE;
    prog->variables['L'-'A'].numval = 8.0;
    assert(check_inslst(prog, res) == true);
    fclose(file);
    free_turtle(res);
    free_prog(prog);

    file = fopen("Testing/INSLST/noend.ttl", "r");
    prog = get_program(file);
    res = init_turtle("test_file_for_tests.txt");
    assert(check_inslst(prog, res) == false);
    fclose(file);
    free_turtle(res);

    free_prog(prog);
}

void test_process_rgt(void){
    Turtle* res = init_turtle("test_file_for_tests");
    process_rgt(res, 40);
    assert(res->angle > 39 && res->angle < 41);
    process_rgt(res, -40);
    assert(res->angle > -1 && res->angle < 1);
    free_turtle(res);
}

void test_convert_colour_to_char(void){
    char a = convert_colour_to_char((neillcol)black);
    assert(a == 'K');
    a = convert_colour_to_char((neillcol)yellow);
    assert(a == 'Y');
    a = convert_colour_to_char((neillcol)white);
    assert(a == 'W');
    a = convert_colour_to_char((neillcol)green);
    assert(a == 'G');
    a = convert_colour_to_char((neillcol)magenta);
    assert(a == 'M');
}

void test_fetch_colour(void){
    char colour[10];
    strcpy(colour, "\"K\"");
    neillcol val;
    fetch_colour(colour, &val);
    assert(val != black);
    strcpy(colour, "\"BLUE\"");
    fetch_colour(colour, &val);
    assert(val == blue);
    strcpy(colour, "\"CYAN\"");
    fetch_colour(colour, &val);
    assert(val == cyan);
    strcpy(colour, "\"WHITE\"");
    fetch_colour(colour, &val);
    assert(val == white);
}

void test_process_colour(void){
    Turtle* res = init_turtle("test_file_for_tests");
    process_colour(res, black);
    assert(res->colour == black);
    process_colour(res, white);
    assert(res->colour == white);
    process_colour(res, yellow);
    assert(res->colour == yellow);
    process_colour(res, cyan);
    assert(res->colour == cyan);
    process_colour(res, red);
    assert(res->colour == red);
    free_turtle(res);
}

void test_str_to_var(void){
    char var = str_to_var("V");
    assert(var == 'V');
    var = str_to_var("$F");
    assert(var == 'F');
    var = str_to_var("$L");
    assert(var == 'L');
    var = str_to_var("Z");
    assert(var == 'Z');
    var = str_to_var("EINC");
    assert(var == 'A'+26);
    var = str_to_var("$KAP");
    assert(var == 'A'+26);
}

void test_is_validcoord(void){
    assert(is_validcoord(0, 0, 11, 3));
    assert(is_validcoord(10, 0, 11, 3));
    assert(is_validcoord(0, 2, 11, 3));
    assert(is_validcoord(10, 2, 11, 3));
    assert(!is_validcoord(11, 0, 11, 3));
    assert(!is_validcoord(0, 3, 11, 3));
    assert(!is_validcoord(11, 3, 11, 3));
    assert(!is_validcoord(0, -8, 11, 3));
    assert(!is_validcoord(-6, 0, 11, 3));
}

void test_update_stack(void){
    coll* stack = coll_init();
    VAR a, b;
    a.vartype = DOUBLE;
    b.vartype = DOUBLE;
    a.numval = 7.0;
    b.numval = 1.0;
    coll_add(stack, a);
    coll_add(stack, b);
    update_stack(stack, '+');
    assert(stack->size == 1);
    assert(stack->a[stack->size-1].numval > 7.0 && stack->a[stack->size-1].numval < 9.0);
    coll_free(stack);
}

void test_add_to_looplist(void){
    LOOPLIST* looplst = (LOOPLIST*)calloc(1, sizeof(LOOPLIST));
    VAR a;
    a.vartype = STRING;
    strcpy(a.strval, "TESTVAL");
    add_to_looplist(looplst, a);
    assert(looplst->size ==  1);
    assert(looplst->list[0].vartype == STRING);
    assert(strcmp(looplst->list[0].strval, "TESTVAL") == 0);
    VAR b;
    b.vartype = DOUBLE;
    b.numval = 5.0;
    add_to_looplist(looplst, b);
    assert(looplst->size ==  2);
    assert(looplst->list[1].vartype == DOUBLE);
    assert(looplst->list[1].numval > 4.9 && looplst->list[1].numval < 5.1);
    free(looplst);
}



// void test_prog(void){

// }

