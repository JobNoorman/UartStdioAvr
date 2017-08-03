#include "UartStdio.h"

#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

namespace
{

FILE UartStdout;

template<size_t Size>
class Buffer
{
public:

    bool write(uint8_t byte) volatile
    {
        if (isFull())
            return false;

        buffer_[writePos_] = byte;
        writePos_ = nextPos(writePos_);
        return true;
    }

    bool read(uint8_t& byte) volatile
    {
        if (isEmpty())
            return false;

        byte = buffer_[readPos_];
        readPos_ = nextPos(readPos_);
        return true;
    }

    bool isEmpty() const volatile
    {
        return readPos_ == writePos_;
    }

    bool isFull() const volatile
    {
        return nextPos(writePos_) == readPos_;
    }

private:

    size_t nextPos(size_t pos) const volatile
    {
        return (pos + 1) % Size;
    }

    uint8_t buffer_[Size];
    size_t readPos_ = 0;
    size_t writePos_ = 0;
};

volatile Buffer<64> RxBuffer;

int UartPutchar(char c, FILE* stream)
{
    if (c == '\n')
        UartPutchar('\r', stream);

    // Wait until data register is empty
    while (!(UCSR0A & (1 << UDRE0))) {}

    UDR0 = c;
    return 0;
}

int UartGetchar(FILE*)
{
    if (SREG & (1 << SREG_I))
    {
        // If interrupts are enabled, keep trying to read from the buffer until
        // there is data.
        uint8_t byte;
        bool done;

        do
        {
            ATOMIC_BLOCK(ATOMIC_FORCEON)
            {
                done = RxBuffer.read(byte);
            }
        }
        while (!done);

        return byte;
    }
    else
    {
        // If interrupts are disabled, first check if there is data in the
        // buffer. This may happen of interrupts were enabled in the past. If
        // there is no data in the buffer, wait until there is data in the UART
        // buffer.
        uint8_t byte;

        if (RxBuffer.read(byte))
            return byte;

        while (!(UCSR0A & (1 << RXC0))) {}

        return UDR0;
    }
}

}

namespace UartStdio
{

void Init()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Baudrate: 115200 @16MHz
        UBRR0 = 8;

        // The default frame settings (8-bit characters, 1 stop bit, no parity
        // bit) are fine for us

        // Enable transmitter, receiver, and receiver interrupt
        UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    }

    UartStdout.put = UartPutchar;
    UartStdout.get = UartGetchar;
    UartStdout.flags = _FDEV_SETUP_RW;
    stdout = stdin = &UartStdout;
}

}

ISR(USART_RX_vect)
{
    RxBuffer.write(UDR0);
}
