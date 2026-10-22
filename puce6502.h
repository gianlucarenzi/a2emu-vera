/* Puce6502 - MOS 6502 cpu emulator Last modified 1st of August 2020 Copyright (c) 2018 Arthur... */


#ifndef _PUCE6502_H
#define _PUCE6502_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef enum { false, true } bool;

/* 32-bit cycle counter (native on the m68k -- the old 64-bit `ticks` cost
 * multi-instruction adds/compares on EVERY opcode).  All consumers use ticks
 * DIFFERENCES (paddle/sound timing) or don't care (floating-bus), so the
 * ~70-minute 32-bit wrap is harmless; the exec loop compares wrap-safely. */
extern unsigned int ticks;

uint16_t puce6502Exec(unsigned int cycleCount);
void puce6502RST();
void puce6502IRQ();
void puce6502NMI();

/* Full CPU register state, for save/load snapshots. */
typedef struct { uint16_t pc; uint8_t a, x, y, sp, p; } Puce6502State;
void puce6502GetState(Puce6502State *s);
void puce6502SetState(const Puce6502State *s);

// void printRegs();
// void dasm(uint16_t address);
// void setPC(uint16_t address);
// uint16_t getPC();

#endif
