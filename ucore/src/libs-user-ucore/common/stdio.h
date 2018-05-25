#ifndef __LIBS_STDIO_H__
#define __LIBS_STDIO_H__

#include <types.h>
#include <stdarg.h>

/*
  For user libs, we ensure that console-print is thread-safe.
  But fprintf and snprintf is not thread-safe. We doesn't provide lock for them 
  because it is inefficient. User should lock them if necessarry.
*/
sem_t cprintf_lock;

/* kern/libs/stdio.c */
int cprintf(const char *fmt, ...);
int vcprintf(const char *fmt, va_list ap);
void cputchar(int c);
int cputs(const char *str);
int getchar(void);

/* kern/libs/readline.c */
char *readline(const char *prompt);

/* libs/printfmt.c */
void printfmt(void (*putch) (int, void *, int), int fd, void *putdat,
	      const char *fmt, ...);
void vprintfmt(void (*putch) (int, void *, int), int fd, void *putdat,
	       const char *fmt, va_list ap);
int snprintf(char *str, size_t size, const char *fmt, ...);
int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);

#endif /* !__LIBS_STDIO_H__ */
