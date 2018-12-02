#ifndef LOGIC_SEEN
#define LOGIC_SEEN

#include "gba.h"

#define LEFT (((u8)0) << 7)
#define RIGHT (((u8)1) << 7)
#define UP (((u8)2) << 7)
#define DOWN (((u8)3) << 7)

#define NUMBER (0)
#define OPERATOR (1 << 6)
#define CONTROL (2 << 6)
#define END (3 << 6)

#define OUT_CHAR (1)
#define OUT_INT (2)
#define OUT_OF_BOUND (4)
#define EDITOR_CURS_MOVED (5)
#define ENTER_SELECTION (6)
#define EXIT_SELECTION (7)
#define EDITOR_SELECT_MOVED (8)
#define EDITOR_SELECT_PAGE_CHANGED (9)
#define COMPLETE_SELECTION (10)
#define EDITOR_ERASED (11)
#define ENTER_EXECUTION (12)
#define EXIT_EXECUTION (13)
#define BEGIN_EXECUTION (3)
#define EXECUTION_CURS_MOVED (14)
#define STACK_OVERFLOW (15)

#define MODE_EDITOR (1)
#define MODE_EDITOR_SELECT (2)
#define MODE_EDITOR_DIR (3)
#define MODE_EXECUTION_PAUSE (4)
#define MODE_EXECUTION_AUTO (5)


typedef struct {
    // bits 6 to 4 are y coordinate bits, bits 3 to 0 are x coordinate bits
    u8 cur; //0b0yyyxxxx
    char data[128];
} Board;

typedef struct {
    u8 cur;
    int data[100];
} Stack;

// top 2 bits are direction bits, 3 bits after that are y coordinate bits, 
// bottom 4 bits are x coordinate bits
typedef u16 ProgramCounter; //0bddyyyxxxx
// top 2 bits are page bits (category), 5 bits after that are y coordinate bits,
// bottom 1 bit is x coordinate bit
typedef u8 SelectPos; //0bppyyyyyx
// top 4 bits are game end indicator bits, bottom 4 bits are game mode bits
typedef u8 Mode; //0beeeemmmm
// top 28 bits are data bits, bottom 4 bits are mode bits
typedef u32 Change; //0b dddd dddd dddd dddd dddd dddd dddd mmmm

typedef struct {
    Mode mode;
    Change change;
    SelectPos select_pos;
    u8 board_in;
    int out;
    Board board;
    Stack stack;
    ProgramCounter pc;
} AppState;


// This function can initialize an unused AppState struct.
void initializeAppState(AppState *appState);

// This function will be used to process app frames.
void processAppState(AppState *currentAppState, u32 keysPressedBefore, u32 keysPressedNow);


#endif
