#include "gba.h"
#include "logic.h"
#include "graphics.h"
#include "images/bf_tit.h"

#include <stdio.h>
#include <stdlib.h>

// AppState enum definition
typedef enum {
    START,
    START_NODRAW,
    APP_INIT,
    APP,
    APP_EXIT,
    APP_EXIT_NODRAW,
} GBAState;

int main(void) {
    REG_DISPCNT = MODE3 | BG2_ENABLE;
    GBAState state = START;

    AppState currentAppState;

    // We store the current and previous values of the button input.
    u32 previousButtons = BUTTONS;
    u32 currentButtons = BUTTONS;
    while(1) {
        // Load the current state of the buttons
        currentButtons = BUTTONS;

        switch(state) {
        case START:
            // Wait for vblank
            waitForVBlank();
            drawFullScreenImageDMA(bf_tit);
            drawCenteredString(10, 50, 220, 8, "BEFUNGE ABRIDGED", WHITE);
            drawCenteredString(10, 60, 220, 8, "ADVANCE", WHITE);
            drawCenteredString(10, 100, 220, 8, "Press Start to Continue",
                               WHITE);
            state = START_NODRAW;
            previousButtons = currentButtons;
            break;
        case START_NODRAW:
            // Start the app by switching the state to APP_INIT.
            if (!KEY_DOWN(BUTTON_START, previousButtons) &&
                KEY_DOWN(BUTTON_START, currentButtons)) {
                state = APP_INIT;
            }
            break;
        case APP_INIT:
            // Initialize the app. Switch to the APP state.
            initializeAppState(&currentAppState);
            // Draw the initial state of the app
            waitForVBlank();
            drawInitialAppState();
            state = APP;
            break;
        case APP:
            if (!KEY_DOWN(BUTTON_SELECT, previousButtons) &&
                KEY_DOWN(BUTTON_SELECT, currentButtons)) {
                state = APP_EXIT;
                break;
            }
            // Process the app for one frame, store the next state
            processAppState(&currentAppState, previousButtons, currentButtons);

            // Wait for vblank before we do any drawing.
            waitForVBlank();
            // Draw the current state
            drawAppState(&currentAppState);

            // Check if the app is exiting. If it is, then go to the exit state.
            if (currentAppState.mode & 0xf0) state = APP_EXIT;

            break;
        case APP_EXIT:
            // Wait for vblank
            waitForVBlank();
            drawFullScreenImageDMA(bf_tit);
            switch (currentAppState.change & 0xf) {
            case OUT_OF_BOUND:
                drawCenteredString(10, 60, 220, 8,
                                   "OUT OF BOUND", WHITE);
                break;
            case STACK_OVERFLOW:
                drawCenteredString(10, 60, 220, 8,
                                   "Stack Overflow", WHITE);
                break;
            default:
                drawCenteredString(10, 60, 220, 8,
                                   "Thanks for Using", WHITE);
            }
            
            drawCenteredString(10, 100, 220, 8, "Press Start to Continue",
                               WHITE);
            state = APP_EXIT_NODRAW;
            break;
        case APP_EXIT_NODRAW:
            // TA-TODO: Check for a button press here to go back to the start screen
            if (!KEY_DOWN(BUTTON_START, previousButtons) &&
                KEY_DOWN(BUTTON_START, currentButtons)) {
                state = START;
            }
            break;
        }

        // Store the current state of the buttons
        previousButtons = currentButtons;
    }

    return 0;
}
