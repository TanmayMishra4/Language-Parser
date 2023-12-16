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
        printf("Parsed OK\n");
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
    int original_curword = curword;
    if(!strsame(prog->words[curword], "START")){
        return false;
    }
    prog->curword++;
    bool is_inslst = check_inslst(prog);
    if(is_inslst){
        return true;
    }
    else{
        prog->curword = original_curword;
        return false;
    }
}

bool check_inslst(Program* prog){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "END")){
        prog->curword++;
        return true;
    }
    else{
        bool is_ins = check_ins(prog);
        if(is_ins){
            bool is_inslst = check_inslst(prog);
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

bool check_ins(Program* prog){
    int original_curword = prog->curword;
    if(check_fwd(prog)){
        return true;
    }
    prog->curword = original_curword;
    if(check_rgt(prog)){
        return true;
    }
    prog->curword = original_curword;
    if(check_col(prog)){
        return true;
    }
    prog->curword = original_curword;
    if(check_loop(prog)){
        return true;
    }
    prog->curword = original_curword;
    if(check_set(prog)){
        return true;
    }
    else{
        prog->curword = original_curword;
        return false;
    }
}

bool check_fwd(Program* prog){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "FORWARD")){
        prog->curword++;
        bool is_valid = check_varnum(prog);
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

bool check_rgt(Program* prog){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "RIGHT")){
        prog->curword++;
        bool is_valid = check_varnum(prog);
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

bool check_col(Program* prog){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "COLOUR")){
        prog->curword++;
        if(check_var(prog)){
            return true;
        }
        prog->curword = original_curword + 1;
        if(check_word(prog)){
            return true;
        }
        prog->curword = original_curword;
        return false;
    }
    else{
        return false;
    }
}

bool check_loop(Program* prog){
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
        if(!strsame(prog->words[curword], "OVER")){
            prog->curword = original_curword;
            return false;
        }
        prog->curword++;
        if(!check_lst(prog)){
            prog->curword = original_curword;
            return false;
        }
        // prog-
        if(!check_inslst(prog)){
            prog->curword = original_curword;
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
    int original_curword = curword;
    if(strsame(prog->words[curword], "SET")){
        prog->curword++;
        if(!check_ltr(prog, 0)){
            prog->curword = original_curword;
            return false;
        }
        curword = prog->curword;
        if(!strsame(prog->words[curword], "(")){
            prog->curword = original_curword;
            return false;
        }
        prog->curword++;
        if(!check_pfix(prog)){
            prog->curword = original_curword;
            return false;
        }
        return true;
    }
    else{
        return false;
    }
}

bool check_varnum(Program* prog){
    int original_curword = prog->curword;
    bool is_var = check_var(prog);
    if(is_var){
        return true;
    }
    prog->curword = original_curword;
    bool is_num = check_num(prog);
    if(is_num){
        return true;
    }
    prog->curword = original_curword;
    return false;
}
// TODO: Complete fully, only partially working now
bool check_word(Program* prog){
    int curword = prog->curword;
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

bool check_var(Program* prog){
    int curword = prog->curword;
    int original_curword = curword;
    if(prog->words[curword][0] == '$'){
        bool is_ltr = check_ltr(prog, 1);
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

bool check_pfix(Program* prog){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], ")")){
        prog->curword++;
        return true;
    }
    else if(check_op(prog)){
        bool is_valid = check_pfix(prog);
        if(is_valid){
            return true;
        }
        else{
            prog->curword = original_curword;
            return false;
        }
    }
    else if(check_varnum(prog)){
        bool is_valid = check_pfix(prog);
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
// TODO : see for $A and A 
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

bool check_lst(Program* prog){
    int curword = prog->curword;
    int original_curword = curword;
    if(strsame(prog->words[curword], "{")){
        prog->curword++;
        bool is_valid = check_items(prog);
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
    int original_curword = curword;
    if(strsame(prog->words[curword], "}")){
        prog->curword++;
        return true;
    }
    bool is_valid = check_item(prog);
    if(is_valid){
        bool is_items = check_items(prog);
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

bool check_item(Program* prog){
    int original_curword = prog->curword;
    bool is_varnum = check_varnum(prog);
    if(is_varnum){
        return true;
    }
    prog->curword = original_curword;
    bool is_word = check_word(prog);
    if(is_word){
        return true;
    }
    prog->curword = original_curword;
    return false;
}

void copy_word_from_str(char* word, char* str){
    int index = 1;
    while(str[index] != '"'){
        word[index-1] = str[index];
        index++;
    }
}

