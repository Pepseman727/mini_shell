#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CMD 256
#define MAX_ARGCOUNT 256



//По идее эта шутка должна вернуть массив с командой и с аргументами
//Мб сделать структуру тип cmd??
struct CMD 
{
    char* command;
    char* args[MAX_ARGCOUNT];
    //Как будто и без неё всё работает, но вдруг компилятор не будет делать null для ячеек массива, которые не использовались
    size_t argslen;
};

struct CMD* parse_command(char* cmd_input) {
    struct CMD* result = malloc(sizeof(struct CMD));
    char* cmd_tok = strtok(cmd_input, " ");
    size_t index = 0;

    result->command = cmd_tok;
    while (cmd_tok != NULL) {
        
        cmd_tok = strtok(NULL, " ");
        result->args[index] = cmd_tok;
        ++index;
    }
    //Уменьшаем на единицу, чтобы не трогать элемент, идущий за последним аргументом
    result->argslen = --index;
    return result;

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

        struct CMD* cmd = parse_command(cmd_input);
        
        printf("%s\n", cmd->command);
        for (size_t index = 0; index < cmd->argslen; ++index) {
            printf("%s ", cmd->args[index]);

        }
        printf("\n");
        free(cmd);
    }

    return 0;
}