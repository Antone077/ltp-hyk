__asm__("/* nothing */");
/*
 * check-name: asm-toplevel.c
 * check-command: test-linearize $file
 * check-known-to-fail
 * check-output-ignore
 * check-output-contains: asm *".. nothing .."
 */
