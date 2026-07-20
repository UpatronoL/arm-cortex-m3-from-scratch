#include "mini_strcmp.h"

int mini_strcmp(char *a, char *b) {
    while ((*a != '\0') || (*b != '\0')) {
        if (*a != *b) return -1;
        if (*a != '\0') a++;
        if (*b != '\0') b++;
    }
    return 0;
}
