#include "interface.h"

extern void button_single_click_cb_up(void *arg, void *usr_data);
extern void button_single_click_cb_down(void *arg, void *usr_data);

void inputFun(char* input, void *arg, void *usr_data){
    char* t;
    for(t=input;*t!='\0';t++){
        for (int i = 0; i < *t - 32; i++)
            button_single_click_cb_down(arg, usr_data);
        button_single_click_cb_up(arg, usr_data);
        usleep(100000);
    }

}