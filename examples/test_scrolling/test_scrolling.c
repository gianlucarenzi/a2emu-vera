/* test_scrolling.c - VERA layer scrolling test for Apple II
 *
 * Tests the VERA scrolling registers (HSCROLL/VSCROLL) by filling
 * the framebuffer with a pattern and scrolling it.
 * 
 * Ported from C64 VERA_C64/tests/test_gradient_scroll.c
 */

#include <stdio.h>
#include <stdint.h>
#include "../vera_detect.h"

typedef volatile uint8_t vreg8;

/* VERA register offsets */
#define VERA_ADDR_L     0
#define VERA_ADDR_M     1
#define VERA_ADDR_H     2
#define VERA_DATA0      3
#define VERA_DATA1      4
#define VERA_CTRL       5

/* DC (Display Controller) register offsets - need DCSEL in CTRL */
#define DC_VIDEO        0x09    /* DCSEL=0, offset 0 */
#define DC_HSCALE       0x0A    /* DCSEL=0 */
#define DC_VSCALE       0x0B    /* DCSEL=0 */
#define DC_BORDER       0x0F    /* DCSEL=0 */

#define DC_HSTART       0x09    /* DCSEL=1 */
#define DC_HSTOP        0x0A    /* DCSEL=1 */
#define DC_VSTART       0x0B    /* DCSEL=1 */
#define DC_VSTOP        0x0C    /* DCSEL=1 */

#define DC_L0_CONFIG    0x09    /* DCSEL=2 */
#define DC_L0_MAPBASE   0x0A    /* DCSEL=2 */
#define DC_L0_TILEBASE  0x0B    /* DCSEL=2 */

#define DC_L0_HSCROLL_L 0x09    /* DCSEL=3 */
#define DC_L0_HSCROLL_H 0x0A    /* DCSEL=3 */
#define DC_L0_VSCROLL_L 0x0B    /* DCSEL=3 */
#define DC_L0_VSCROLL_H 0x0C    /* DCSEL=3 */

/* Write to VERA register via DC (Display Controller) */
static void vera_dc_write(uint16_t dcsel, uint8_t reg_offset, uint8_t value)
{
    uint16_t base = vera_get_base();
    uint8_t ctrl = (uint8_t)((dcsel << 1) & 0x7E);  /* DCSEL in bits [6:1] */
    
    *(vreg8 *)(base + VERA_CTRL) = ctrl;
    *(vreg8 *)(base + reg_offset) = value;
}

/* Read from VERA register via DC */
static uint8_t vera_dc_read(uint16_t dcsel, uint8_t reg_offset)
{
    uint16_t base = vera_get_base();
    uint8_t ctrl = (uint8_t)((dcsel << 1) & 0x7E);
    
    *(vreg8 *)(base + VERA_CTRL) = ctrl;
    return *(vreg8 *)(base + reg_offset);
}

/* Set VERA VRAM address and auto-increment */
static void vera_set_addr(uint32_t addr, uint8_t incr)
{
    uint16_t base = vera_get_base();
    *(vreg8 *)(base + VERA_CTRL) = 0;
    *(vreg8 *)(base + VERA_ADDR_L) = (uint8_t)(addr & 0xFF);
    *(vreg8 *)(base + VERA_ADDR_M) = (uint8_t)((addr >> 8) & 0xFF);
    *(vreg8 *)(base + VERA_ADDR_H) = (uint8_t)(((addr >> 16) & 0x01) | ((incr & 0x0F) << 4));
}

/* Fill framebuffer with gradient pattern */
static void fill_gradient(void)
{
    uint16_t addr;
    uint8_t row, col, color;
    uint16_t base = vera_get_base();
    
    printf("Filling VERA framebuffer with gradient pattern...\n");
    vera_set_addr(0, 0);  /* Start at address 0, no auto-increment */
    
    for (addr = 0; addr < 0x10000; addr++) {
        /* Create diagonal gradient */
        row = (addr >> 8) & 0xFF;
        col = addr & 0xFF;
        color = (uint8_t)((row + col) & 0x0F);
        
        *(vreg8 *)(base + VERA_DATA0) = color;
        
        if ((addr & 0x2000) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf(" Done!\n");
}

/* Set layer 0 scroll position */
static void set_scroll(uint16_t h_scroll, uint16_t v_scroll)
{
    /* HSCROLL: 12-bit register (DCSEL=3, offsets 0x09/0x0A) */
    vera_dc_write(3, DC_L0_HSCROLL_L, (uint8_t)(h_scroll & 0xFF));
    vera_dc_write(3, DC_L0_HSCROLL_H, (uint8_t)((h_scroll >> 8) & 0x0F));
    
    /* VSCROLL: 12-bit register (DCSEL=3, offsets 0x0B/0x0C) */
    vera_dc_write(3, DC_L0_VSCROLL_L, (uint8_t)(v_scroll & 0xFF));
    vera_dc_write(3, DC_L0_VSCROLL_H, (uint8_t)((v_scroll >> 8) & 0x0F));
}

int main(void)
{
    int slot;
    uint16_t hscroll = 0, vscroll = 0;
    uint8_t speed = 1;
    int i;
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║  VERA Scrolling Test                   ║\n");
    printf("║  Apple II Edition                      ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    printf("Detecting VERA card...");
    fflush(stdout);
    
    slot = vera_detect();
    if (slot == 0) {
        printf(" NOT FOUND\n");
        printf("VERA card not detected in slots 1-7\n");
        return 1;
    }
    
    printf(" found in slot %d\n\n", slot);
    
    printf("This test fills the VERA framebuffer with a diagonal\n");
    printf("gradient pattern and demonstrates scrolling by changing\n");
    printf("the HSCROLL and VSCROLL registers.\n\n");
    
    /* Fill framebuffer with gradient */
    fill_gradient();
    printf("\n");
    
    /* Test scrolling */
    printf("Testing scroll operations...\n");
    printf("Scrolling right: ");
    fflush(stdout);
    for (i = 0; i < 20; i++) {
        hscroll += 4;
        if (hscroll > 256) hscroll = 0;
        set_scroll(hscroll, vscroll);
        printf(".");
        fflush(stdout);
    }
    printf(" Done\n");
    
    printf("Scrolling down:  ");
    fflush(stdout);
    for (i = 0; i < 20; i++) {
        vscroll += 4;
        if (vscroll > 256) vscroll = 0;
        set_scroll(hscroll, vscroll);
        printf(".");
        fflush(stdout);
    }
    printf(" Done\n");
    
    printf("Scrolling left:  ");
    fflush(stdout);
    for (i = 0; i < 20; i++) {
        if (hscroll >= 4) {
            hscroll -= 4;
        } else {
            hscroll = 256 - (4 - hscroll);
        }
        set_scroll(hscroll, vscroll);
        printf(".");
        fflush(stdout);
    }
    printf(" Done\n");
    
    printf("Scrolling up:    ");
    fflush(stdout);
    for (i = 0; i < 20; i++) {
        if (vscroll >= 4) {
            vscroll -= 4;
        } else {
            vscroll = 256 - (4 - vscroll);
        }
        set_scroll(hscroll, vscroll);
        printf(".");
        fflush(stdout);
    }
    printf(" Done\n");
    
    /* Return to origin */
    printf("Restoring scroll to origin...\n");
    set_scroll(0, 0);
    
    printf("\nScrolling test completed!\n");
    printf("If you saw the pattern move on the VERA window,\n");
    printf("the scrolling registers are working correctly.\n");
    
    printf("\nPress RETURN to exit\n");
    getchar();
    
    return 0;
}
