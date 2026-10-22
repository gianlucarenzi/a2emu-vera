/* vera_detect.h -- Runtime VERA slot detection for Apple II
 * 
 * Probes slots 1-7 to find the VERA card by attempting to access
 * its ADDR_L register. Once found, provides a consistent base address
 * for accessing all VERA I/O registers.
 *
 * Usage:
 *   int slot = vera_detect();
 *   if (slot == 0) {
 *       printf("VERA card not found!\n");
 *       return 1;
 *   }
 *   printf("VERA found in slot %d\n", slot);
 *   // Use vera_get_base() to get the I/O base address
 */

#ifndef VERA_DETECT_H
#define VERA_DETECT_H

#include <stdint.h>

typedef volatile uint8_t vreg8;

/* Global variable to store detected VERA base address */
static uint16_t g_vera_base = 0;

/* Probe for VERA card by testing ADDR_L register in each slot.
 * Returns slot number (1-7) if found, or 0 if not found.
 * Also sets g_vera_base to the I/O base address.
 *
 * Note: If a different controller (e.g., hard disk) occupies a slot,
 * the ADDR_L test will fail naturally because those controllers don't
 * have a register at offset 0 that echoes write values.
 */
static int vera_detect(void)
{
    int slot;
    volatile uint8_t *addr_l;
    uint8_t test_val, read_val;
    int retries;

    /* Try each slot from 1 to 7 */
    for (slot = 1; slot <= 7; slot++) {
        uint16_t base = 0xC000 + (slot << 8);
        addr_l = (volatile uint8_t *)(base + 0);  /* ADDR_L at offset 0 */

        /* Test: write a test pattern and read it back multiple times.
         * ADDR_L is a write/read register that holds the low byte of the address pointer.
         * We test with a distinctive pattern and verify consistency. */
        test_val = 0xA5;  /* Pattern: 10100101 */

        for (retries = 0; retries < 2; retries++) {
            *addr_l = test_val;
            read_val = *addr_l;

            if (read_val == test_val) {
                /* Write another test pattern to confirm it's VERA, not stale data */
                uint8_t test_val2 = 0x5A;  /* Inverse pattern: 01011010 */
                *addr_l = test_val2;
                if (*addr_l == test_val2) {
                    /* Confirmed: this is the VERA ADDR_L register */
                    g_vera_base = base;
                    return slot;
                }
            }
        }
    }

    return 0;  /* Not found */
}

/* Return the base address for VERA I/O registers.
 * Must call vera_detect() first.
 */
static uint16_t vera_get_base(void)
{
    return g_vera_base;
}

#endif /* VERA_DETECT_H */
