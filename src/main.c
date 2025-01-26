/*
 * matrix.c - Draws a matrix-like rain of characters on the screen.
 *            then scroll the screen down
 *
 *  Compile example (on 2.11BSD, might need -lm for math library):
 *    make -f sdcc.mk
 *
 *  Original Author: Dave Plummer, 2024.
 *  Modified for Zeal 8-bit by David Higgins, 2025.
 *  License: GPL 2.0
 *
 *  Original Source: https://github.com/davepl/pdpsrc/blob/main/bsd/screensavers/matrix.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zvb_hardware.h>


// Zeal specific code
#define SCREEN_COL80_WIDTH  80
#define SCREEN_COL80_HEIGHT 40
char rand8_quick(void) __naked
{
    __asm__(
    "ld a, r\n"
    "ret\n"
    );
}

uint8_t mmu_page_current;
const __sfr __banked __at(0xF0) mmu_page0_ro;
__sfr __at(0xF2) mmu_page2;
uint8_t __at(0x8000) SCR_TEXT[SCREEN_COL80_HEIGHT][SCREEN_COL80_WIDTH];
uint8_t __at(0x9000) SCR_COLOR[SCREEN_COL80_HEIGHT][SCREEN_COL80_WIDTH];

__sfr __banked __at(0x9d) vid_ctrl_status;

static inline void text_map_vram(void)
{
    mmu_page_current = mmu_page0_ro;
    __asm__("di");
    mmu_page2 = VID_MEM_PHYS_ADDR_START >> 14;
}

static inline void text_demap_vram(void)
{
    __asm__("ei");
    mmu_page2 = mmu_page_current;
}

static zos_err_t err;


#define SCREEN_WIDTH  SCREEN_COL80_WIDTH
#define SCREEN_HEIGHT SCREEN_COL80_HEIGHT
#define MAX_TRAILS    16

/* Structure to represent a trail */
struct Trail {
        int column;     /* Column where the trail is active */
        int rows_drawn; /* Number of rows drawn so far */
        int length;     /* Length of the trail */
        int active;     /* Whether this trail is active */
};

struct Trail trails[SCREEN_WIDTH];
uint8_t total_trails = 0;
int trail_timer = 0;

void initialize_trails(void)
{
    int i;
    for (i = 0; i < SCREEN_WIDTH; i++) {
        trails[i].active = 0;
    }
}

void start_new_trail(int length)
{
    int i;
    do {
        i = rand() % SCREEN_WIDTH;
        if (!trails[i].active) {
            trails[i].column     = i; // rand() % SCREEN_WIDTH;
            trails[i].rows_drawn = 0; /* Start with no rows drawn */
            trails[i].length     = length;
            trails[i].active     = 1;
            total_trails++;
            break;
        }
    } while(!trails[i].active);
}

void update_trails(void)
{
    int i;

    for (i = 0; i < SCREEN_WIDTH; i++) {
        char c  = ' ';
        if (trails[i].active) {
            /* Draw only in the first row if the trail is active */
            if (trails[i].rows_drawn < trails[i].length) {
                // printf("\033[1;%dH", trails[i].column + 1);
                c = (rand() % (135));
                // Mirrored Katakana character
                c = c + '!';

                /* Increment the number of rows drawn */
                trails[i].rows_drawn++;
            } else {
                trails[i].active = 0;
                total_trails--;
            }
        }

        zvb_peri_text_print_char = c;
    }
}

int main(void)
{
    int trail_length = 16; /* Configurable length of the trail */
    int spawn_rate   = 1; /* Configurable spawn rate */

    /* Seed the random generator */
    uint16_t seed = rand8_quick();
    srand(seed);

    /* Hide the cursor */
    zvb_peri_text_curs_time = 0;

    /* Clear screen */
    err = ioctl(DEV_STDOUT, CMD_CLEAR_SCREEN, (void *)NULL);
    if(err != ERR_SUCCESS) exit(err);

    /* Initialize trails */
    initialize_trails();

    text_map_vram();

    zvb_peri_text_color = 0x0A;
    zvb_peri_text_curs_x = 0;
    zvb_peri_text_curs_y = SCREEN_HEIGHT-1;

    for (;;) {
        /* Start a new trail periodically */
        if (trail_timer % spawn_rate == 0) {
            start_new_trail((rand() % trail_length) + 2);
        }

        /* Update and draw trails */
        update_trails();

        /* Small delay */
        msleep(50);
        trail_timer++;
    }

    /* Normally never reached */
    err = ioctl(DEV_STDOUT, CMD_RESET_SCREEN, (void *)NULL);
    return 0;
}
