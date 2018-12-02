#include "graphics.h"
#include "gba.h"
#include "images/curs.h"
#include "images/bf_bg.h"
#include "images/block_curs_up.h"
#include "images/block_curs_right.h"
#include "images/block_curs_left.h"
#include "images/block_curs_down.h"
#include "images/block_num_0.h"
#include "images/block_num_1.h"
#include "images/block_num_2.h"
#include "images/block_num_3.h"
#include "images/block_num_4.h"
#include "images/block_num_5.h"
#include "images/block_num_6.h"
#include "images/block_num_7.h"
#include "images/block_num_8.h"
#include "images/block_num_9.h"
#include "images/block_op_plus.h"
#include "images/block_op_minus.h"
#include "images/block_op_asterisk.h"
#include "images/block_op_slash.h"
#include "images/block_op_percent.h"
#include "images/block_op_bang.h"
#include "images/block_op_backtick.h"
#include "images/block_op_question.h"
#include "images/block_cnt_underscore.h"
#include "images/block_cnt_pipe.h"
#include "images/block_cnt_colon.h"
#include "images/block_cnt_backslash.h"
#include "images/block_cnt_dollar.h"
#include "images/block_cnt_fullstop.h"
#include "images/block_cnt_comma.h"
#include "images/block_cnt_hash.h"
#include "images/block_end.h"
#include "images/curs_exec_left.h"
#include "images/curs_exec_right.h"
#include "images/curs_exec_up.h"
#include "images/curs_exec_down.h"
#include "images/exec_pause.h"
#include "images/exec_auto.h"

static const u16* nums[] = {block_num_0, block_num_1, block_num_2,
                            block_num_3, block_num_4, block_num_5,
                            block_num_6, block_num_7, block_num_8,
                            block_num_9};

static const u16* ops[] = {block_op_plus, block_op_minus,
                           block_op_asterisk, block_op_slash,
                           block_op_percent, block_op_bang,
                           block_op_backtick, block_op_question};

static const u16* cnts[] = {block_cnt_underscore, block_cnt_pipe,
                            block_cnt_colon, block_cnt_backslash,
                            block_cnt_dollar, block_cnt_fullstop,
                            block_cnt_comma, block_cnt_hash};

static char* page[] = {"NUMBER", "OPERATOR", "CONTROL", "TERMINATOR"};

void drawInitialAppState(void) {
    drawFullScreenImageDMA(bf_bg);
    for (u8 i = 40; i <= 130; drawRectDMA(40, i, 160 + 17, 1, WHITE),
                              i += 11);
    for (u8 i = 40; i <= 220; drawRectDMA(i, 40, 1, 80 + 9, WHITE),
                              i += 11);
    drawImageDMA(46, 46, 5, 5, curs);
}

#define PATCH(pos) {\
    if (!(state->board.data[pos])) {\
        patchBackground(41 + 11 * (pos & 0xf),\
                        41 + 11 * ((pos & 0x70) >> 4), 10, 10,\
                        bf_bg);\
    } else {\
        switch (state->board.data[pos]) {\
        case '^':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_curs_up);\
            break;\
        case '>':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_curs_right);\
            break;\
        case '<':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_curs_left);\
            break;\
        case 'v':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_curs_down);\
            break;\
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
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, nums[state->board.data[pos] - '0']);\
            break;\
        case '+':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_plus);\
            break;\
        case '-':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_minus);\
            break;\
        case '*':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_asterisk);\
            break;\
        case '/':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_slash);\
            break;\
        case '%':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_percent);\
            break;\
        case '!':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_bang);\
            break;\
        case '`':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_backtick);\
            break;\
        case '?':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_op_question);\
            break;\
        case '_':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_underscore);\
            break;\
        case '|':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_pipe);\
            break;\
        case ':':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_colon);\
            break;\
        case '\\':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_backslash);\
            break;\
        case '$':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_dollar);\
            break;\
        case '.':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_fullstop);\
            break;\
        case ',':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_comma);\
            break;\
        case '#':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_cnt_hash);\
            break;\
        case '@':\
            drawImageDMA(41 + 11 * (pos & 0xf),\
                         41 + 11 * ((pos & 0x70) >> 4),\
                         10, 10, block_end);\
            break;\
        }\
    }\
}

// Updates graphics for state using change message
void drawAppState(AppState *state) {
    u8 pos = 0;
    // handles changes
    if (state->change) {
        switch (state->change & 0xf) {
        // patches old cursor whose coordinates are stored in the
        // top 7 bits
        case EDITOR_CURS_MOVED:
            pos = (state->change & (0x7f << 25)) >> 25;
            PATCH(pos);
            break;
        // draws the blocks to be chosen by user
        // draws category string
        case ENTER_SELECTION:
            drawRectDMA(5, 30, 60, 8, BLACK);
            drawCenteredString(5, 30, 60, 8, "NUMBER", WHITE);
            for (u8 i = 0; i < 10; drawImageDMA(11 + (i & 1) * 11,
                                                41 + ((i & 0x3e) >> 1) * 11,
                                                10, 10, nums[i]), i++);
            break;
        // clean ups
        case EXIT_SELECTION:
            patchBackground(5, 30, 60, 8, bf_bg);
            patchBackground(11, 41, 25, 100, bf_bg);
            break;
        // patches the old cursor
        case EDITOR_SELECT_MOVED:
            pos = (state->change & (0x3f << 26)) >> 26;
            switch (state->select_pos & 0xc0) {
            case NUMBER:
                drawImageDMA(11 + (pos & 1) * 11,
                            41 + ((pos & 0x3e) >> 1) * 11,
                            10, 10, nums[pos]);
                break;
            case OPERATOR:
                drawImageDMA(11 + (pos & 1) * 11,
                            41 + ((pos & 0x3e) >> 1) * 11,
                            10, 10, ops[pos]);
                break;
            case CONTROL:
                drawImageDMA(11 + (pos & 1) * 11,
                            41 + ((pos & 0x3e) >> 1) * 11,
                            10, 10, cnts[pos]);
                break;
            }
            break;
        // updates blocks and category string
        case EDITOR_SELECT_PAGE_CHANGED:
            drawRectDMA(5, 30, 60, 8, BLACK);
            drawCenteredString(5, 30, 60, 8,
                               page[(state->select_pos & 0xc0) >> 6], WHITE);
            patchBackground(11, 41, 25, 100, bf_bg);
            switch (state->select_pos & 0xc0) {
            case NUMBER:
                for (u8 i = 0; i < 10; drawImageDMA(11 + (i & 1) * 11,
                                                    41 + ((i & 0x3e) >> 1) * 11,
                                                    10, 10, nums[i]), i++);
                break;
            case OPERATOR:
                for (u8 i = 0; i < 8; drawImageDMA(11 + (i & 1) * 11,
                                                   41 + ((i & 0x3e) >> 1) * 11,
                                                   10, 10, ops[i]), i++);
                break;
            case CONTROL:
                for (u8 i = 0; i < 8; drawImageDMA(11 + (i & 1) * 11,
                                                   41 + ((i & 0x3e) >> 1) * 11,
                                                   10, 10, cnts[i]), i++);
                break;
            case END:
                drawImageDMA(11, 41, 10, 10, block_end);
            }
            break;
        // cleanups and draws the block chosen to the editor grid
        case COMPLETE_SELECTION:
            patchBackground(5, 30, 60, 8, bf_bg);
            patchBackground(11, 41, 25, 100, bf_bg);
            switch (state->select_pos & 0xc0) {
            case NUMBER:
                drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                             41 + 11 * ((state->board.cur & 0x70) >> 4),
                             10, 10, nums[state->select_pos & 0x3f]);
                break;
            case OPERATOR:
                drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                             41 + 11 * ((state->board.cur & 0x70) >> 4),
                             10, 10, ops[state->select_pos & 0x3f]);
                break;
            case CONTROL:
                drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                             41 + 11 * ((state->board.cur & 0x70) >> 4),
                             10, 10, cnts[state->select_pos & 0x3f]);
                break;
            case END:
                drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                             41 + 11 * ((state->board.cur & 0x70) >> 4),
                             10, 10, block_end);
                break;
            }
            break;
        // patches the block
        case EDITOR_ERASED:
            patchBackground(41 + 11 * (state->board.cur & 0xf),
                            41 + 11 * ((state->board.cur & 0x70) >> 4),
                            10, 10, bf_bg);
            drawImageDMA(46 + 11 * (state->board.cur & 0xf),
                         46 + 11 * ((state->board.cur & 0x70) >> 4),
                         5, 5, curs);
            break;
        // patches the editor cursor and draws the execution cursor
        case ENTER_EXECUTION:
            drawImageDMA(75, 10, 10, 10, exec_pause);
            pos = state->board.cur & 0x7f;
            PATCH(pos);
            pos = (state->change & (0x7f << 25)) >> 25;
            PATCH(pos);
            switch (state->pc & 0x180) {
            case LEFT:
                drawImageDMA(41 + 11 * (state->pc & 0xf),
                             41 + 11 * ((state->pc & 0x70) >> 4),
                             10, 10, curs_exec_left);
                break;
            case RIGHT:
                drawImageDMA(41 + 11 * (state->pc & 0xf),
                             41 + 11 * ((state->pc & 0x70) >> 4),
                             10, 10, curs_exec_right);
                break;
            case UP:
                drawImageDMA(41 + 11 * (state->pc & 0xf),
                             41 + 11 * ((state->pc & 0x70) >> 4),
                             10, 10, curs_exec_up);
                break;
            case DOWN:
                drawImageDMA(41 + 11 * (state->pc & 0xf),
                             41 + 11 * ((state->pc & 0x70) >> 4),
                             10, 10, curs_exec_down);
                break;
            }
            drawRectDMA(5, 10, 42, 8, BLACK);
            drawString(5, 10, "Current", WHITE);
            drawRectDMA(50, 10, 6, 8, BLACK);
            drawRectDMA(5, 20, 42, 8, BLACK);
            drawString(5, 20, "Output", WHITE);
            drawRectDMA(50, 20, 6, 8, BLACK);
            drawRectDMA(50, 30, 60, 8, BLACK);
            break;
        case BEGIN_EXECUTION:
            drawImageDMA(75, 10, 10, 10, exec_auto);
            break;
        // cleanups
        case EXIT_EXECUTION:
            patchBackground(75, 10, 10, 10, bf_bg);
            PATCH(0);
            patchBackground(5, 10, 150, 30, bf_bg);
            break;
        // patches the old cursor
        case EXECUTION_CURS_MOVED:
            pos = (state->change & (0x7f << 25)) >> 25;
            PATCH(pos);
            break;
        // does output
        case OUT_CHAR:
            drawRectDMA(50, 20, 6, 8, BLACK);
            drawChar(50, 20, state->out, WHITE);
            pos = (state->change & (0x7f << 25)) >> 25;
            PATCH(pos);
            break;
        // does output
        case OUT_INT:
            drawRectDMA(50, 30, 60, 8, BLACK);
            drawInt(50, 30, state->out, WHITE);
            pos = (state->change & (0x7f << 25)) >> 25;
            PATCH(pos);
            break;
        // leave immediately, preserve change status
        case STACK_OVERFLOW:
        case OUT_OF_BOUND:
            return;
        }
        state->change = 0;
    }
    // draws new stuff
    switch (state->mode & 0xf) {
    case MODE_EDITOR:
        drawImageDMA(46 + 11 * (state->board.cur & 0xf),
                     46 + 11 * ((state->board.cur & 0x70) >> 4),
                     5, 5, curs);
        break;
    case MODE_EDITOR_SELECT:
        drawImageDMA(16 + (state->select_pos & 1) * 11,
                     46 + ((state->select_pos & 0x3e) >> 1) * 11,
                     5, 5, curs);
        break;
    case MODE_EDITOR_DIR:
        switch (state->board_in) {
        case '^':
            drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                         41 + 11 * ((state->board.cur & 0x70) >> 4),
                         10, 10, block_curs_up);
            break;
        case '>':
            drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                         41 + 11 * ((state->board.cur & 0x70) >> 4),
                         10, 10, block_curs_right);
            break;
        case '<':
            drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                         41 + 11 * ((state->board.cur & 0x70) >> 4),
                         10, 10, block_curs_left);
            break;
        case 'v':
            drawImageDMA(41 + 11 * (state->board.cur & 0xf),
                         41 + 11 * ((state->board.cur & 0x70) >> 4),
                         10, 10, block_curs_down);
            break;
        }
        break;
    case MODE_EXECUTION_AUTO:
        drawRectDMA(50, 10, 6, 8, BLACK);
        drawChar(50, 10, state->board.data[state->pc & 0x7f], WHITE);
        switch (state->pc & 0x180) {
        case LEFT:
            drawImageDMA(41 + 11 * (state->pc & 0xf),
                        41 + 11 * ((state->pc & 0x70) >> 4),
                        10, 10, curs_exec_left);
            break;
        case RIGHT:
            drawImageDMA(41 + 11 * (state->pc & 0xf),
                        41 + 11 * ((state->pc & 0x70) >> 4),
                        10, 10, curs_exec_right);
            break;
        case UP:
            drawImageDMA(41 + 11 * (state->pc & 0xf),
                        41 + 11 * ((state->pc & 0x70) >> 4),
                        10, 10, curs_exec_up);
            break;
        case DOWN:
            drawImageDMA(41 + 11 * (state->pc & 0xf),
                        41 + 11 * ((state->pc & 0x70) >> 4),
                        10, 10, curs_exec_down);
            break;
        }
        // delay
        for (volatile u32 i = 0; i < 100000; i++);
        break;
    }
}
