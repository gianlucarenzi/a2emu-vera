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
 * Detection method: VERA's ADDR_L register at offset 0 in the slot will
 * echo whatever value is written to it. We test by:
 * 1. Write 0xA5, read back to verify
 * 2. Write 0x5A (inverse pattern), read back to verify
 * This two-pattern test avoids false positives from uninitialized or
 * zero-filled memory, and detects controllers that always return the same value.
 *
 * If a different controller (e.g., hard disk) occupies a slot, the test
 * will fail because those controllers don't have a writable register at
 * offset 0 that echoes input values.
 */
static int vera_detect(void)
{
    int slot;
    volatile uint8_t *addr_l;
    uint8_t test_val, read_val;

    /* Try each slot from 1 to 7 */
    for (slot = 1; slot <= 7; slot++) {
        uint16_t base = 0xC000 + (slot << 8);
        addr_l = (volatile uint8_t *)(base + 0);  /* ADDR_L at offset 0 */

        /* Test 1: Write 0xA5 and verify it echoes back */
        test_val = 0xA5;  /* Pattern: 10100101 */
        *addr_l = test_val;
        read_val = *addr_l;

        if (read_val != test_val) {
            continue;  /* Not VERA in this slot */
        }

        /* Test 2: Write 0x5A (inverse) and verify it echoes back.
         * This confirms the register is truly writable and not returning
         * a stale/cached value from the first write. */
        test_val = 0x5A;  /* Pattern: 01011010 (inverse) */
        *addr_l = test_val;
        read_val = *addr_l;

        if (read_val == test_val) {
            /* Confirmed: this is the VERA ADDR_L register */
            g_vera_base = base;
            return slot;
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
