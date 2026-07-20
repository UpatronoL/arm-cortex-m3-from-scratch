static char *private_buffer;

char *mini_strtok(char *buffer) {
    char *toc, *start;
    if(buffer == 0 && private_buffer != 0) {
        toc = start = private_buffer;
    } else {
        toc = start = buffer;
    }
    while (*start != ' ' && *start != '\0') start++;
    if (start == toc) return 0;
    if (*start == ' ') {
        *start = '\0';
        start++;
    }
    private_buffer = start;
    return toc;
}
