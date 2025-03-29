#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include <signal.h>

#define loop while(true)
#define MAX_INPUT 256
#define MAX_ARGCOUNT 256
#define MAX_BG_PROC 5
#define SIGACT_COUNT 2
#define SIGTSTP_KEY 0
#define SIGINT_KEY  1


extern char **environ;

size_t bg_proc_count = 0;
pid_t  bg_procs[MAX_BG_PROC];
pid_t  fg_pid = -1;

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
struct CMD * default_cmd() 
{
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
        fg_pid = pid;
        //В целом работает, но косячит приглашение ввода
        if (cmd->run_in_bg) {
            printf("[-] run in background by PID: %d\n", pid);
            res = pid;
        } else {

            if (!waitpid(fg_pid, NULL, 0)) {
                perror("waitpid error");
                res = -1;
            }
        }
        fg_pid = -1;

    }
    return res;
}

void sigtstp_handler() 
{
    if (fg_pid > 0) {
        printf("\n[-] Stopped %d\n", fg_pid);
        kill(fg_pid, SIGTSTP);
        bg_procs[bg_proc_count++] = fg_pid;
        fg_pid = -1;
    }
}

void sigint_handler() 
{
    if (fg_pid > 0) {
        printf("\nKilled... Good Bye %d :(\n", fg_pid);
        kill(fg_pid, SIGINT);
    }
}

//signal() deprecated
//sigaction is cool, modern -- yeeeaaah
void signals_init(struct sigaction *sa_arr) 
{

    sa_arr[SIGTSTP_KEY].sa_handler = sigtstp_handler;
    sigemptyset(&sa_arr[SIGTSTP_KEY].sa_mask);
    sa_arr[SIGTSTP_KEY].sa_flags = 0;
    sigaction(SIGTSTP, &sa_arr[SIGTSTP_KEY], NULL);

    sa_arr[SIGINT_KEY].sa_handler = sigint_handler;
    sigemptyset(&sa_arr[SIGINT_KEY].sa_mask);
    sa_arr[SIGINT_KEY].sa_flags = 0;
    sigaction(SIGINT, &sa_arr[SIGINT_KEY], NULL);

}

void run_last_bgproc() 
{
    if (bg_proc_count > 0) {
        pid_t pid = bg_procs[--bg_proc_count];
        fg_pid = pid;
        kill(pid, SIGCONT);
        waitpid(pid, NULL, 0);
        fg_pid = -1;
    } else {
        printf("\nNo stopped processes\n");
    }
}

//TODO добавить создание и запись в файл истории выполнения
void show_history() {
    struct CMD *cmd = split_command("cat .minish", " ");
    run_command(cmd);
    free(cmd);
}

void clear_cmdinput(char *cmd_input, size_t size) 
{
    for (size_t i = 0; i < size; ++i) {
        cmd_input[i] = '\0';
    }
}

//TODO надо сделать так, чтобы при нажатии ctrl+z|c не происходило никаких крашей
int main()
{
    const char *prompt = "mini_shell > ";
    char cmd_input[MAX_INPUT];
    clear_cmdinput(cmd_input, MAX_INPUT);
    struct sigaction sa_arr[SIGACT_COUNT];
    signals_init(sa_arr);

    loop {
        printf("%s", prompt);
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

            if (strcmp(cmd->args[0], "exit") == 0) {
                exit(EXIT_SUCCESS);
            }

            if (strcmp(cmd->args[0], "fg") == 0) {
                run_last_bgproc();
                continue;
            }
            if (strcmp(cmd->args[0], "history") == 0) {
                show_history();
                continue;
            }

            pid_t res = run_command(cmd);
            if (res < 0) {
                printf("%d\n", res);
                perror("[x] error run command");
            }
            free(cmd);
        }

        putchar('\n');
        clear_cmdinput(cmd_input, MAX_INPUT);
        free(cmd_seq);
    }

    return 0;
}
