#ifndef __UART_H
#define __UART_H

void uart_init(void);

// use printf and friends from stdio for output

int getch(void);

int getch_wait(void);

int readline(char *line, int size);

#endif
