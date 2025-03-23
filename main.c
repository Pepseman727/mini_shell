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
    result->in         = STDIN_FILENO;
    result->out        = STDOUT_FILENO;
    result->run_in_bg  = false;
    
    return result;
}

/*Название не совсем подходит, так как тут помимо разбиения происходит 
    что-то типо парсинга
*/



struct CMD * split_command(char *cmd_input, char *delim) 
{
    if (cmd_input == NULL) {
        fprintf(stderr, "[x] split_command error: input is NULL\n");
        return NULL;
    }

    struct CMD *result_cmd = default_cmd();

    if (!result_cmd) {
        perror("[x] create CMD error");
        return NULL;
    }

    char *cmd_tok = strtok(cmd_input, delim);
    size_t index = 1;
    
    if (strchr(cmd_tok, '&') !=NULL && (strlen(cmd_tok) != 1)) {
        result_cmd->run_in_bg = true;
        //Мб условие гавно
    } else if (strchr(cmd_tok, '&') !=NULL && (strlen(cmd_tok) == 1)){
        fprintf(stderr, "[x] Syntax error. Incorrect command: %s\n", cmd_tok);
        return NULL;
    }


    result_cmd->args[0] = cmd_tok;
    while (cmd_tok != NULL && index < MAX_ARGCOUNT - 1) {
        cmd_tok = strtok(NULL, delim);
        result_cmd->args[index++] = cmd_tok;
    }
    
    result_cmd->args[index] = NULL;
    //Уменьшаем на единицу, чтобы не трогать элемент, идущий за последним аргументом
    result_cmd->argslen     = --index;

    return result_cmd;
}


void set_bgrun(struct CMD *cmd)
{
    //Какой-то хуйни накрутил
    for (size_t index = 0; index < cmd->argslen; ++index) {
        char *tmp = cmd->args[index];
        char *bg_symb = strchr(tmp, '&');
        
        if (!strcmp(tmp, "&")) {
            cmd->args[index] = NULL;
            cmd->argslen -=1;
            cmd->run_in_bg = true;
        } else if (bg_symb) {
            size_t bg_i = bg_symb - tmp;
            tmp[bg_i] = '\0';
            cmd->run_in_bg = true;
        }
    }

}

//Выполняет одну атомарную команду
pid_t run_command(struct CMD *cmd) 
{
    pid_t pid = fork();
    pid_t res = 0;

    if (pid < 0) {
        perror("[x] fork error");
        res = -1;
    } else if (pid == 0) {
        execvpe(cmd->args[0], cmd->args, environ);
        perror("[x] exec error");
        _exit(EXIT_FAILURE);
    } else {

        //В целом работает, но косячит приглашение ввода
        if (cmd->run_in_bg) {
            printf("[-] run in background by PID: %d\n", pid);
            res = pid;
        } else {
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
            fprintf(stderr, "[x] cmd seq struct is NULL\n");
            continue;
        }

        
        for (size_t index = 0; index < cmd_seq->argslen; ++index) {
            struct CMD *cmd = split_command(cmd_seq->args[index], " ");

            set_bgrun(cmd);

            if (cmd == NULL) {
                fprintf(stderr, "[x] cmd struct is NULL\n");
            }

            pid_t res = run_command(cmd);
            if (res < 0) {
                printf("%d\n", res);
                perror("[x] error run command");
            }
            free(cmd);
        }

        putchar('\n');
        free(cmd_seq);
    }

    return 0;
}
