#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

#define loop for(;;)
#define MAX_INPUT 256
#define MAX_ARGCOUNT 256

extern char **environ;

//Атомарная команда. То, что больше нельзя дробить
//Дробить можно по ; или |
//Всё остальное относится к команде
//Дробление по пробелам означает изменение полей структуры
struct CMD 
{
    char  *args[MAX_ARGCOUNT];
    size_t argslen;
    int    in;
    int    out;
    bool   run_in_bg;
};

//Функция-конструктор, чтобы заполнить поля нужными значениями
struct CMD * default_cmd() {
    struct CMD *result = malloc(sizeof(struct CMD));
    result->run_in_bg = false;
    result->in        = STDIN_FILENO;
    result->out       = STDOUT_FILENO;
    
    return result;
}

struct CMD * split_command(char *cmd_input, char *delim) 
{
    if (cmd_input == NULL) {
        fprintf(stderr, "split_command error: input is NULL\n");
        return NULL;
    }

    struct CMD *result = default_cmd();

    if (!result) {
        perror("create CMD error");
        return NULL;
    }

    char *cmd_tok = strtok(cmd_input, delim);
    size_t index = 1;
    

    result->args[0] = cmd_tok;
    while (cmd_tok != NULL && index < MAX_ARGCOUNT - 1) {
        cmd_tok = strtok(NULL, delim);
        result->args[index++] = cmd_tok;
    }
    //Уменьшаем на единицу, чтобы не трогать элемент, идущий за последним аргументом
    result->args[index] = NULL;
    result->argslen     = --index;

    return result;
}


//Выполняет одну атомарную команду
pid_t run_command(struct CMD *cmd) 
{
    pid_t pid = fork();
    pid_t res;

    if (pid < 0) {
        perror("fork error");
        res = -1;
    } else if (pid == 0) {
        execvpe(cmd->args[0], cmd->args, environ);
        perror("exec error");
        _exit(EXIT_FAILURE);
    } else {

        if (!cmd->run_in_bg) {
            int status;
            if (!waitpid(pid, &status, 0)) {
                perror("waitpid error");
                res = -1;
            }
        }

    }
    return res;
}

int main()
{
    //Пока не буду делать разницу между пользователями
    const char *prompt = "mini_shell ->";
    char cmd_input[MAX_INPUT] = "";

    loop {
        printf("%s ", prompt);
        scanf(" %256[^\n\r]", cmd_input);
        
        struct CMD *cmd_seq = split_command(cmd_input, ";");
        if (cmd_seq == NULL) {
            fprintf(stderr, "cmd struct is NULL\n");
        }

        
        for (size_t index = 0; index < cmd_seq->argslen; ++index) {
            struct CMD *cmd = split_command(cmd_seq->args[index], " ");
            if (cmd == NULL) {
                fprintf(stderr, "cmd struct is NULL\n");
            }
            
            pid_t res = run_command(cmd);
            if (res < 0) {
                printf("%d\n", res);
                perror("error run command");
            }
            free(cmd);
        }

        putchar('\n');
        free(cmd_seq);
    }

    return 0;
}
