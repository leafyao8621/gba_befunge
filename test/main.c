#include "../logic.h"
#include <stdio.h>
#include <string.h>

int randint(int a, int b) {return 0;}
int main(void) {
    AppState as;
    initializeAppState(&as);
    char code[] = {
    '>', ':', '1', '-', ':', 'v', 0, 'v', 0, '*', '_', '$', '.', '@', 0, 0,
    '^', 0, 0, 0, 0, '_', '$', '>', '\\', ':', '^'
    };
    memcpy(as.board.data, code, 27);
    as.mode = MODE_EXECUTION;
    as.stack.data[as.stack.cur++] = 5;
    while (1) {

        // printf("%hhu %hhu %c\n", (as.pc & 0x70) >> 4, (as.pc & 0xf), as.board.data[as.pc & (~0x180)] ? as.board.data[as.pc & (~0x180)] : ' ');
        // for (int* i = as.stack.data; i != as.stack.data + as.stack.cur; printf("%d ", *(i++)));
        // putchar(10);
        processAppState(&as, 0, 0);
        if (as.mode & 0xf != MODE_EXECUTION) break;
        switch (as.change) {
        case OUT_INT:
            printf("%d", as.out);
            break;
        case OUT_CHAR:
            putchar(as.out);
            break;
        }
    }
}
