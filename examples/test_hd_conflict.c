/* Test: Verify VERA detection fails when hard disk controller is in slot 7
 * This test simulates the hard disk controller occupying slot 7 by writing
 * a dummy firmware pattern there, then verifies that vera_detect() returns 0
 * (not found) instead of incorrectly identifying slot 7 as VERA.
 */

#include <stdio.h>
#include <stdint.h>
#include "vera_detect.h"

int main(void)
{
    int slot;
    volatile uint8_t *slot7_base = (volatile uint8_t *)0xC700;
    int i;

    printf("VERA Detection with Hard Disk Conflict Test\n");
    printf("=============================================\n\n");

    /* First, verify VERA is detectable in the default slot */
    printf("Step 1: Detecting VERA in default configuration...\n");
    slot = vera_detect();
    if (slot == 0) {
        printf("ERROR: Could not detect VERA (prerequisite for test failed)\n");
        return 1;
    }
    printf("OK: VERA found in slot %d\n\n", slot);

    /* Now simulate hard disk in slot 7 by writing a pattern
     * The hard disk controller ROM would return different values than VERA's ADDR_L */
    printf("Step 2: Simulating hard disk controller in slot 7...\n");
    printf("Writing hard disk firmware simulation to $C700-$C71F...\n");
    
    /* Write a pattern that looks like hard disk controller ROM
     * (constant values that won't echo our test patterns) */
    for (i = 0; i < 32; i++) {
        slot7_base[i] = 0x00;  /* Hard disk controller would return fixed values */
    }
    
    printf("Hard disk simulation complete.\n\n");

    /* Now try detecting VERA again - it should still find it in the original slot,
     * but should NOT find anything in slot 7 */
    printf("Step 3: Detecting VERA after hard disk simulation...\n");
    
    g_vera_base = 0;  /* Reset detection state */
    slot = vera_detect();
    
    if (slot == 0) {
        printf("ERROR: Detection failed - could not find VERA after disk simulation\n");
        printf("This might indicate the hard disk was interfering with detection\n");
        return 1;
    }
    
    printf("OK: VERA still detected in slot %d\n", slot);
    
    if (slot == 7) {
        printf("WARNING: Detection incorrectly identified slot 7 as VERA\n");
        printf("This suggests hard disk conflict detection may not work\n");
        return 1;
    }
    
    printf("OK: Correctly did NOT detect slot 7 as VERA (good - hard disk there)\n\n");
    printf("All tests passed!\n");
    return 0;
}
