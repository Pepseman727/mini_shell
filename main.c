#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>


#define loop for(;;)
#define MAX_CMD 256
#define MAX_ARGCOUNT 256

extern char **environ;

struct CMD 
{
    char *args[MAX_ARGCOUNT];
    //Как будто и без неё всё работает, но вдруг компилятор не будет делать null для ячеек массива, которые не использовались
    size_t argslen;
};

struct CMD * parse_command(char *cmd_input) 
{
    if (cmd_input == NULL) {
        fprintf(stderr, "parse_command error: input is NULL\n");
        return NULL;
    }

    char *cmd_tok = strtok(cmd_input, " ");
    size_t index = 1;
    struct CMD *result = malloc(sizeof(struct CMD));
    if (!result) {
        perror("malloc error");
        return NULL;
    }

    result->args[0] = cmd_tok;
    while (cmd_tok != NULL && index < MAX_ARGCOUNT - 1) {
        cmd_tok = strtok(NULL, " ");
        result->args[index++] = cmd_tok;
    }
    //Уменьшаем на единицу, чтобы не трогать элемент, идущий за последним аргументом
    result->args[index] = NULL;
    result->argslen = index;

    return result;
}

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
        int status;
        if (!waitpid(pid, &status, 0)) {
            perror("waitpid error");
            res = -1;
        }
    }
    return res;
}

int main()
{
    //Пока не буду делать разницу между пользователями
    const char *prompt = "mini_shell ->";
    char cmd_input[MAX_CMD] = "";

    loop {
        printf("%s ", prompt);
        scanf(" %256[^\n\r]", cmd_input);
        
        struct CMD *cmd = parse_command(cmd_input);
        if (cmd == NULL) {
            fprintf(stderr, "cmd struct is NULL\n");
        }

        pid_t res = run_command(cmd);
        if (res < 0) {
            printf("%d\n", res);
            perror("error run command");
        }

        putchar('\n');
        free(cmd);
    }

    return 0;
}
