#include "UartStdio.h"

#include <stdio.h>

#include <avr/io.h>

namespace
{

FILE UartStdout;

int UartPutchar(char c, FILE* stream)
{
    if (c == '\n')
        UartPutchar('\r', stream);

    // Wait until data register is empty
    while (!(UCSR0A & (1 << UDRE0))) {}

    UDR0 = c;
    return 0;
}

}

namespace UartStdio
{

void Init()
{
    // Baudrate: 115200 @16MHz
    UBRR0 = 8;

    // The default frame settings (8-bit characters, 1 stop bit, no parity bit)
    // are fine for us

    // Enable transmitter
    UCSR0B = (1 << TXEN0);

    UartStdout.put = UartPutchar;
    UartStdout.flags = _FDEV_SETUP_WRITE;
    stdout = &UartStdout;
}

}
