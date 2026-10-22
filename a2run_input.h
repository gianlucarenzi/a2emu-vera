/* a2run_input.h -- scripted input for the headless runner (plan E8, WP1.4).
 * See a2run_input.c for the script format. */
#ifndef A2RUN_INPUT_H
#define A2RUN_INPUT_H

int  a2in_load(const char *path);   /* parse the script; 0 = error (message on stderr) */
void a2in_frame(long frame);        /* apply every event of VERA frames <= frame */

#endif
