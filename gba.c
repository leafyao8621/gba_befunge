#include "gba.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

volatile u16 *videoBuffer = (volatile u16*)0x6000000;
volatile u16 *videoBufferEnd = (volatile u16*)0x6000000 + 240 * 160;

u32 vblankCounter = 0;

void waitForVBlank(void) {

    // Write a while loop that loops until we're NOT in vBlank anymore:
    // (This prevents counting one vblank more than once if your app is too fast)

    while (*SCANLINECOUNTER > 160);
    // Write a while loop that keeps going until we're in vBlank:
    while (*SCANLINECOUNTER < 160);

    // Finally, increment the vBlank counter:
    vblankCounter++;

}

static int __qran_seed= 42;
static int qran(void) {
    __qran_seed= 1664525*__qran_seed+1013904223;
    return (__qran_seed>>16) & 0x7FFF;
}

int randint(int min, int max) {
    return (qran()*(max-min)>>15)+min;
}

void setPixel(u8 x, u8 y, u16 color) {
    int ost = OFFSET(y, x, 240);
    if (ost >= 0 && ost < 38400)
        videoBuffer[OFFSET(y, x, 240)] = color;
}

void drawRectDMA(u8 x, u8 y, u8 width, u8 height, volatile u16 color) {
    volatile u16* i = videoBuffer + OFFSET(y, x, 240);
    for (u8 cnt = 0;
         cnt < height && i >= videoBuffer && i < videoBufferEnd;
         cnt++, i += 240) {
        DMA[3].src = &color;
        DMA[3].dst = i;
        DMA[3].cnt = DMA_ON |
                     DMA_SOURCE_FIXED |
                     DMA_DESTINATION_INCREMENT |
                     MIN(width, 240 - x);
    }
}

void drawFullScreenImageDMA(const u16 *image) {
    DMA[3].src = image;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = 38400 |
                 DMA_DESTINATION_INCREMENT |
                 DMA_SOURCE_INCREMENT |
                 DMA_ON;

}

void drawImageDMA(u8 x, u8 y, u8 width, u8 height, const u16 *image) {
    volatile u16* i = (u16*)image;
    volatile u16* j = videoBuffer + OFFSET(y, x, 240);
    for (u8 cnt = 0;
         cnt < height && j >= videoBuffer && j < videoBufferEnd;
         cnt++, i += width, j += 240) {
        DMA[3].src = i;
        DMA[3].dst = j;
        DMA[3].cnt = MIN(width, 240 - width) |
                     DMA_SOURCE_INCREMENT |
                     DMA_DESTINATION_INCREMENT |
                     DMA_ON;
    }
}

void fillScreenDMA(volatile u16 color) {
    DMA[3].cnt = 0;
    DMA[3].src = &color;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = 38400 |
                 DMA_DESTINATION_INCREMENT |
                 DMA_SOURCE_FIXED |
                 DMA_ON;
}

void drawChar(u8 col, u8 row, char ch, u16 color) {
    for(u8 r = 0; r < 8; r++) {
        for(u8 c = 0; c < 6; c++) {
            if(fontdata_6x8[OFFSET(r, c, 6) + ch*48]) {
                setPixel(col + c, row + r, color);
            }
        }
    }
}

void drawInt(u8 col, u8 row, int n, u16 color) {
    if (!n) {
        drawChar(col, row, '0', color);
        return;
    }
    static char buf[20];
    if (n & 0x80000000) {
        n = -n;
        drawChar(col, row, '-', color);
        col += 6;
    }
    char* i = buf;
    for (; n; *(i++) = n % 10, n /= 10);
    for (; i != buf; drawChar(col, row, *(--i) + '0', color), col += 6);
}

void drawString(u8 col, u8 row, char *str, u16 color) {
    while(*str) {
        drawChar(col, row, *str++, color);
        col += 6;
    }
}

void drawCenteredString(u8 x, u8 y, u8 width, u8 height, char *str,
                        u16 color) {
    u8 len = 0;
    char *strCpy = str;
    while (*strCpy) {
        len++;
        strCpy++;
    }

    u8 strWidth = 6 * len;
    u8 strHeight = 8;

    u8 col = x + ((width - strWidth) >> 1);
    u8 row = y + ((height - strHeight) >> 1);
    drawString(col, row, str, color);
}

void patchBackground(u8 x, u8 y, u8 width, u8 height, const u16* image) {
    volatile u16* i = (u16*)image + OFFSET(y, x, 240);
    volatile u16* j = videoBuffer + OFFSET(y, x, 240);
    for (u8 k = 0; k < height; k++, i += 240, j += 240) {
        DMA[3].src = i;
        DMA[3].dst = j;
        DMA[3].cnt = width | DMA_SOURCE_INCREMENT |
                             DMA_DESTINATION_INCREMENT |
                             DMA_ON;
    }
}
