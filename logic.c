#include "gba.h"
#include "logic.h"
#include "images/bf_bg.h"
// #define DBG
static int o1 = 0;
static int o2 = 0;

static const u8 lim[] = {5, 4, 4, 1};
static const char op[] = {'+', '-', '*', '/', '%', '!', '`', '?'};
static const char cnt[] = {'_', '|', ':', '\\', '$', '.', ',', '#'};

#define PUSH(s, d) (s).data[(s).cur++] = d
#define POP(s) ((s).cur ? (s).data[--(s).cur] : 0)
#define PEEK(s) ((s).cur ? (s).data[(s).cur - 1] : 0)

#define MOVE_PC(p) {\
    switch (p & 0x180) {\
    case LEFT:\
        p--;\
        break;\
    case RIGHT:\
        p++;\
        break;\
    case DOWN:\
        p += 16;\
        break;\
    case UP:\
        p -= 16;\
        break;\
    }\
}

#define MOVE_BOARD(b, d) {\
    switch (d) {\
    case LEFT:\
        b = (b) & 0xf ? (b) - 1 : ((b) & ~0xf) | 0xf;\
        break;\
    case RIGHT:\
        b = ((b) & 0xf) != 15 ? (b) + 1 : (b) & ~0xf;\
        break;\
    case DOWN:\
        b = ((b) & 0x70) != 0x70 ? (b) + 16 : (b) & ~0x70;\
        break;\
    case UP:\
        b = ((b) & 0x70) ? (b) - 16 : ((b) & ~0x70) | 0x70;\
        break;\
    }\
}

#define MOVE_SELECT(s, d, m) {\
    switch (d) {\
    case LEFT:\
    case RIGHT:\
        s ^= ((s) & 0xc0) != END ? 1 : 0;\
        break;\
    case UP:\
        s = ((s) & 0x3e) ? (s) - 2 : ((s) & ~0x3e) | (((m) - 1) << 1);\
        break;\
    case DOWN:\
        s = ((s) & 0x3e) != (((m) - 1) << 1) ? (s) + 2 : (s) & ~0x3e;\
        break;\
    }\
}

#define MOVE_SELECT_PAGE(s, d) {\
    switch (d) {\
    case LEFT:\
        s = s & 0xc0 ? s - (1 << 6) : (s & 0x3f) | END;\
        break;\
    case RIGHT:\
        s = (s & 0xc0) != END ? s + (1 << 6) : s & 0x3f;\
        break;\
    }\
}
#define SET_BOARD(b, d) (b).data[(b).cur] = d

#define HANDLE(a, b, s, p, c) {\
    switch(c) {\
    case '0':\
    case '1':\
    case '2':\
    case '3':\
    case '4':\
    case '5':\
    case '6':\
    case '7':\
    case '8':\
    case '9':\
        PUSH(s, c - '0');\
        break;\
    case '>':\
        p &= ~0x180;\
        p |= RIGHT;\
        break;\
    case '^':\
        p &= ~0x180;\
        p |= UP;\
        break;\
    case '<':\
        p &= ~0x180;\
        p |= LEFT;\
        break;\
    case 'v':\
        p &= ~0x180;\
        p |= DOWN;\
        break;\
    case '+':\
        o2 = POP(s);\
        o1 = POP(s);\
        PUSH(s, o1 + o2);\
        break;\
    case '-':\
        o2 = POP(s);\
        o1 = POP(s);\
        PUSH(s, o1 - o2);\
        break;\
    case '*':\
        o2 = POP(s);\
        o1 = POP(s);\
        PUSH(s, o1 * o2);\
        break;\
    case '/':\
        o2 = POP(s);\
        o1 = POP(s);\
        PUSH(s, o1 / o2);\
        break;\
    case '%':\
        o2 = POP(s);\
        o1 = POP(s);\
        PUSH(s, o1 % o2);\
        break;\
    case '!':\
        o1 = POP(s);\
        PUSH(s, !o1);\
        break;\
    case '`':\
        o2 = POP(s);\
        o1 = POP(s);\
        PUSH(s, o1 > o2);\
        break;\
    case '?':\
        p &= ~0x180;\
        p |= (((u8)randint(0, 4)) << 7);\
        break;\
    case '_':\
        p &= ~0x180;\
        o1 = POP(s);\
        p |= o1 ? LEFT : RIGHT;\
        break;\
    case '|':\
        p &= ~0x180;\
        o1 = POP(s);\
        p |= o1 ? DOWN : UP;\
        break;\
    case ':':\
        o1 = PEEK(s);\
        PUSH(s, o1);\
        break;\
    case '\\':\
        o2 = POP(s);\
        o1 = POP(s);\
        PUSH(s, o2);\
        PUSH(s, o1);\
        break;\
    case '$':\
        o1 = POP(s);\
        break;\
    case '.':\
        (a).out = POP(s);\
        (a).change = OUT_INT;\
        break;\
    case ',':\
        (a).out = POP(s);\
        (a).change = OUT_CHAR;\
        break;\
    case '#':\
        MOVE_PC(p);\
        break;\
    case '@':\
        (a).mode = MODE_EXECUTION_PAUSE;\
        (a).change = ENTER_EXECUTION;\
        (a).pc = RIGHT;\
        (a).stack.cur = 0;\
        break;\
    }\
}

void initializeAppState(AppState* appState) {
    appState->mode = MODE_EDITOR;
    appState->change = 0;
    appState->out = 0;
    appState->board.cur = 0;
    char* end = appState->board.data + 128;
    for (char* i = appState->board.data; i != end; *(i++) = 0);
    appState->stack.cur = 0;
    appState->pc = RIGHT;
    appState->select_pos = 0;
}

void processAppState(AppState *currentAppState, u32 keysPressedBefore,
                     u32 keysPressedNow) {
    char cur = 0;
    u8 cur_board = currentAppState->board.cur;
    u8 cur_sel = currentAppState->select_pos;
    u16 pc = currentAppState->pc;
    switch (currentAppState->mode & 0xf) {
    case MODE_EDITOR:
        if (!KEY_DOWN(BUTTON_DOWN, keysPressedBefore) &&
            KEY_DOWN(BUTTON_DOWN, keysPressedNow)) {
            MOVE_BOARD(currentAppState->board.cur, DOWN);
            currentAppState->change = EDITOR_CURS_MOVED;
        }
        if (!KEY_DOWN(BUTTON_UP, keysPressedBefore) &&
            KEY_DOWN(BUTTON_UP, keysPressedNow)) {
            MOVE_BOARD(currentAppState->board.cur, UP);
            currentAppState->change = EDITOR_CURS_MOVED;
        }
        if (!KEY_DOWN(BUTTON_LEFT, keysPressedBefore) &&
            KEY_DOWN(BUTTON_LEFT, keysPressedNow)) {
            MOVE_BOARD(currentAppState->board.cur, LEFT);
            currentAppState->change = EDITOR_CURS_MOVED;
        }
        if (!KEY_DOWN(BUTTON_RIGHT, keysPressedBefore) &&
            KEY_DOWN(BUTTON_RIGHT, keysPressedNow)) {
            MOVE_BOARD(currentAppState->board.cur, RIGHT);
            currentAppState->change = EDITOR_CURS_MOVED;
        }
        if (currentAppState->change) {
            currentAppState->change &= ~(0x7f << 25);
            currentAppState->change |= (cur_board & 0x7f) << 25;
            break;
        }
        if (!KEY_DOWN(BUTTON_L, keysPressedBefore) &&
            KEY_DOWN(BUTTON_L, keysPressedNow)) {
            currentAppState->board.data[cur_board] = 0;
            currentAppState->change = EDITOR_ERASED;
            break;
        }
        if (!KEY_DOWN(BUTTON_B, keysPressedBefore) &&
            KEY_DOWN(BUTTON_B, keysPressedNow)) {
            currentAppState->mode &= 0xf0;
            currentAppState->mode |= MODE_EDITOR_DIR;
            currentAppState->board_in = '^';
            break;
        }
        if (!KEY_DOWN(BUTTON_A, keysPressedBefore) &&
            KEY_DOWN(BUTTON_A, keysPressedNow)) {
            currentAppState->mode &= 0xf0;
            currentAppState->mode |= MODE_EDITOR_SELECT;
            currentAppState->change |= ENTER_SELECTION;
            currentAppState->change &= ~(0x7f << 25);
            currentAppState->change |= (cur_board & 0x7f) << 25;
            currentAppState->select_pos = 0;
            break;
        }
        if (!KEY_DOWN(BUTTON_START, keysPressedBefore) &&
            KEY_DOWN(BUTTON_START, keysPressedNow)) {
            currentAppState->change = ENTER_EXECUTION;
            currentAppState->mode &= 0xf0;
            currentAppState->mode |= MODE_EXECUTION_PAUSE;
        }
        break;
    case MODE_EDITOR_SELECT:
        if (!KEY_DOWN(BUTTON_LEFT, keysPressedBefore) &&
            KEY_DOWN(BUTTON_LEFT, keysPressedNow)) {
            MOVE_SELECT(currentAppState->select_pos, LEFT,
                        lim[currentAppState->select_pos >> 6]);
            currentAppState->change = EDITOR_SELECT_MOVED;
        }
        if (!KEY_DOWN(BUTTON_RIGHT, keysPressedBefore) &&
            KEY_DOWN(BUTTON_RIGHT, keysPressedNow)) {
            MOVE_SELECT(currentAppState->select_pos, RIGHT,
                        lim[currentAppState->select_pos >> 6]);
            currentAppState->change = EDITOR_SELECT_MOVED;
        }
        if (!KEY_DOWN(BUTTON_DOWN, keysPressedBefore) &&
            KEY_DOWN(BUTTON_DOWN, keysPressedNow)) {
            MOVE_SELECT(currentAppState->select_pos, DOWN,
                        lim[currentAppState->select_pos >> 6]);
            currentAppState->change = EDITOR_SELECT_MOVED;
        }
        if (!KEY_DOWN(BUTTON_UP, keysPressedBefore) &&
            KEY_DOWN(BUTTON_UP, keysPressedNow)) {
            MOVE_SELECT(currentAppState->select_pos, UP,
                        lim[currentAppState->select_pos >> 6]);
            currentAppState->change = EDITOR_SELECT_MOVED;
        }
        if (currentAppState->change) {
            currentAppState->change &= ~(0x3f << 26);
            currentAppState->change |= (cur_sel & 0x3f) << 26;
            break;
        }
        if (!KEY_DOWN(BUTTON_R, keysPressedBefore) &&
            KEY_DOWN(BUTTON_R, keysPressedNow)) {
            MOVE_SELECT_PAGE(currentAppState->select_pos, RIGHT)
            currentAppState->change = EDITOR_SELECT_PAGE_CHANGED;
        }
        if (!KEY_DOWN(BUTTON_L, keysPressedBefore) &&
            KEY_DOWN(BUTTON_L, keysPressedNow)) {
            MOVE_SELECT_PAGE(currentAppState->select_pos, LEFT);
            currentAppState->change = EDITOR_SELECT_PAGE_CHANGED;
        }
        if (currentAppState->change) {
            currentAppState->select_pos &= 0xc0;
            currentAppState->change &= ~(0x3f << 26);
            currentAppState->change |= (cur_sel & 0x3f) << 26;
            break;
        }
        if (!KEY_DOWN(BUTTON_B, keysPressedBefore) &&
            KEY_DOWN(BUTTON_B, keysPressedNow)) {
            currentAppState->mode &= 0xf0;
            currentAppState->mode |= MODE_EDITOR;
            currentAppState->change = EXIT_SELECTION;
            break;
        }
        if (!KEY_DOWN(BUTTON_A, keysPressedBefore) &&
            KEY_DOWN(BUTTON_A, keysPressedNow)) {
            switch (currentAppState->select_pos & 0xc0) {
            case NUMBER:
                currentAppState->board.data[cur_board] =
                (currentAppState->select_pos & 0x3f) + '0';
                break;
            case OPERATOR:
                currentAppState->board.data[cur_board] =
                op[currentAppState->select_pos & 0x3f];
                break;
            case CONTROL:
                currentAppState->board.data[cur_board] =
                cnt[currentAppState->select_pos & 0x3f];
                break;
            case END:
                currentAppState->board.data[cur_board] = '@';
                break;
            }
            currentAppState->mode &= 0xf0;
            currentAppState->mode |= MODE_EDITOR;
            currentAppState->change = COMPLETE_SELECTION;
            break;
        }
        break;
    case MODE_EDITOR_DIR:
        if (!KEY_DOWN(BUTTON_DOWN, keysPressedBefore) &&
            KEY_DOWN(BUTTON_DOWN, keysPressedNow)) {
            currentAppState->board_in = 'v';
        }
        if (!KEY_DOWN(BUTTON_UP, keysPressedBefore) &&
            KEY_DOWN(BUTTON_UP, keysPressedNow)) {
            currentAppState->board_in = '^';
        }
        if (!KEY_DOWN(BUTTON_LEFT, keysPressedBefore) &&
            KEY_DOWN(BUTTON_LEFT, keysPressedNow)) {
            currentAppState->board_in = '<';
        }
        if (!KEY_DOWN(BUTTON_RIGHT, keysPressedBefore) &&
            KEY_DOWN(BUTTON_RIGHT, keysPressedNow)) {
            currentAppState->board_in = '>';
        }
        if (!KEY_DOWN(BUTTON_B, keysPressedBefore) &&
            KEY_DOWN(BUTTON_B, keysPressedNow)) {
            currentAppState->board.data[cur_board] = currentAppState->board_in;
            currentAppState->mode &= 0xf0;
            currentAppState->mode |= MODE_EDITOR;
        }
        break;
    case MODE_EXECUTION_PAUSE:
        if (!KEY_DOWN(BUTTON_START, keysPressedBefore) &&
            KEY_DOWN(BUTTON_START, keysPressedNow)) {
            currentAppState->mode &= 0xf0;
            currentAppState->mode |= MODE_EXECUTION_AUTO;
            currentAppState->change = BEGIN_EXECUTION;
        }
        if (!KEY_DOWN(BUTTON_R, keysPressedBefore) &&
            KEY_DOWN(BUTTON_R, keysPressedNow)) {
            currentAppState->change = ENTER_EXECUTION;
            currentAppState->pc = RIGHT;
        }
        if (!KEY_DOWN(BUTTON_B, keysPressedBefore) &&
            KEY_DOWN(BUTTON_B, keysPressedNow)) {
            currentAppState->mode = MODE_EDITOR;
            currentAppState->change = EXIT_EXECUTION;
            currentAppState->pc = RIGHT;
            currentAppState->stack.cur = 0;
        }
        break;
    case MODE_EXECUTION_AUTO:
        cur = currentAppState->board.data[pc & (~0x180)];
        HANDLE(*currentAppState, currentAppState->board,
               currentAppState->stack, currentAppState->pc,
               cur);
        if ((currentAppState->mode & 0xf) == MODE_EXECUTION_PAUSE) break;
        if (currentAppState->stack.cur >= 100) {
            currentAppState->mode = 0;
            currentAppState->mode = 0xff | MODE_EDITOR;
            currentAppState->change = STACK_OVERFLOW;
        }
        switch (currentAppState->pc & 0xf) {
        case 0:
            if ((currentAppState->pc & 0x180) == LEFT) {
                currentAppState->mode = 0;
                currentAppState->mode = 0xff | MODE_EDITOR;
                currentAppState->change = OUT_OF_BOUND;
            }
            break;
        case 15:
            if ((currentAppState->pc & 0x180) == RIGHT) {
                currentAppState->mode = 0;
                currentAppState->mode = 0xff | MODE_EDITOR;
                currentAppState->change = OUT_OF_BOUND;
            }
            break;
        }
        switch (currentAppState->pc & 0x70) {
        case 0:
            if ((currentAppState->pc & 0x180) == UP) {
                currentAppState->mode = 0;
                currentAppState->mode = 0xff | MODE_EDITOR;
                currentAppState->change = OUT_OF_BOUND;
            }
            break;
        case 0x70:
            if ((currentAppState->pc & 0x180) == DOWN) {
                currentAppState->mode = 0;
                currentAppState->mode = 0xff | MODE_EDITOR;
                currentAppState->change = OUT_OF_BOUND;
            }
            break;
        }

        MOVE_PC(currentAppState->pc);
        if (currentAppState->change) {
            currentAppState->change |= (pc & 0x7f) << 25;
            break;
        }
        currentAppState->change = EXECUTION_CURS_MOVED;
        currentAppState->change |= (pc & 0x7f) << 25;
        break;
    }
}
