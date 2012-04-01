#ifndef _CONSOLE_IO_H
#define _CONSOLE_IO_H

/* Author: Henrique Dantas */

void	term_initio();
void	term_exitio();
void	term_puts(char *s);
void	term_putchar(char c);
int	term_getchar_nb();
int	term_getchar();

#endif
