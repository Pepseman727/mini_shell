#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CMD 256
#define MAX_ARGCOUNT 256



//По идее эта шутка должна вернуть массив с командой и с аргументами
//Мб сделать структуру тип cmd??
/*void parse_command(char* cmd_input)  
{
    char* cmd_tok = NULL;
    char* rest = cmd_input;
    
    while((cmd_tok = strtok_r(NULL, " ", &rest))) {
        printf("%s\n", cmd_tok);
    }

}
*/

void parse_command(char* cmd_input) {
    char* cmd_tok = strtok(cmd_input, " ");
    while (cmd_tok != NULL) {
        printf("\t%s\n", cmd_tok);
        cmd_tok = strtok(NULL, " ");
    }
    

}


int main()
{
    
    //Пока не буду делать разницу между пользователями
    const char* prompt = "Enter ur command ->";
    char cmd_input[MAX_CMD] = "";
    
    //Пока пиши как получается, потом отрефакторишь
    for(;;) {
        printf("%s ", prompt);
        scanf(" %256[^\n\r]", cmd_input);

        parse_command(cmd_input);
        
        //Проблема в том, что на одной итерации, он считывает только одно слово, после этого он переходит на другую.
        //В итоге забывается предыдущая опция => Надо как-то считать всю строку так, чтоб она не перетиралась и осталось всё в одной итерации

        
    }

    return 0;
}