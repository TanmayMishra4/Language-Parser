#include "parse.h"

int main(int argc, char** argv){
    test();
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
    Program* prog = get_program(file);
    bool is_valid = check_prog(prog);
    free(prog);
    return is_valid;
}

Program* get_program(FILE* file){
    Program* prog = (Program*)calloc(1, sizeof(Program));
    int i = 0;
    while(fscanf(file, "%s", prog->words[i++])==1){
        strip_new_line(prog->words[i]);
    }
    return prog;
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

bool check_word(Program* prog){
    int curword = prog->curword;
    int len = strlen(prog->words[curword]);
    if(prog->words[curword][0] != '"' || prog->words[curword][len-1] != '"'){
        return false;
    }
    if(len == 2){
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
    test_pfix();
    test_set();
    test_item();
    test_items();
    test_lst();
    test_loop();
    test_ins();
    test_inslst();
    test_prog();
}

void test_check_ltr(void){
    FILE* file = fopen("Testing/LTR/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_ltr(prog, 0) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/LTR/digit.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/LTR/smallcase.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/LTR/word.ttl", "r");
    prog = get_program(file);
    assert(check_ltr(prog, 0) == false);
    fclose(file);

    free(prog);
}

void test_check_var(void){
    FILE* file = fopen("Testing/VAR/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_var(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/VAR/digit.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/VAR/smallcase.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/VAR/word.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/VAR/nodollar.ttl", "r");
    prog = get_program(file);
    assert(check_var(prog) == false);
    fclose(file);

    free(prog);
}

void test_check_num(void){
    FILE* file = fopen("Testing/NUM/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_num(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/NUM/bignum.ttl", "r");
    prog = get_program(file);
    assert(check_num(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/NUM/negative.ttl", "r");
    prog = get_program(file);
    assert(check_num(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/NUM/notnum.ttl", "r");
    prog = get_program(file);
    assert(check_num(prog) == false);
    fclose(file);

    free(prog);
}

void test_check_word(void){
    FILE* file = fopen("Testing/WORD/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_word(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/WORD/empty.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/WORD/incomp.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/WORD/notword.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/WORD/number.ttl", "r");
    prog = get_program(file);
    assert(check_word(prog) == true);
    fclose(file);

    free(prog);
}

void test_check_op(void){
    FILE* file = fopen("Testing/OP/add.ttl", "r");
    Program* prog = get_program(file);
    assert(check_op(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/OP/subtract.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/OP/multiply.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/OP/divide.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/OP/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_op(prog) == false);
    fclose(file);

    free(prog);
}

void test_check_varnum(void){
    FILE* file = fopen("Testing/VARNUM/num.ttl", "r");
    Program* prog = get_program(file);
    assert(check_varnum(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/VARNUM/var.ttl", "r");
    prog = get_program(file);
    assert(check_varnum(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/VARNUM/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_varnum(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/VARNUM/incorrect2.ttl", "r");
    prog = get_program(file);
    assert(check_varnum(prog) == false);
    fclose(file);

    free(prog);
}

void test_fwd(void){
    FILE* file = fopen("Testing/FWD/correctnum.ttl", "r");
    Program* prog = get_program(file);
    assert(check_fwd(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/FWD/correctvar.ttl", "r");
    prog = get_program(file);
    assert(check_fwd(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/FWD/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_fwd(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/FWD/mispelled.ttl", "r");
    prog = get_program(file);
    assert(check_fwd(prog) == false);
    fclose(file);

    free(prog);
}

void test_rgt(void){
    FILE* file = fopen("Testing/RGT/correctnum.ttl", "r");
    Program* prog = get_program(file);
    assert(check_rgt(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/RGT/correctvar.ttl", "r");
    prog = get_program(file);
    assert(check_rgt(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/RGT/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_rgt(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/RGT/mispelled.ttl", "r");
    prog = get_program(file);
    assert(check_rgt(prog) == false);
    fclose(file);

    free(prog);
}

void test_col(void){
    FILE* file = fopen("Testing/COL/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_col(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/COL/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_col(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/COL/invalidcol.ttl", "r");
    prog = get_program(file);
    assert(check_col(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/COL/number.ttl", "r");
    prog = get_program(file);
    assert(check_col(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/COL/varcol.ttl", "r");
    prog = get_program(file);
    assert(check_col(prog) == true);
    fclose(file);

    free(prog);
}

void test_pfix(void){
    FILE* file = fopen("Testing/PFIX/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_pfix(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/PFIX/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_pfix(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/PFIX/invalidsymbol.ttl", "r");
    prog = get_program(file);
    assert(check_pfix(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/PFIX/multiple.ttl", "r");
    prog = get_program(file);
    assert(check_pfix(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/PFIX/var.ttl", "r");
    prog = get_program(file);
    assert(check_pfix(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/PFIX/varnum.ttl", "r");
    prog = get_program(file);
    assert(check_pfix(prog) == true);
    fclose(file);

    free(prog);
}

void test_set(void){
    FILE* file = fopen("Testing/SET/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_set(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/SET/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/SET/incorrect2.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/SET/emptyvalue.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/SET/pfix.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/SET/word.ttl", "r");
    prog = get_program(file);
    assert(check_set(prog) == false);
    fclose(file);

    free(prog);
}

void test_item(void){
    FILE* file = fopen("Testing/ITEM/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_item(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/ITEM/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_item(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/ITEM/varnum.ttl", "r");
    prog = get_program(file);
    assert(check_item(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/ITEM/varnum2.ttl", "r");
    prog = get_program(file);
    assert(check_item(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/ITEM/smallcase.ttl", "r");
    prog = get_program(file);
    assert(check_item(prog) == true);
    fclose(file);

    free(prog);
}

void test_items(void){
    FILE* file = fopen("Testing/ITEMS/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_items(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/ITEMS/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_items(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/ITEMS/empty.ttl", "r");
    prog = get_program(file);
    assert(check_items(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/ITEMS/words.ttl", "r");
    prog = get_program(file);
    assert(check_items(prog) == true);
    fclose(file);

    free(prog);
}

void test_lst(void){
    FILE* file = fopen("Testing/LST/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_lst(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/LST/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_lst(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/LST/incorrect2.ttl", "r");
    prog = get_program(file);
    assert(check_lst(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/LST/empty.ttl", "r");
    prog = get_program(file);
    assert(check_lst(prog) == true);
    fclose(file);

    free(prog);
}

void test_loop(void){
    FILE* file = fopen("Testing/LOOP/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_loop(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/LOOP/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_loop(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/LOOP/incorrect2.ttl", "r");
    prog = get_program(file);
    assert(check_loop(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/LOOP/noloopbody.ttl", "r");
    prog = get_program(file);
    assert(check_loop(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/LOOP/nested.ttl", "r");
    prog = get_program(file);
    assert(check_loop(prog) == true);
    fclose(file);

    free(prog);
}

void test_ins(void){
    FILE* file = fopen("Testing/INS/col.ttl", "r");
    Program* prog = get_program(file);
    assert(check_ins(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INS/fwd.ttl", "r");
    prog = get_program(file);
    assert(check_ins(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INS/rgt.ttl", "r");
    prog = get_program(file);
    assert(check_ins(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INS/loop.ttl", "r");
    prog = get_program(file);
    assert(check_ins(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INS/rgt.ttl", "r");
    prog = get_program(file);
    assert(check_ins(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INS/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_ins(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/INS/incorrect2.ttl", "r");
    prog = get_program(file);
    assert(check_ins(prog) == false);
    fclose(file);

    free(prog);
}

void test_inslst(void){
    FILE* file = fopen("Testing/INSLST/correct.ttl", "r");
    Program* prog = get_program(file);
    assert(check_inslst(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INSLST/incorrect.ttl", "r");
    prog = get_program(file);
    assert(check_inslst(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/INSLST/emptyfile.ttl", "r");
    prog = get_program(file);
    assert(check_inslst(prog) == false);
    fclose(file);
    free(prog);

    file = fopen("Testing/INSLST/empty.ttl", "r");
    prog = get_program(file);
    assert(check_inslst(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INSLST/multiple.ttl", "r");
    prog = get_program(file);
    assert(check_inslst(prog) == true);
    fclose(file);
    free(prog);

    file = fopen("Testing/INSLST/noend.ttl", "r");
    prog = get_program(file);
    assert(check_inslst(prog) == false);
    fclose(file);

    free(prog);
}




