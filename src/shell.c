#include "shell.h"
#include "mini_printf.h"
#include "mini_strtok.h"
#include "mini_strcmp.h"

Handler command_handler[] = { 
    {"Led", &LedOnOff},
    {"Pullup", &PullupOnOff}
};

const int handler_size = sizeof(command_handler)/sizeof(command_handler[0]);

void LedOnOff(void) {
    char *tok = mini_strtok(0);
    if (tok == 0) {
        mini_printf ("Argument Error: No arguments where given\r");
        return;
    }
    if (!mini_strcmp("ON", tok)) mini_printf("Led ON\r");
    else if (!mini_strcmp("OFF", tok)) mini_printf("Led OFF\r");
    else mini_printf("Argument Error: Led <arg ON/OFF>\r");
}

void PullupOnOff(void) {
    char *tok = mini_strtok(0);
    if (tok == 0) {
        mini_printf ("Argument Error: No arguments where given\r");
        return;
    }
    if (!mini_strcmp("ON", tok)) mini_printf("Pullup ON\r");
    else if (!mini_strcmp("OFF", tok)) mini_printf("Pullup OFF\r");
    else mini_printf("Argument Error: Pullup <arg ON/OFF>\r");
}
