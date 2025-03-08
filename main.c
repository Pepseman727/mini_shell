#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//По идее эта шутка должна вернуть массив с командой и с аргументами
//Мб сделать структуру тип cmd??
void make_cmd() {

}


int main() {

    //Пока не буду делать разницу между пользователями
    const char* prompt = "->";
    char cmd_input[256] = "";
    char* cmd_tokenized = NULL;

    //Пока пиши как получается, потом отрефакторишь
    while (true) {

        printf("Enter ur command %s ", prompt);
        scanf("%256s", cmd_input);
        
        //Проблема в том, что на одной итерации, он считывает только одно слово, после этого он переходит на другую.
        //В итоге забывается предыдущая опция =>Надо как-то считать всю строку так, чтоб она не перетиралась и осталось всё в одной итерации

        
    }



    return 0;
}