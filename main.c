#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CMD 256
#define MAX_ARGCOUNT 256

struct CMD 
{
    //Мб command вообще не нужен...
    //char* command;
    char* args[MAX_ARGCOUNT];
    //Как будто и без неё всё работает, но вдруг компилятор не будет делать null для ячеек массива, которые не использовались
    size_t argslen;
};

struct CMD* parse_command(char* cmd_input) {
    struct CMD* result = malloc(sizeof(struct CMD));
    char* cmd_tok = strtok(cmd_input, " ");
    size_t index = 1;

    result->args[0] = cmd_tok;
    //result->command = cmd_tok;
    while (cmd_tok != NULL) {
        cmd_tok = strtok(NULL, " ");
        result->args[index] = cmd_tok;
        ++index;
    }
    //Уменьшаем на единицу, чтобы не трогать элемент, идущий за последним аргументом
    result->args[index] = NULL;
    result->argslen = --index;
    return result;

}

//Не совсем уверен, что правильно выхожу из функции
pid_t run_command(struct CMD* cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork error\n");
        return -1;
    } else if (pid == 0) {
        char* path_env = getenv("PATH");
        char* envp[] = {path_env, NULL};
        int res = execvpe(cmd->args[0], cmd->args, envp);
        exit(res);
    } else {
        pid_t chld_pid = wait(NULL);
        return chld_pid;
    }
}

int main()
{
    //Пока не буду делать разницу между пользователями
    const char* prompt = "mini_shell ->";
    char cmd_input[MAX_CMD] = "";
    
    //Пока пиши как получается, потом отрефакторишь
    for(;;) {
        printf("%s ", prompt);
        scanf(" %256[^\n\r]", cmd_input);
        struct CMD* cmd = parse_command(cmd_input);
        run_command(cmd);
        printf("\n");
        free(cmd);
    }

    return 0;
}