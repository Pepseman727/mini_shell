#include <stdio.h>
#include <stdbool.h>
#include <string.h>


int main() {

    //Пока не буду делать разницу между пользователями
    const char* prompt = "->";
    char cmd_input[256] = "";
    char* cmd_tokenized = NULL;

    //Пока пиши как получается, потом отрефакторишь
    while (true) {
        //Как-то это по-уебански выглядит
        char cmd[256] = "";
        char* arguments[256][256] = 
        printf("Enter ur command %s ", prompt);
        scanf("%256s", cmd_input);
        cmd_tokenized = strtok(cmd_input, " ");
        while (cmd_tokenized != NULL) {
            
            cmd_tokenized = strtok(NULL, " ");
        }
    }



    return 0;
}