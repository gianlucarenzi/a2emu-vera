/* test_font.c - VERA font display test for Apple II
 * 
 * Fills the VERA framebuffer with a pattern showing various colors
 * and patterns to test the VERA graphics capabilities.
 * 
 * This is a simplified version compared to the C64 test_font.c,
 * as Apple II doesn't have a text console on VERA - instead we fill
 * the framebuffer with patterns to verify color modes and rendering.
 */

#include <stdio.h>
#include <stdint.h>
#include "../vera_detect.h"

typedef volatile uint8_t vreg8;

/* Write to VERA register at offset */
static void vera_write_reg(uint16_t offset, uint8_t value)
{
    uint16_t base = vera_get_base();
    *(vreg8 *)(base + offset) = value;
}

/* Read from VERA register at offset */
static uint8_t vera_read_reg(uint16_t offset)
{
    uint16_t base = vera_get_base();
    return *(vreg8 *)(base + offset);
}

/* Set VERA address and write data */
static void vera_set_addr(uint32_t addr)
{
    uint16_t base = vera_get_base();
    *(vreg8 *)(base + 0) = (uint8_t)(addr & 0xFF);
    *(vreg8 *)(base + 1) = (uint8_t)((addr >> 8) & 0xFF);
    *(vreg8 *)(base + 2) = (uint8_t)((addr >> 16) & 0x01);
    *(vreg8 *)(base + 5) = 0x00;  /* CTRL: ADDRSEL=0 */
}

/* Fill framebuffer with color gradient pattern */
static void fill_color_pattern(void)
{
    uint16_t addr;
    uint8_t color;
    uint16_t base = vera_get_base();
    
    printf("Filling VERA framebuffer with color pattern...\n");
    vera_set_addr(0);
    
    /* Fill 64KB with color pattern that creates visual effect */
    for (addr = 0; addr < 0x10000; addr++) {
        /* Create pattern: vertical stripes of different colors */
        color = (uint8_t)((addr >> 8) & 0x0F);  /* Change every 256 bytes */
        *(vreg8 *)(base + 3) = color;  /* DATA0 */
        
        if ((addr & 0x2000) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf("\nDone!\n");
}

/* Fill with checkerboard pattern */
static void fill_checkerboard(void)
{
    uint16_t addr;
    uint8_t color;
    uint16_t base = vera_get_base();
    
    printf("Filling VERA framebuffer with checkerboard pattern...\n");
    vera_set_addr(0);
    
    for (addr = 0; addr < 0x10000; addr++) {
        /* Checkerboard: alternate colors based on row and column */
        uint8_t row = (addr >> 8) & 0xFF;
        uint8_t col = addr & 0xFF;
        color = ((row & 0x10) ^ (col & 0x10)) ? 0x0F : 0x00;
        *(vreg8 *)(base + 3) = color;  /* DATA0 */
        
        if ((addr & 0x2000) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf("\nDone!\n");
}

/* Fill with diagonal gradient */
static void fill_diagonal_gradient(void)
{
    uint16_t addr;
    uint8_t color;
    uint16_t base = vera_get_base();
    
    printf("Filling VERA framebuffer with diagonal gradient...\n");
    vera_set_addr(0);
    
    for (addr = 0; addr < 0x10000; addr++) {
        /* Diagonal: combine row and column for gradient effect */
        uint8_t row = (addr >> 8) & 0xFF;
        uint8_t col = addr & 0xFF;
        color = (uint8_t)((row + col) & 0x0F);
        *(vreg8 *)(base + 3) = color;  /* DATA0 */
        
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
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║  VERA Font/Graphics Display Test      ║\n");
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
    
    printf("This test fills the VERA framebuffer with various\n");
    printf("patterns to verify color rendering capabilities:\n\n");
    
    /* Test 1: Color gradient stripes */
    printf("Test 1: Vertical Color Stripes\n");
    printf("════════════════════════════════\n");
    fill_color_pattern();
    printf("Press RETURN to continue...\n");
    getchar();
    
    /* Test 2: Checkerboard pattern */
    printf("\nTest 2: Checkerboard Pattern\n");
    printf("════════════════════════════════\n");
    fill_checkerboard();
    printf("Press RETURN to continue...\n");
    getchar();
    
    /* Test 3: Diagonal gradient */
    printf("\nTest 3: Diagonal Gradient\n");
    printf("════════════════════════════════\n");
    fill_diagonal_gradient();
    printf("Press RETURN to continue...\n");
    getchar();
    
    printf("\nAll tests completed!\n");
    printf("If you see colored patterns on the VERA window,\n");
    printf("the framebuffer rendering is working correctly.\n");
    
    return 0;
}
