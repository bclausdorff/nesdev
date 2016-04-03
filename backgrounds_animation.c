/**
 * Backgrounds and Animation
 *
 * http://timcheeseman.com/nesdev/
 */

#include <stdint.h>
#include <stddef.h>

#define TV_NTSC 1
#include "nes.h"
#include "reset.h"

#include "data.h"

#pragma bss-name(push, "ZEROPAGE")
uint8_t i; // loop counter

// used by WritePPU method
uintptr_t       ppu_addr;      // destination PPU address
uint8_t const * ppu_data;      // pointer to data to copy to PPU
uint8_t         ppu_data_size; // # of bytes to copy to PPU

uint8_t         game_state;
uint8_t const * background;
#pragma bss-name(pop)

#pragma bss-name(push, "OAM")
sprite_t player;
sprite_t pause_sprites[5];
#pragma bss-name(pop)

void ResetScroll() {
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;
}

void EnablePPU() {
    PPU_CTRL = PPUCTRL_NAMETABLE_0 | // use nametable 0
               PPUCTRL_BPATTERN_0  | // background uses pattern table 0
               PPUCTRL_NMI_ON      ; // enable NMIs

    PPU_MASK = PPUMASK_COLOR    | // show colors
               PPUMASK_BSHOW    | // show background
               PPUMASK_L8_BSHOW | // show background tiles in leftmost 8px
               PPUMASK_SSHOW    | // show sprites
               PPUMASK_L8_SSHOW ; // show sprites in leftmost 8px
}

void WritePPU() {
    PPU_ADDRESS = (uint8_t)(ppu_addr >> 8);
    PPU_ADDRESS = (uint8_t)(ppu_addr);
    for ( i = 0; i < ppu_data_size; ++i ) {
        PPU_DATA = ppu_data[i];
    }
}

void DrawBackground() {
    PPU_ADDRESS = (uint8_t)(PPU_NAMETABLE_0 >> 8);
    PPU_ADDRESS = (uint8_t)(PPU_NAMETABLE_0);
    UnRLE(background);
}

void main(void) {
    // write palettes
    ppu_addr = PPU_PALETTE;
    ppu_data = PALETTES;
    ppu_data_size = sizeof(PALETTES);
    WritePPU();

    // draw background
    background = GameBackground;
    DrawBackground();

    // initialize player sprite
    player.x = (MAX_X / 2) - (SPRITE_WIDTH / 2);
    player.y = (MAX_Y / 2) - (SPRITE_HEIGHT / 2);
    player.tile_index = SPRITE_PLAYER;

    // initialize pause sprites
    pause_sprites[0].x = 0x70;
    pause_sprites[0].y = 0xef;
    pause_sprites[0].tile_index = (uint8_t) 'P';
    pause_sprites[0].attributes = 0x01;
    pause_sprites[1].x = 0x78;
    pause_sprites[1].y = 0xef;
    pause_sprites[1].tile_index = (uint8_t) 'A';
    pause_sprites[1].attributes = 0x01;
    pause_sprites[2].x = 0x80;
    pause_sprites[2].y = 0xef;
    pause_sprites[2].tile_index = (uint8_t) 'U';
    pause_sprites[2].attributes = 0x01;
    pause_sprites[3].x = 0x88;
    pause_sprites[3].y = 0xef;
    pause_sprites[3].tile_index = (uint8_t) 'S';
    pause_sprites[3].attributes = 0x01;
    pause_sprites[4].x = 0x90;
    pause_sprites[4].y = 0xef;
    pause_sprites[4].tile_index = (uint8_t) 'E';
    pause_sprites[4].attributes = 0x01;

    // turn on rendering
    ResetScroll();
    EnablePPU();

    game_state = STATE_GAME;

    while (1) {
        WaitFrame();
        ResetScroll();

        switch(game_state) {
            case STATE_GAME: {
                if ((InputPort1 & BUTTON_START) && !(InputPort1Prev & BUTTON_START)) {
                    game_state = STATE_PAUSE;
                    for (i = 0; i < sizeof(pause_sprites) / sizeof(sprite_t); ++i) {
                        pause_sprites[i].y = 0x70;
                    }
                    break;
                }

                if (InputPort1 & BUTTON_UP) {
                    if (player.y > MIN_Y + SPRITE_HEIGHT) {
                        --player.y;
                    }
                }

                if (InputPort1 & BUTTON_DOWN) {
                    if (player.y < MAX_Y - (2 * SPRITE_HEIGHT)) {
                        ++player.y;
                    }
                }

                if (InputPort1 & BUTTON_LEFT) {
                    if (player.x > MIN_X + SPRITE_WIDTH) {
                        --player.x;
                    }
                }

                if (InputPort1 & BUTTON_RIGHT) {
                    if (player.x < MAX_X - (2 * SPRITE_WIDTH)) {
                        ++player.x;
                    }
                }
            }
                break;
            case STATE_PAUSE: {
                if ((InputPort1 & BUTTON_START) && !(InputPort1Prev & BUTTON_START)) {
                    game_state = STATE_GAME;
                    for (i = 0; i < sizeof(pause_sprites) / sizeof(sprite_t); ++i) {
                        pause_sprites[i].y = 0xef;
                    }
                }
            }
                break;
        } // switch(game_state)
    };
};

