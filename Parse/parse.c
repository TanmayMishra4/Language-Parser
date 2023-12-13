#include "parse.h"

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr, "Wrong number of params: Correct usage - ./parse [filename]\n");
        exit(EXIT_FAILURE);
    }
    char* file_name = argv[1];
    FILE* input_file = fopen(file_name, "r");
    if(input_file == NULL){
        fprintf(stderr, "Cannot open file\n");
        exit(EXIT_FAILURE);
    }
    bool is_valid = parse_file(input_file);
    fclose(input_file);
    if(is_valid){
        return 0;
    }
    else{
        return 1;
    }
}

bool parse_file(FILE* file){
    Program* prog = (Program*)calloc(1, sizeof(Program));
    int i = 0;
    while(fscanf(file, "%s", prog->words[i++])==1){
        strip_new_line(prog->words[i]);
    }
    bool is_valid = check_prog(prog);
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

bool check_prog(Program* prog){
    int curword = prog->curword;
    if(!strsame(prog->words[curword], "START")){
        return false;
    }
    prog->curword++;
    return check_inslst(prog);
}

bool check_inslst(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "END")){
        prog->curword++;
        return true;
    }
    else{
        if(check_ins(prog)){
            if(check_inslst(prog)){
                return true;
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
    }
}

bool check_ins(Program* prog){
    int curword = prog->curword;
    if(check_fwd(prog)){
        return true;
    }
    else if(check_rgt(prog)){
        return true;
    }
    else if(check_col(prog)){
        return true;
    }
    else if(check_loop(prog)){
        return true;
    }
    else if(check_set(prog)){
        return true;
    }
    else{
        return false;
    }
}

bool check_fwd(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "FORWARD")){
        prog->curword++;
        return check_varnum(prog);
    }
    else{
        return false;
    }
}

bool check_rgt(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "RIGHT")){
        prog->curword++;
        return check_varnum(prog);
    }
    else{
        return false;
    }
}

bool check_col(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "COLOUR")){
        prog->curword++;
        if(check_var(prog)){
            return true;
        }
        else if(check_word(prog)){
            return true;
        }
        return false;
    }
    else{
        return false;
    }
}

bool check_loop(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "LOOP")){
        prog->curword++;
        if(!check_ltr(prog)){
            return false;
        }
        curword = prog->curword;
        if(!strsame(prog->words[curword], "OVER")){
            return false;
        }
        prog->curword++;
        if(!check_lst(prog)){
            return false;
        }
        if(!check_inslst(prog)){
            return false;
        }
        return true;
    }
    else{
        return false;
    }
}

bool check_set(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "SET")){
        prog->curword++;
        if(!check_ltr(prog)){
            return false;
        }
        curword = prog->curword;
        if(!strsame(prog->words[curword], "(")){
            return false;
        }
        prog->curword++;
        if(!check_pfix(prog)){
            return false;
        }
        return true;
    }
    else{
        return false;
    }
}

bool check_varnum(Program* prog){
    if(check_var(prog)){
        return true;
    }
    else if(check_num(prog)){
        return true;
    }
    return false;
}
// TODO: Complete fully, only partially working now
bool check_word(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "RED")){
        prog->curword++;
        return true;
    }
    else if(strsame(prog->words[curword], "GREEN")){
        prog->curword++;
        return true;
    }
    else if(strsame(prog->words[curword], "YELLOW")){
        prog->curword++;
        return true;
    }
    else if(strsame(prog->words[curword], "CYAN")){
        prog->curword++;
        return true;
    }
    else if(strsame(prog->words[curword], "MAGENTA")){
        prog->curword++;
        return true;
    }
    return false;
    
}

bool check_var(Program* prog){
    int curword = prog->curword;
    if(prog->words[curword][0] == '$'){
        return check_ltr(prog);
    }
    return false;
}

bool check_pfix(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], ")")){
        prog->curword++;
        return true;
    }
    else if(check_op(prog)){
        return check_pfix(prog);
    }
    else if(check_varnum(prog)){
        return check_pfix(prog);
    }
    else{
        return false;
    }
}

bool check_ltr(Program* prog){
    int curword = prog->curword;
    int len = strlen(prog->words[curword]);
    if(len != 2){
        return false;
    }
    char letter = prog->words[curword][1];
    if(letter >=  'A' && letter <= 'Z'){
        prog->curword++;
        return true;
    }
    return false;
}

bool check_lst(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "{")){
        prog->curword++;
        return check_items(prog);
    }
    else{
        return false;
    }
}

bool check_num(Program* prog){
    int curword = prog->curword;
    double num;
    int num_vars = sscanf(prog->words[curword], "%lf", &num);
    if(num_vars != 1){
        return false;
    }
    prog->curword++;
    return  true;
}

bool check_op(Program* prog){
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

bool check_items(Program* prog){
    int curword = prog->curword;
    if(strsame(prog->words[curword], "}")){
        prog->curword++;
        return true;
    }
    else if(check_item(prog)){
        return check_items(prog);
    }
    else{
        return false;
    }
}

bool check_item(Program* prog){
    if(check_varnum(prog)){
        return true;
    }
    else if(check_word(prog)){
        return true;
    }
    return false;
}


