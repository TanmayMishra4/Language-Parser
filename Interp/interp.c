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
    int curword = prog->curword;
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
        for(int i=0;i<loop_lst.size;i++){
            VAR v = loop_lst.list[i];
            if(v.vartype == STRING){
                // printf("%s ", v.strval);
            }
            else{
                // printf("%lf ", v.numval);
            }
        }
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
    // printf("inside lst curword = %i, word = ", curword);
    // puts(prog->words[curword]);
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
        int prevx = (int)res->col;
        int prevy = (int)res->row;
        neillfgcol(res->colour);
        for(int i=0;i<(int)num;i++){
            int x, y;
            // printf("angle = %.2lf, cos(angle) = %.2lf, sine(angle) = %.2lf\n", angle, cos(angle), sin(angle));   
            y = (int)(res->row);
            x = (int)(res->col);
            int diffy = y - prevy;
            int diffx = x - prevx;
            
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
        return 'A';
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
        strcpy(c->a[size].strval, d.strval);
        c->size = c->size + 1;
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
    while(fscanf(file, "%s", prog->words[i++])==1){
        strip_new_line(prog->words[i]);
    }
    return prog;
}

// TESTING FUNCTIONS

void test(void){
    test_check_ltr();
    test_check_var();
    // test_check_num();
    // test_check_word();
    // test_check_op();
    // test_check_varnum();
    // test_fwd();
    // test_rgt();
    // test_col();
    // test_pfix();
    // test_set();
    // test_item();
    // test_items();
    // test_lst();
    // test_loop();
    // test_ins();
    // test_inslst();
    // test_prog();
}

void test_check_ltr(void){
    FILE* file = fopen("Testing/LTR/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_ltr(prog, 0) == true);
    fclose(file);

    file = fopen("Testing/LTR/digit.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);

    file = fopen("Testing/LTR/smallcase.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);

    file = fopen("Testing/LTR/word.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);

    free(prog);
}

void test_check_var(void){
    FILE* file = fopen("Testing/VAR/correct.ttl", "r");
    VAR var;
    Program* prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);


    file = fopen("Testing/VAR/digit.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);

    Turtle* res = init_turtle("test_file_for_tests.txt");
    file = fopen("Testing/VAR/setanduse.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog, res));
    assert(check_var(prog, &var) == true);
    assert(var.vartype == DOUBLE);
    assert(var.numval > 4.9 && var.numval < 5.1);
    fclose(file);
    free_turtle(res);

    res = init_turtle("test_file_for_tests.txt");
    file = fopen("Testing/VAR/setanduse2.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog, res));
    assert(check_var(prog, &var) == true);
    assert(var.vartype == DOUBLE);
    assert(var.numval > 11.9 && var.numval < 12.1);
    fclose(file);
    free_turtle(res);

    file = fopen("Testing/VAR/smallcase.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);

    file = fopen("Testing/VAR/word.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);

    file = fopen("Testing/VAR/nodollar.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog, &var) == false);
    fclose(file);

    free(prog);
}

// void test_check_num(void){
//     FILE* file = fopen("Testing/NUM/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_num(prog) == true);
//     fclose(file);

//     file = fopen("Testing/NUM/bignum.ttl", "r");
//     prog = get_program(file);
//     assert(check_num(prog) == true);
//     fclose(file);

//     file = fopen("Testing/NUM/negative.ttl", "r");
//     prog = get_program(file);
//     assert(check_num(prog) == true);
//     fclose(file);

//     file = fopen("Testing/NUM/notnum.ttl", "r");
//     prog = get_program(file);
//     assert(check_num(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_check_word(void){
//     FILE* file = fopen("Testing/WORD/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_word(prog) == true);
//     fclose(file);

//     file = fopen("Testing/WORD/empty.ttl", "r");
//     prog = get_program(file);
//     assert(check_word(prog) == false);
//     fclose(file);

//     file = fopen("Testing/WORD/incomp.ttl", "r");
//     prog = get_program(file);
//     assert(check_word(prog) == false);
//     fclose(file);

//     file = fopen("Testing/WORD/notword.ttl", "r");
//     prog = get_program(file);
//     assert(check_word(prog) == false);
//     fclose(file);

//     file = fopen("Testing/WORD/number.ttl", "r");
//     prog = get_program(file);
//     assert(check_word(prog) == true);
//     fclose(file);

//     free(prog);
// }

// void test_check_op(void){
//     FILE* file = fopen("Testing/OP/add.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_op(prog) == true);
//     fclose(file);

//     file = fopen("Testing/OP/subtract.ttl", "r");
//     prog = get_program(file);
//     assert(check_op(prog) == true);
//     fclose(file);

//     file = fopen("Testing/OP/multiply.ttl", "r");
//     prog = get_program(file);
//     assert(check_op(prog) == true);
//     fclose(file);

//     file = fopen("Testing/OP/divide.ttl", "r");
//     prog = get_program(file);
//     assert(check_op(prog) == true);
//     fclose(file);

//     file = fopen("Testing/OP/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_op(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_check_varnum(void){
//     FILE* file = fopen("Testing/VARNUM/num.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_varnum(prog) == true);
//     fclose(file);

//     file = fopen("Testing/VARNUM/var.ttl", "r");
//     prog = get_program(file);
//     assert(check_varnum(prog) == true);
//     fclose(file);

//     file = fopen("Testing/VARNUM/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_varnum(prog) == false);
//     fclose(file);

//     file = fopen("Testing/VARNUM/incorrect2.ttl", "r");
//     prog = get_program(file);
//     assert(check_varnum(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_fwd(void){
//     FILE* file = fopen("Testing/FWD/correctnum.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_fwd(prog) == true);
//     fclose(file);

//     file = fopen("Testing/FWD/correctvar.ttl", "r");
//     prog = get_program(file);
//     assert(check_fwd(prog) == true);
//     fclose(file);

//     file = fopen("Testing/FWD/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_fwd(prog) == false);
//     fclose(file);

//     file = fopen("Testing/FWD/mispelled.ttl", "r");
//     prog = get_program(file);
//     assert(check_fwd(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_rgt(void){
//     FILE* file = fopen("Testing/RGT/correctnum.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_rgt(prog) == true);
//     fclose(file);

//     file = fopen("Testing/RGT/correctvar.ttl", "r");
//     prog = get_program(file);
//     assert(check_rgt(prog) == true);
//     fclose(file);

//     file = fopen("Testing/RGT/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_rgt(prog) == false);
//     fclose(file);

//     file = fopen("Testing/RGT/mispelled.ttl", "r");
//     prog = get_program(file);
//     assert(check_rgt(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_col(void){
//     FILE* file = fopen("Testing/COL/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_col(prog) == true);
//     fclose(file);

//     file = fopen("Testing/COL/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_col(prog) == false);
//     fclose(file);

//     file = fopen("Testing/COL/invalidcol.ttl", "r");
//     prog = get_program(file);
//     assert(check_col(prog) == true);
//     fclose(file);

//     file = fopen("Testing/COL/number.ttl", "r");
//     prog = get_program(file);
//     assert(check_col(prog) == false);
//     fclose(file);

//     file = fopen("Testing/COL/varcol.ttl", "r");
//     prog = get_program(file);
//     assert(check_col(prog) == true);
//     fclose(file);

//     free(prog);
// }

// void test_pfix(void){
//     FILE* file = fopen("Testing/PFIX/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_pfix(prog) == true);
//     fclose(file);

//     file = fopen("Testing/PFIX/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_pfix(prog) == false);
//     fclose(file);

//     file = fopen("Testing/PFIX/invalidsymbol.ttl", "r");
//     prog = get_program(file);
//     assert(check_pfix(prog) == false);
//     fclose(file);

//     file = fopen("Testing/PFIX/multiple.ttl", "r");
//     prog = get_program(file);
//     assert(check_pfix(prog) == true);
//     fclose(file);

//     file = fopen("Testing/PFIX/var.ttl", "r");
//     prog = get_program(file);
//     assert(check_pfix(prog) == true);
//     fclose(file);

//     file = fopen("Testing/PFIX/varnum.ttl", "r");
//     prog = get_program(file);
//     assert(check_pfix(prog) == true);
//     fclose(file);

//     free(prog);
// }

// void test_set(void){
//     FILE* file = fopen("Testing/SET/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_set(prog) == true);
//     fclose(file);

//     file = fopen("Testing/SET/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_set(prog) == false);
//     fclose(file);

//     file = fopen("Testing/SET/incorrect2.ttl", "r");
//     prog = get_program(file);
//     assert(check_set(prog) == false);
//     fclose(file);

//     file = fopen("Testing/SET/emptyvalue.ttl", "r");
//     prog = get_program(file);
//     assert(check_set(prog) == true);
//     fclose(file);

//     file = fopen("Testing/SET/pfix.ttl", "r");
//     prog = get_program(file);
//     assert(check_set(prog) == true);
//     fclose(file);

//     file = fopen("Testing/SET/word.ttl", "r");
//     prog = get_program(file);
//     assert(check_set(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_item(void){
//     // add test case for checking num where num = 563yt
//     FILE* file = fopen("Testing/ITEM/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_item(prog) == true);
//     fclose(file);

//     file = fopen("Testing/ITEM/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_item(prog) == false);
//     fclose(file);

//     file = fopen("Testing/ITEM/varnum.ttl", "r");
//     prog = get_program(file);
//     assert(check_item(prog) == true);
//     fclose(file);

//     file = fopen("Testing/ITEM/varnum2.ttl", "r");
//     prog = get_program(file);
//     assert(check_item(prog) == true);
//     fclose(file);

//     file = fopen("Testing/ITEM/smallcase.ttl", "r");
//     prog = get_program(file);
//     assert(check_item(prog) == true);
//     fclose(file);

//     free(prog);
// }

// void test_items(void){
//     FILE* file = fopen("Testing/ITEMS/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_items(prog) == true);
//     fclose(file);

//     file = fopen("Testing/ITEMS/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_items(prog) == false);
//     fclose(file);

//     file = fopen("Testing/ITEMS/empty.ttl", "r");
//     prog = get_program(file);
//     assert(check_items(prog) == true);
//     fclose(file);

//     file = fopen("Testing/ITEMS/words.ttl", "r");
//     prog = get_program(file);
//     assert(check_items(prog) == true);
//     fclose(file);

//     free(prog);
// }

// void test_lst(void){
//     FILE* file = fopen("Testing/LST/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_lst(prog) == true);
//     fclose(file);

//     file = fopen("Testing/LST/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_lst(prog) == false);
//     fclose(file);

//     file = fopen("Testing/LST/incorrect2.ttl", "r");
//     prog = get_program(file);
//     assert(check_lst(prog) == false);
//     fclose(file);

//     file = fopen("Testing/LST/empty.ttl", "r");
//     prog = get_program(file);
//     assert(check_lst(prog) == true);
//     fclose(file);

//     free(prog);
// }

// void test_loop(void){
//     FILE* file = fopen("Testing/LOOP/correct.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_loop(prog) == true);
//     fclose(file);

//     file = fopen("Testing/LOOP/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_loop(prog) == false);
//     fclose(file);

//     file = fopen("Testing/LOOP/incorrect2.ttl", "r");
//     prog = get_program(file);
//     assert(check_loop(prog) == false);
//     fclose(file);

//     file = fopen("Testing/LOOP/noloopbody.ttl", "r");
//     prog = get_program(file);
//     assert(check_loop(prog) == true);
//     fclose(file);

//     file = fopen("Testing/LOOP/nested.ttl", "r");
//     prog = get_program(file);
//     assert(check_loop(prog) == true);
//     fclose(file);

//     free(prog);
// }

// void test_ins(void){
//     FILE* file = fopen("Testing/INS/col.ttl", "r");
//     Program* prog = get_program(file);
//     assert(check_ins(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INS/fwd.ttl", "r");
//     prog = get_program(file);
//     assert(check_ins(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INS/rgt.ttl", "r");
//     prog = get_program(file);
//     assert(check_ins(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INS/loop.ttl", "r");
//     prog = get_program(file);
//     assert(check_ins(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INS/rgt.ttl", "r");
//     prog = get_program(file);
//     assert(check_ins(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INS/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_ins(prog) == false);
//     fclose(file);

//     file = fopen("Testing/INS/incorrect2.ttl", "r");
//     prog = get_program(file);
//     assert(check_ins(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_inslst(void){
//     FILE* file = fopen("Testing/LST/correct.ttl", "r");
//     Program* prog = get_program(file);
//     // assert(check_inslst(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INSLST/incorrect.ttl", "r");
//     prog = get_program(file);
//     assert(check_inslst(prog) == false);
//     fclose(file);

//     file = fopen("Testing/INSLST/emptyfile.ttl", "r");
//     prog = get_program(file);
//     assert(check_inslst(prog) == false);
//     fclose(file);

//     file = fopen("Testing/INSLST/empty.ttl", "r");
//     prog = get_program(file);
//     assert(check_inslst(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INSLST/multiple.ttl", "r");
//     prog = get_program(file);
//     assert(check_inslst(prog) == true);
//     fclose(file);

//     file = fopen("Testing/INSLST/noend.ttl", "r");
//     prog = get_program(file);
//     assert(check_inslst(prog) == false);
//     fclose(file);

//     free(prog);
// }

// void test_prog(void){

// }

