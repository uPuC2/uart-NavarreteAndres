#include <avr/io.h>
#define NULL ((void*)0)


// Estructura y mapeo de registros UART
typedef struct {
    volatile uint8_t *ubrrh;
    volatile uint8_t *ubrrl;
    volatile uint8_t *ucsra;
    volatile uint8_t *ucsrb;
    volatile uint8_t *ucsrc;
    volatile uint8_t *udr;
    uint8_t rxen_bit;
    uint8_t txen_bit;
    uint8_t rxcie_bit;
    uint8_t udre_bit;
    uint8_t rxc_bit;
    uint8_t ucsza0_bit;
    uint8_t ucsza1_bit;
    uint8_t upm0_bit;
    uint8_t upm1_bit;
    uint8_t usbs_bit;
} UART_Registers;

const UART_Registers UART[] = {
    { &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0, RXEN0, TXEN0, RXCIE0, UDRE0, RXC0, UCSZ00, UCSZ01, UPM00, UPM01, USBS0 },
    { &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1, RXEN1, TXEN1, RXCIE1, UDRE1, RXC1, UCSZ10, UCSZ11, UPM10, UPM11, USBS1 },
    { &UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2, RXEN2, TXEN2, RXCIE2, UDRE2, RXC2, UCSZ20, UCSZ21, UPM20, UPM21, USBS2 },
    { &UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3, RXEN3, TXEN3, RXCIE3, UDRE3, RXC3, UCSZ30, UCSZ31, UPM30, UPM31, USBS3 }
};

void UART_Ini(uint8_t com, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop) {
    if (com > 3) return;

    const UART_Registers *uart = &UART[com];
    uint16_t ubrr = (16000000UL / (16UL * baudrate)) - 1;

    *(uart->ubrrh) = (ubrr >> 8);
    *(uart->ubrrl) = ubrr;

    *(uart->ucsrb) = (1 << uart->rxen_bit) | (1 << uart->txen_bit);
    *(uart->ucsrc) = 0;

    if (size == 8) {
        *(uart->ucsrc) |= (1 << uart->ucsza0_bit) | (1 << uart->ucsza1_bit);
    }

    if (parity == 1) {
        *(uart->ucsrc) |= (1 << uart->upm0_bit) | (1 << uart->upm1_bit); // impar
    } else if (parity == 2) {
        *(uart->ucsrc) |= (1 << uart->upm1_bit); // par
    }

    if (stop == 2) {
        *(uart->ucsrc) |= (1 << uart->usbs_bit);
    }
}

void UART_putchar(uint8_t com, char c) {
    const UART_Registers *uart = &UART[com];
    while (!(*(uart->ucsra) & (1 << uart->udre_bit)));
    *(uart->udr) = c;
}

char UART_getchar(uint8_t com) {
    const UART_Registers *uart = &UART[com];
    while (!(*(uart->ucsra) & (1 << uart->rxc_bit)));
    return *(uart->udr);
}

void UART_puts(uint8_t com, char *str) {
    while (*str) {
        UART_putchar(com, *str++);
    }
}

uint8_t UART_available(uint8_t com) {
    const UART_Registers *uart = &UART[com];
    return (*(uart->ucsra) & (1 << uart->rxc_bit)) != 0;
}

void UART_gets(uint8_t com, char *str)
{
    if (com > 3 || str == NULL) return;
    
    char c;
    uint16_t i = 0;

    while (1) {
       // while (!UART_available(com));  // Espera hasta que haya un carácter disponible
        
        c = UART_getchar(com);  // Recibir carácter
		
		
		

        if (c == '\n' || c == '\r') {  
            str[i] = '\0';  // Terminar cadena
            break;
        }
        else if (c == '\b' || c == 127) {  // Retroceso 
            if (i > 0) {
                i--;
                UART_puts(com, "\b \b");  // Borra el carácter en la pantalla
            }
        }
        else if (i < 19) {  // Evitar desbordamiento
            str[i++] = c;
            UART_putchar(com, c); 
        }
    }
}

void UART_clrscr(uint8_t com) {
    UART_puts(com, "\x1B[2J\x1B[H");
}

void itoa(uint16_t number, char* str, uint8_t base)
{
	uint8_t i = 0;
    uint8_t temp;
	if(base==10)
	{
		if (number == 0) {
            str[i++] = '0';  
        } else {
            while (number > 0) {
                temp = number % 10;
                str[i++] = temp + '0'; 
                number /= 10;
            }
        }
	}
	if(base == 16)
	{
		if (number == 0) {
            str[i++] = '0';  
        } else {
            while (number > 0) {
                temp = number % 16;
                str[i++] = (temp < 10) ? (temp + '0') : (temp - 10 + 'A');  
                number /= 16;
            }
        }
	}
	if(base == 2)
	{
		if (number == 0) {
            str[i++] = '0';  
        } else {
            while (number > 0) {
                str[i++] = (number % 2) + '0';  
                number /= 2;
            }
        }
	}
	if(base == 8)
	{
		 if (number == 0) {
            str[i++] = '0';  
        } else {
            while (number > 0) {
                temp = number % 8;
                str[i++] = temp + '0';  
                number /= 8;
            }
        }
	}

	 str[i] = '\0';
    
    
    uint8_t start = 0;
    uint8_t end = i - 1;
    while (start < end) {
        
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}
uint16_t atoi(char *str)
{
	 uint16_t result = 0;
    uint8_t i = 0;
    while (str[i] != '\0') { 
        if (str[i] >= '0' && str[i] <= '9') { 
            result = result * 10 + (str[i] - '0');
        } else { 
            break;  
        }
        i++;
    }
    
    return result;
}

void UART_setColor(uint8_t com, uint8_t color) {
    char seq[10];
    UART_puts(com, "\x1B[");
    itoa(color, seq, 10);
    UART_puts(com, seq);
    UART_puts(com, "m");
}

void UART_gotoxy(uint8_t com, uint8_t x, uint8_t y) {
    char num[10];
    UART_puts(com, "\x1B[");

    itoa(y, num, 10);
    UART_puts(com, num);
    UART_puts(com, ";");

    itoa(x, num, 10);
    UART_puts(com, num);
    UART_puts(com, "H");
}
