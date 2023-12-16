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
        free(res);
        fclose(input_file);
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
    res->colour = white;
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
        VAR var;
        bool is_valid = check_varnum(prog, res, &var);
        if(is_valid){
            // int num = fetch_num(prog, step_pos, res);
            int num = (int)var.numval;
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
        VAR var;
        bool is_valid = check_varnum(prog, res, &var);
        if(is_valid){
            // int num = fetch_num(prog, step_pos, res);
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
        if(check_var(prog, res, &var)){
            neillcol colour;
            bool is_valid = fetch_colour_var(&var, &colour);
            if(!is_valid){
                return false;
            }
            process_colour(res, colour);
            return true;
        }
        prog->curword = original_curword + 1;
        printf("inside col, word = ");
        puts(prog->words[prog->curword]);
        if(check_word(prog, res)){
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
        printf("variable name = %s\n", prog->words[prog->curword-1]);
        char var_name = str_to_var(prog->words[prog->curword-1]);
        prog->is_var_used[var_name-'A'] = true;
        curword = prog->curword;
        if(!strsame(prog->words[curword], "(")){
            prog->curword = original_curword;
            return false;
        }
        prog->curword++;
        VAR val;
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

bool check_varnum(Program* prog, Turtle* res, VAR* num){
    int original_curword = prog->curword;
    bool is_var = check_var(prog, res, num);
    if(is_var){
        return true;
    }
    prog->curword = original_curword;
    bool is_num = check_num(prog, res, num);
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

    prog->curword++;
    return true;
}

bool check_var(Program* prog, Turtle* res, VAR* var){
    int curword = prog->curword;
    int original_curword = curword;
    if(prog->words[curword][0] == '$'){
        bool is_ltr = check_ltr(prog, 1, res);
        if(is_ltr){
            char var_name = str_to_var(prog->words[curword]);
            int pos = var_name - 'A';
            // check if variable has been set before
            if(prog->is_var_used[pos]){
                var->vartype = prog->variables[pos].vartype;
                var->numval = prog->variables[pos].numval;
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
    if(check_op(prog, res, &op)){
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
    printf("curword = %s is not operator\n", prog->words[prog->curword]);
    if(check_varnum(prog, res, var)){
        printf("inside pfix, added %lf to stack\n", var->numval);
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

bool check_num(Program* prog, Turtle* res, VAR* var){
    int curword = prog->curword;
    double num;
    int num_vars = sscanf(prog->words[curword], "%lf", &num);
    if(num_vars != 1){
        return false;
    }
    var->vartype = DOUBLE;
    var->numval = num;
    prog->curword++;
    return  true;
}

bool check_op(Program* prog, Turtle* res, char* op){
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
    VAR var;
    bool is_varnum = check_varnum(prog, res, &var);
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
        printf("num = %i\n", num);
        char colour = convert_colour_to_char(res->colour);
        int multiplier = (num >= 0?1:-1);
        num = (int)abs(num);
        for(int i=0;i<num;i++){
            int x, y;
            printf("angle = %.2lf, cos(angle) = %.2lf, sine(angle) = %.2lf\n", angle, cos(angle), sin(angle));
            res->row = res->row + multiplier*cos(angle);
            res->col = res->col + multiplier*sin(angle);
            y = (int)(res->row);
            x = (int)(res->col);
            // res->row = y;
            // res->col = x;
            printf("coordinates = %i, %i\n", y, x);
            res->matrix[y][x] = colour;
        }
        // // res->row = res->row - cos(angle);
        // // res->col = res->col - sin(angle); 
    }
    else{ // Post Script FILE case

    }
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
        printf("invalid string passed to str_to_val\n");
        return 'A';
    }
}

void set_var(Program* prog, char var_name, VAR* val){
    int pos = var_name - 'A';
    prog->variables[pos].numval = val->numval;
    prog->variables[pos].vartype = val->vartype;
    prog->is_var_used[pos] = true;
    printf("var %c set to %lf\n", var_name, prog->variables[pos].numval);
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
    free(c);
    return true;
}

bool update_stack(coll* stack, char op){
    VAR a;
    VAR b;
    coll_pop(stack, &a);
    coll_pop(stack, &b);
    printf("values on stack = %lf, %lf\n", a.numval, b.numval);
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
        if(a.numval == 0){
            fprintf(stderr, "divide by zero\n");
            return false;
        }
        res = b.numval / a.numval;
    }
    VAR c;
    c.vartype = DOUBLE;
    c.numval = res;
    coll_add(stack, c);
    return true;
}

