/* vera_demo.c -- Simple VERA demo for Apple II
 * Fills the default VERA framebuffer with a color pattern.
 */

#include <stdio.h>
#include <stdint.h>
#include "../vera_detect.h"

typedef volatile uint8_t vreg8;

/* Set VERA address for reading/writing */
static void vera_set_addr(uint32_t addr)
{
    uint16_t base = vera_get_base();
    *(vreg8 *)(base + 0) = (uint8_t)(addr & 0xFF);
    *(vreg8 *)(base + 1) = (uint8_t)((addr >> 8) & 0xFF);
    *(vreg8 *)(base + 2) = (uint8_t)((addr >> 16) & 0x01);
    *(vreg8 *)(base + 5) = 0x00;
}

/* Fill default VRAM framebuffer (first 64KB) with a pattern */
static void vera_fill_framebuffer(void)
{
    uint16_t addr;
    uint8_t color;
    uint16_t base = vera_get_base();
    
    printf("Filling VERA framebuffer...\n");
    vera_set_addr(0);
    
    for (addr = 0; addr < 0x10000; addr++) {
        color = (uint8_t)((addr >> 8) ^ (addr & 0xFF) ^ ((addr >> 4) & 0x0F));
        *(vreg8 *)(base + 3) = color;  /* DATA0 at offset 3 */
        
        if ((addr & 0x2000) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf("\nDone!\n");
}

int main(void)
{
    int slot;
    
    printf("VERA Framebuffer Demo for Apple II\n");
    printf("===================================\n\n");
    
    printf("Detecting VERA card...");
    fflush(stdout);
    
    slot = vera_detect();
    if (slot == 0) {
        printf(" NOT FOUND\n");
        printf("VERA card not detected in slots 1-7\n");
        return 1;
    }
    
    printf(" found in slot %d\n\n", slot);
    
    printf("Writing to VERA VRAM...\n");
    vera_fill_framebuffer();
    
    printf("\nYou should see patterns on the VERA window.\n");
    printf("Press RETURN to exit\n");
    getchar();
    
    return 0;
}
