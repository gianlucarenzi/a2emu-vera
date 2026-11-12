/**
 * a2_slinky.h  --  Apple II Memory Expansion Card ("Slinky") model
 *
 * Register behaviour as the Apple card (checked against MAME's a2memexp.cpp):
 *   $C0n0  address bits 0-7    read/write
 *   $C0n1  address bits 8-15   read/write
 *   $C0n2  address bits 16-23  read/write; reads OR $F0 (Apple card)
 *   $C0n3  data: each read or write moves the address forward by one
 *          (24-bit counter; the memory repeats every SLINKY_SIZE bytes)
 *   $C0n4-$C0nE  plain registers (read back what was written)
 *   $C0nF  plain register; bit 0 = 1 maps the VERA registers over
 *          $Cn00-$Cn1F instead of the slot ROM (this card only: on the
 *          Apple card it does nothing, on the RamFactor it banks its ROM)
 *
 * An Apple reset clears all registers and the address (the VERA window goes
 * back to the ROM), never the memory.  The slot ROM (6502/src/slinky_rom.s)
 * makes the memory a ProDOS block device.
 *
 * Differences from MAME: one address counter (MAME keeps a separate copy for
 * the $C0n0-2 writes, so writing one byte after some $C0n3 accesses would
 * reload the others with stale values; a counter with byte loads keeps them).
 *
 * No ESP-IDF dependencies: also compiled by the host test (test/host).
 * Style: Allman
 */

#ifndef A2_SLINKY_H
#define A2_SLINKY_H

#include <stdint.h>
#include <stdbool.h>

#ifndef A2_INLINE
#define A2_INLINE static inline __attribute__((always_inline))
#endif

#define SLINKY_SIZE      (256u * 1024u)     /* bytes, power of two */
#define SLINKY_BLOCKS    (SLINKY_SIZE / 512u)
#define SLINKY_REG_VERA  0x0Fu              /* $C0nF */
#define SLINKY_VERA_MAP  0x01u              /* $C0nF bit 0 */

typedef struct
{
    uint32_t addr;          /* 24-bit address counter */
    uint8_t  regs[16];      /* $C0n4-$C0nF (entries 0-3 unused) */
    uint8_t *mem;           /* SLINKY_SIZE bytes */
} Slinky;

A2_INLINE void slinky_reset(Slinky *s)
{
    s->addr = 0;
    for (uint32_t i = 0; i < 16u; i++)
        s->regs[i] = 0;
}

A2_INLINE void slinky_init(Slinky *s, uint8_t *mem)
{
    s->mem = mem;
    slinky_reset(s);
}

A2_INLINE bool slinky_vera_mapped(const Slinky *s)
{
    return (s->regs[SLINKY_REG_VERA] & SLINKY_VERA_MAP) != 0;
}

/* Read of $C0n0 + reg (reg = A0-A3). */
A2_INLINE uint8_t slinky_read(Slinky *s, uint32_t reg)
{
    switch (reg)
    {
    case 0:
        return (uint8_t)s->addr;
    case 1:
        return (uint8_t)(s->addr >> 8);
    case 2:
        return (uint8_t)(s->addr >> 16) | 0xF0u;
    case 3:
    {
        const uint8_t v = s->mem[s->addr & (SLINKY_SIZE - 1u)];
        s->addr = (s->addr + 1u) & 0xFFFFFFu;
        return v;
    }
    default:
        return s->regs[reg];
    }
}

/* Write of value to $C0n0 + reg (reg = A0-A3). */
A2_INLINE void slinky_write(Slinky *s, uint32_t reg, uint8_t value)
{
    switch (reg)
    {
    case 0:
        s->addr = (s->addr & 0xFFFF00u) | value;
        break;
    case 1:
        s->addr = (s->addr & 0xFF00FFu) | ((uint32_t)value << 8);
        break;
    case 2:
        s->addr = (s->addr & 0x00FFFFu) | ((uint32_t)value << 16);
        break;
    case 3:
        s->mem[s->addr & (SLINKY_SIZE - 1u)] = value;
        s->addr = (s->addr + 1u) & 0xFFFFFFu;
        break;
    default:
        s->regs[reg] = value;
        break;
    }
}

/*
 * Apple reset detector.  The card has no /RESET input: it watches the 6502
 * fetch the reset vector, a read of $FFFC followed by a read of $FFFD in the
 * next cycle.  A JMP ($FFFC) looks the same and counts as a reset too.
 * Call once per bus cycle; returns true on the $FFFD read of a vector fetch.
 */
A2_INLINE bool a2_reset_seen(uint32_t *prev, uint32_t addr, bool read)
{
    const uint32_t cur = read ? addr : 0x10000u;    /* writes never match */
    const bool hit = (*prev == 0xFFFCu) && (cur == 0xFFFDu);
    *prev = cur;
    return hit;
}

#endif /* A2_SLINKY_H */
