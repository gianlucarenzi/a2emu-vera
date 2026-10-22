/* test_fx.c — VERA FX coprocessor register / behaviour test.
 *
 * Port of VERA_C64/tests/test_fx.c.  Booted by sdk/boot.s, no DOS: finds the
 * A2VERA card in any slot; output goes to the Apple screen, not VERA.
 *
 * CTRL bit 7 is NOT used: on real VERA it triggers an FPGA reconfigure.
 * DCSEL=0/1 registers (DC_VIDEO, DC_HSCALE…) are not relevant here.
 *
 * Tests:
 *  2. FX_CTRL (DCSEL=2) write/read roundtrip
 *  3. All other FX regs are write-only: reads return the firmware identity
 *     bytes of DCSEL 63 ('V',major,minor,build: 'V',48,0,1 on 48.0.1)
 *  9. Multiplier: A*B result written to VRAM via DATA0
 * 10. Transparency: zero byte skipped, non-zero written
 *
 * Benchmark ticks are VERA frames (59.5 Hz): the Apple has no clock, so the
 * loops poll ISR VSYNC every 256 iterations (fewer than one frame's worth).
 *
 * Build: see Makefile.  main never returns.
 */

#include <conio.h>
#include "vera.h"

/* CTRL DCSEL field values (bits [6:1]) */
#define DCSEL_0   0x00   /* DC_VIDEO … DC_BORDER      (owned by VERA.SYS) */
#define DCSEL_1   0x02   /* DC_HSTART … DC_VSTOP                          */
#define DCSEL_2   0x04   /* FX_CTRL, FX_TILEBASE, FX_MAPBASE, FX_MULT     */
#define DCSEL_3   0x06   /* FX_X_INCR_L/H, FX_Y_INCR_L/H                  */
#define DCSEL_4   0x08   /* FX_X/Y_POS_L/H (integer part)                  */
#define DCSEL_5   0x0A   /* FX_X/Y_POS_S (subpixel), FX_POLY_FILL_L/H     */
#define DCSEL_6   0x0C   /* FX cache bytes / accumulator side-effects       */

/* FX_CTRL bitmasks */
#define FX_TRANSP          0x80
#define FX_CACHE_WR_EN     0x40
#define FX_CACHE_FILL_EN   0x20
#define FX_ONE_BYTE_CACHE  0x10
#define FX_16BIT_HOP       0x08
#define FX_4BIT_MODE       0x04

/* A read of DATA0 for its side effects only (cache fill).  cc65 -O drops a
 * read whose value is discarded; the card's address is known at run time
 * only, so the value goes to a global the optimizer must keep. */
static volatile unsigned char g_sink;
#define READ_DATA0() (g_sink = VERA_DATA0)

/* Safe VRAM window for testing: 64 KB in bank 0 */
#define TEST_VRAM_SRC   0x00000UL
#define TEST_VRAM_DST   0x08000UL   /* 32768 is 4-byte aligned (0x8000 % 4 == 0) */
#define TEST_VRAM_MULT  0x00100UL   /* 4-byte area for multiplier test    */
#define TEST_VRAM_TRANS 0x00110UL   /* 1-byte area for transparency test  */
#define BENCH_SIZE      16384       /* 16 KB for benchmarking             */

/* the timer: VERA frames, counted by polling ISR VSYNC (TICK every 256
 * iterations of a benchmark loop) */
static unsigned int g_ticks;

static void tick(void)
{
    if (VERA_ISR & VERA_ISR_VSYNC) {
        VERA_ISR = VERA_ISR_VSYNC;
        ++g_ticks;
    }
}
#define TICK(i) do { if (((unsigned char)(i)) == 0) tick(); } while (0)

static void start_timer(void)
{
    /* Wait for frame start to maximize resolution */
    vera_wait_vsync();
    VERA_ISR = VERA_ISR_VSYNC;
    g_ticks = 0;
}

static unsigned int end_timer(void)
{
    tick();
    return g_ticks;
}

/* ------------------------------------------------------------------ */
/* Test scaffolding                                                     */
/* ------------------------------------------------------------------ */
static unsigned char g_pass = 0;
static unsigned char g_fail = 0;

static unsigned char g_lines = 0;
static const char *g_section = "";

/* Only FAIL lines are printed (OK checks are just counted), and the output
 * pauses every 20 lines so nothing scrolls away unread. */
static void check_b(const char *name, unsigned char got, unsigned char expected)
{
    if (got == expected)
    {
        g_pass++;
        return;
    }
    if (g_lines >= 20)
    {
        cprintf("-- press a key --\r\n");
        cgetc();
        g_lines = 0;
    }
    cprintf("FAIL %s %s\r\n     got $%02X exp $%02X\r\n", g_section, name,
           (unsigned int)got, (unsigned int)expected);
    g_lines += 2;
    g_fail++;
}

/* ------------------------------------------------------------------ */
/* VRAM helpers (no FX modes, ADDRSEL=0)                               */
/* ------------------------------------------------------------------ */

/* ADDR_H auto-increment: bits[7:4]; bank in bit[0] */
#define ADDR_H_NOINC_B0  0x00   /* bank=0, no increment    */
#define ADDR_H_INC1_B0   0x10   /* bank=0, +1 per access   */

static void set_addr0(unsigned long addr, unsigned char addr_h)
{
    VERA_CTRL  = DCSEL_0;   /* ADDRSEL=0, DCSEL=0 */
    VERA_ADDR_L = (unsigned char)(addr & 0xFF);
    VERA_ADDR_M = (unsigned char)((addr >> 8) & 0xFF);
    VERA_ADDR_H = addr_h;
}

static void vram_write(unsigned long addr, unsigned char value)
{
    set_addr0(addr, ADDR_H_NOINC_B0);
    vera_w0(value);
}

static unsigned char vram_read(unsigned long addr)
{
    set_addr0(addr, ADDR_H_NOINC_B0);
    return VERA_DATA0;
}

/* ------------------------------------------------------------------ */
/* Test 2: FX_CTRL (DCSEL=2) write/read roundtrip                      */
/* ------------------------------------------------------------------ */
static void test_fx_ctrl(void)
{
    unsigned char v;

    g_section = "[2]";

    VERA_CTRL  = DCSEL_2;
    /* transparency=1, 4bit=1, mode=NORMAL → 0x84 */
    VERA_POKE(0x09, 0x84);
    v = VERA_REG09;
    check_b("FX_CTRL=0x84 r/b", v, 0x84);

    VERA_POKE(0x09, 0x00);
    v = VERA_REG09;
    check_b("FX_CTRL=0x00 r/b", v, 0x00);

    VERA_CTRL = DCSEL_0;
}

/* ------------------------------------------------------------------ */
/* Test 3: FX registers are WRITE-ONLY on the real VERA                */
/* Only FX_CTRL (DCSEL=2,$09) and POLY_FILL_L/H (DCSEL=5,$0B/$0C) are   */
/* readable.  Every other DCSEL>=2 read returns the firmware identity   */
/* bytes, the same as DCSEL 63: 'V',major,minor,build ('V',47,0,0 on    */
/* 47.0.2, 'V',48,0,1 on 48.0.1).  Writes are exercised as a smoke      */
/* test; their effect is verified by the behavioural tests below.       */
/* ------------------------------------------------------------------ */
static unsigned char g_id[4];   /* DCSEL 63 identity: 'V', major, minor, build */

static void check_wo(unsigned char dcsel, const char *tag,
                     unsigned char r09, unsigned char r0a,
                     unsigned char r0b, unsigned char r0c)
{
    VERA_CTRL = dcsel;
    if (r09) check_b(tag, VERA_REG09, g_id[0]);
    if (r0a) check_b(tag, VERA_REG0A, g_id[1]);
    if (r0b) check_b(tag, VERA_REG0B, g_id[2]);
    if (r0c) check_b(tag, VERA_REG0C, g_id[3]);
}

static void test_write_only(void)
{
    g_section = "[3]";

    /* Firmware identity from DCSEL 63 */
    VERA_CTRL = 0x7E;
    g_id[0] = VERA_REG09;
    g_id[1] = VERA_REG0A;
    g_id[2] = VERA_REG0B;
    g_id[3] = VERA_REG0C;
    VERA_CTRL = DCSEL_0;
    check_b("DC63 'V'", g_id[0], 0x56);     /* 'V' in ASCII, not PETSCII */
    check_b("DC63 major >= 47", g_id[1] >= 47 ? g_id[1] : 0, g_id[1]);
    cprintf("VERA firmware %u.%u.%u\r\n",
           (unsigned int)g_id[1], (unsigned int)g_id[2], (unsigned int)g_id[3]);

    /* DCSEL=2: only $09 (FX_CTRL) is readable */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x0A, 0xFD);
    VERA_POKE(0x0B, 0x93);
    VERA_POKE(0x0C, 0x3D);
    check_wo(DCSEL_2, "DC2 $0A/$0B/$0C", 0, 1, 1, 1);
    VERA_CTRL = DCSEL_2; VERA_POKE(0x0A, 0); VERA_POKE(0x0B, 0); VERA_POKE(0x0C, 0);

    /* DCSEL=3 INCR */
    VERA_CTRL = DCSEL_3;
    VERA_POKE(0x09, 0x34); VERA_POKE(0x0A, 0x01); VERA_POKE(0x0B, 0xAB); VERA_POKE(0x0C, 0x82);
    check_wo(DCSEL_3, "DC3 INCR", 1, 1, 1, 1);
    VERA_CTRL = DCSEL_3;
    VERA_POKE(0x09, 0); VERA_POKE(0x0A, 0); VERA_POKE(0x0B, 0); VERA_POKE(0x0C, 0);

    /* DCSEL=4 POS integer */
    VERA_CTRL = DCSEL_4;
    VERA_POKE(0x09, 0x50); VERA_POKE(0x0A, 0x02); VERA_POKE(0x0B, 0xC0); VERA_POKE(0x0C, 0x85);
    check_wo(DCSEL_4, "DC4 POS", 1, 1, 1, 1);
    VERA_CTRL = DCSEL_4;
    VERA_POKE(0x09, 0); VERA_POKE(0x0A, 0); VERA_POKE(0x0B, 0); VERA_POKE(0x0C, 0);

    /* DCSEL=5: $09/$0A readable as identity; $0B/$0C = POLY_FILL (live) */
    VERA_CTRL = DCSEL_5;
    VERA_POKE(0x09, 0x3C); VERA_POKE(0x0A, 0x3C);
    check_wo(DCSEL_5, "DC5 POS_S", 1, 1, 0, 0);
    VERA_CTRL = DCSEL_5; VERA_POKE(0x09, 0); VERA_POKE(0x0A, 0);

    /* DCSEL=6 cache: reads of $09/$0A have side effects but return identity */
    VERA_CTRL = DCSEL_6;
    VERA_POKE(0x09, 0xAA); VERA_POKE(0x0A, 0xBB); VERA_POKE(0x0B, 0xCC); VERA_POKE(0x0C, 0xDD);
    check_wo(DCSEL_6, "DC6 CACHE", 1, 1, 1, 1);

    VERA_CTRL = DCSEL_0;
}

/* ------------------------------------------------------------------ */
/* Test 9: Multiplier — cache-write + mult writes A*B to VRAM          */
/* A = 3 (cache[1:0]), B = 4 (cache[3:2]) → A*B = 12 = 0x0C           */
/* Result written as 4 bytes little-endian at TEST_VRAM_MULT.          */
/* ------------------------------------------------------------------ */
static void test_multiplier(void)
{
    unsigned char r0, r1, r2, r3;

    g_section = "[9]";

    /* Pre-fill target area with 0xFF to make wrong results visible */
    set_addr0(TEST_VRAM_MULT, ADDR_H_INC1_B0);
    vera_w0(0xFF);
    vera_w0(0xFF);
    vera_w0(0xFF);
    vera_w0(0xFF);

    /* FX_CTRL: cache_write_enabled=1, all others 0 → 0x40 */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, 0x40);

    /* FX_MULT: bit7=ResetAccum resets accumulator to 0; bit4=mult_enabled → 0x90 */
    VERA_POKE(0x0C, 0x90);

    /* Cache: A=3 (low word), B=4 (high word) */
    VERA_CTRL  = DCSEL_6;
    VERA_POKE(0x09, 0x03);   /* cache[0] = A_lo */
    VERA_POKE(0x0A, 0x00);   /* cache[1] = A_hi */
    VERA_POKE(0x0B, 0x04);   /* cache[2] = B_lo */
    VERA_POKE(0x0C, 0x00);   /* cache[3] = B_hi */

    /* Write triggers vera_fx_write_data: 3*4=12 stored as 4-byte LE */
    set_addr0(TEST_VRAM_MULT, ADDR_H_NOINC_B0);
    vera_w0(0x00);

    /* Disable cache-write mode before reading back */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, 0x00);
    VERA_POKE(0x0C, 0x00);

    /* Read back the 4 bytes written by the multiplier */
    set_addr0(TEST_VRAM_MULT, ADDR_H_INC1_B0);
    r0 = VERA_DATA0;
    r1 = VERA_DATA0;
    r2 = VERA_DATA0;
    r3 = VERA_DATA0;

    VERA_CTRL    = DCSEL_0;

    check_b("VRAM+0 = 0x0C (3*4)", r0, 0x0C);
    check_b("VRAM+1 = 0x00",       r1, 0x00);
    check_b("VRAM+2 = 0x00",       r2, 0x00);
    check_b("VRAM+3 = 0x00",       r3, 0x00);
}

/* ------------------------------------------------------------------ */
/* Test 10: Transparency — zero byte skipped, non-zero written (8-bit) */
/* ------------------------------------------------------------------ */
static void test_transparency(void)
{
    unsigned char v;

    g_section = "[10]";

    /* Write sentinel 0x55 with transparency disabled */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, 0x00);
    vram_write(TEST_VRAM_TRANS, 0x55);
    v = vram_read(TEST_VRAM_TRANS);
    check_b("sentinel 0x55 written", v, 0x55);

    /* Enable transparency: FX_CTRL bit7=1 */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, 0x80);

    /* Write 0x00 — must be skipped */
    vram_write(TEST_VRAM_TRANS, 0x00);
    v = vram_read(TEST_VRAM_TRANS);
    check_b("zero skipped (still 0x55)", v, 0x55);

    /* Write 0xAA — non-zero, must pass through */
    vram_write(TEST_VRAM_TRANS, 0xAA);
    v = vram_read(TEST_VRAM_TRANS);
    check_b("non-zero 0xAA written", v, 0xAA);

    /* Restore */
    VERA_CTRL    = DCSEL_2;
    VERA_POKE(0x09, 0x00);
    VERA_CTRL    = DCSEL_0;
}

/* ------------------------------------------------------------------ */
/* Test 11: Manual Cache Load + Write (Verify Write Path)              */
/* ------------------------------------------------------------------ */
static void test_cache_manual(void)
{
    unsigned char r0, r1, r2, r3;

    g_section = "[11]";

    /* 1. Load cache manually via DCSEL_6 */
    VERA_CTRL  = DCSEL_6;
    VERA_POKE(0x09, 0xDE);
    VERA_POKE(0x0A, 0xAD);
    VERA_POKE(0x0B, 0xBE);
    VERA_POKE(0x0C, 0xEF);

    /* 2. Setup DST address, INC4, ADDRSEL=1 */
    VERA_CTRL  = 0x01; /* ADDRSEL=1, DCSEL=0 */
    VERA_ADDR_L = (unsigned char)(TEST_VRAM_DST & 0xFF);
    VERA_ADDR_M = (unsigned char)((TEST_VRAM_DST >> 8) & 0xFF);
    VERA_ADDR_H = VERA_INC4 | (unsigned char)((TEST_VRAM_DST >> 16) & 0x01);

    /* 3. Enable Cache Write */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, FX_CACHE_WR_EN);

    /* 4. Flush cache to VRAM */
    vera_w1(0);

    /* 5. Disable FX and verify */
    VERA_POKE(0x09, 0x00);
    r0 = vram_read(TEST_VRAM_DST + 0);
    r1 = vram_read(TEST_VRAM_DST + 1);
    r2 = vram_read(TEST_VRAM_DST + 2);
    r3 = vram_read(TEST_VRAM_DST + 3);

    check_b("DST+0 = 0xDE", r0, 0xDE);
    check_b("DST+1 = 0xAD", r1, 0xAD);
    check_b("DST+2 = 0xBE", r2, 0xBE);
    check_b("DST+3 = 0xEF", r3, 0xEF);
}

/* ------------------------------------------------------------------ */
/* Test 12: Cache Fill + Write (The 4:1 Pattern)                      */
/* ------------------------------------------------------------------ */
static void test_cache_copy(void)
{
    unsigned char r0, r1, r2, r3;

    g_section = "[12]";

    /* Prepare source data */
    vram_write(TEST_VRAM_SRC + 0, 0xA1);
    vram_write(TEST_VRAM_SRC + 1, 0xB2);
    vram_write(TEST_VRAM_SRC + 2, 0xC3);
    vram_write(TEST_VRAM_SRC + 3, 0xD4);
    
    /* Clear destination */
    vram_write(TEST_VRAM_DST + 0, 0x00);
    vram_write(TEST_VRAM_DST + 1, 0x00);
    vram_write(TEST_VRAM_DST + 2, 0x00);
    vram_write(TEST_VRAM_DST + 3, 0x00);

    /* 1. Reset Cache Index to 0 via FX_MULT (bits 3:2 = 00) */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x0C, 0x00); 
    
    /* 2. Enable FX Cache Fill and Write */
    VERA_POKE(0x09, FX_CACHE_FILL_EN | FX_CACHE_WR_EN);

    /* 3. Set ADDR0 to source, INC1 */
    VERA_CTRL  = DCSEL_0;   /* ADDRSEL=0 */
    VERA_ADDR_L = (unsigned char)(TEST_VRAM_SRC & 0xFF);
    VERA_ADDR_M = (unsigned char)((TEST_VRAM_SRC >> 8) & 0xFF);
    VERA_ADDR_H = VERA_INC1 | (unsigned char)((TEST_VRAM_SRC >> 16) & 0x01);

    /* 4. Set ADDR1 to destination, INC4 */
    VERA_CTRL  = 0x01;      /* ADDRSEL=1 */
    VERA_ADDR_L = (unsigned char)(TEST_VRAM_DST & 0xFF);
    VERA_ADDR_M = (unsigned char)((TEST_VRAM_DST >> 8) & 0xFF);
    VERA_ADDR_H = VERA_INC4 | (unsigned char)((TEST_VRAM_DST >> 16) & 0x01);

    /* 5. Trigger 4 reads (fills cache) */
    READ_DATA0();
    READ_DATA0();
    READ_DATA0();
    READ_DATA0();
    
    /* 6. Trigger 1 write (flushes cache) */
    vera_w1(0);

    /* Restore normal mode to read back */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, 0x00);
    
    r0 = vram_read(TEST_VRAM_DST + 0);
    r1 = vram_read(TEST_VRAM_DST + 1);
    r2 = vram_read(TEST_VRAM_DST + 2);
    r3 = vram_read(TEST_VRAM_DST + 3);

    check_b("DST+0 = 0xA1", r0, 0xA1);
    check_b("DST+1 = 0xB2", r1, 0xB2);
    check_b("DST+2 = 0xC3", r2, 0xC3);
    check_b("DST+3 = 0xD4", r3, 0xD4);

    VERA_CTRL = DCSEL_0;
}

/* ------------------------------------------------------------------ */
/* Benchmarks                                                         */
/* ------------------------------------------------------------------ */
static void run_benchmarks(void)
{
    unsigned int i;
    unsigned int ticks;
    unsigned int ticks_fill_inc1;
    unsigned int ticks_copy_inc1;
    unsigned int sp;

    cprintf("\r\nBenchmarks (Size: %u KB)\r\n", BENCH_SIZE / 1024);
    cprintf("--------------------------\r\n");

    /* 1. Baseline: Single-byte write (INC1) */
    set_addr0(TEST_VRAM_DST, VERA_INC1);
    start_timer();
    for (i = 0; i < BENCH_SIZE; ++i) {
        vera_w0(0x55);
        TICK(i);
    }
    ticks = end_timer();
    ticks_fill_inc1 = ticks;
    cprintf("Fill (INC1): %3u ticks\r\n", ticks);

    /* 2. Optimized: FX Cache fill (INC4, 4 bytes per write) */
    VERA_CTRL  = DCSEL_6;
    VERA_POKE(0x09, 0xAA); VERA_POKE(0x0A, 0xBB); VERA_POKE(0x0B, 0xCC); VERA_POKE(0x0C, 0xDD);
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, FX_CACHE_WR_EN);
    set_addr0(TEST_VRAM_DST, VERA_INC4);
    start_timer();
    for (i = 0; i < BENCH_SIZE / 4; ++i) {
        vera_w0(0x00);
        TICK(i);
    }
    ticks = end_timer();
    sp = (ticks > 0) ? (unsigned int)((ticks_fill_inc1 * 10u) / ticks) : 0;
    cprintf("Fill (FX 4): %3u ticks (Speedup: %u.%ux)\r\n", ticks, sp / 10, sp % 10);
    
    /* 3. Baseline Copy: Byte-by-byte (DATA0 -> DATA1, INC1) */
    VERA_CTRL  = DCSEL_2; VERA_POKE(0x09, 0x00);
    set_addr0(TEST_VRAM_SRC, VERA_INC1);
    VERA_CTRL  = 0x01; /* ADDRSEL=1 */
    VERA_ADDR_L = (unsigned char)(TEST_VRAM_DST & 0xFF);
    VERA_ADDR_M = (unsigned char)((TEST_VRAM_DST >> 8) & 0xFF);
    VERA_ADDR_H = VERA_INC1;
    start_timer();
    for (i = 0; i < BENCH_SIZE; ++i) {
        vera_w1(VERA_DATA0);
        TICK(i);
    }
    ticks = end_timer();
    ticks_copy_inc1 = ticks;
    cprintf("Copy (INC1): %3u ticks\r\n", ticks);

    /* 4. FX Cache Copy: (4x DATA0 -> 1x DATA1, INC4, 4 bytes per transfer) */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, FX_CACHE_FILL_EN | FX_CACHE_WR_EN);
    set_addr0(TEST_VRAM_SRC, VERA_INC1);
    VERA_CTRL  = 0x01; /* ADDRSEL=1 */
    VERA_ADDR_L = (unsigned char)(TEST_VRAM_DST & 0xFF);
    VERA_ADDR_M = (unsigned char)((TEST_VRAM_DST >> 8) & 0xFF);
    VERA_ADDR_H = VERA_INC4;
    start_timer();
    for (i = 0; i < BENCH_SIZE / 4; ++i) {
        READ_DATA0();
        READ_DATA0();
        READ_DATA0();
        READ_DATA0();
        vera_w1(0);
        TICK(i);
    }
    ticks = end_timer();
    sp = (ticks > 0) ? (unsigned int)((ticks_copy_inc1 * 10u) / ticks) : 0;
    cprintf("Copy (FX 4): %3u ticks (Speedup: %u.%ux)\r\n", ticks, sp / 10, sp % 10);

    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, 0x00);
    VERA_CTRL  = DCSEL_0;
}

/* ------------------------------------------------------------------ */

int main(void)
{
    clrscr();
    if (!vera_detect()) {
        cputs("VERA card not found in slots 1-7\r\n");
        for (;;)
            ;
    }

    /* NOTE: do NOT write CTRL bit 7: on real VERA it reconfigures the whole
     * FPGA (reloads the bitstream, VRAM lost, bus dead for ~100 ms).
     * Start from a known FX state by writing the registers instead. */
    VERA_CTRL  = DCSEL_2;
    VERA_POKE(0x09, 0x00);
    VERA_POKE(0x0C, 0x80);      /* ResetAccum trigger, mult/accum off */
    VERA_CTRL  = DCSEL_0;

    cprintf("VERA FX coprocessor test\r\n");
    cprintf("========================\r\n");

    test_fx_ctrl();
    test_write_only();
    test_multiplier();
    test_transparency();
    test_cache_manual();
    test_cache_copy();

    run_benchmarks();

    cprintf("\r\n========================\r\n");
    cprintf("PASS: %d   FAIL: %d\r\n",
           (unsigned int)g_pass, (unsigned int)g_fail);
    for (;;)
        ;
}
