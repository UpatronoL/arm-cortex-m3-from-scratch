typedef void (*pfnHandler)(void);

typedef struct Command_Handler {
    char *name;
    pfnHandler function; 
} Handler;

void LedOnOff(void);
void PullupOnOff(void);

extern Handler command_handler[];
extern const int handler_size;
