﻿#include <avr/io.h>
#include "uart.h"

uint16_t index_buf = 0;		//указатель на последний символ в uart буфере
char buffer_uart[ELEMENTS];	//uart буфер для приема строк
uint8_t recieve_count = 0;	//показатель завершения приема строки(байта)

//////////////////////////////////////////////////////
void uart_init(unsigned long baud){ //Инициализация UARTа
	#ifdef MKREG
    UBRR0H=0x00;
    UBRR0L=F_CPU/16/baud - 1;

    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    UCSR0C =(1<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01);
	#else
	UBRRH=0x00;
    UBRRL=F_CPU/16/baud - 1;

    UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
    UCSRC =(1<<USBS)|(1<<UCSZ0)|(1<<UCSZ1);
	#endif
}
///////////////////////////////////////////////////////////////////////
void uart_tr(uint8_t data){	//Передача одного байта	
	#ifdef MKREG
	while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data;
	#else
    while ( !( UCSRA & (1<<UDRE)) );
    UDR = data;
	#endif
}
/////////////////////////////////////////////////////
void uart_send(char *string){ //Передача строки
    while(*string != '\0'){
        uart_tr(*string);
        string++;}
}
/////////////////////////////////////////////////////////////////////////
void uart_sendln(char *str){ //Передача строки с переходом на новую
    uart_send(str);
    uart_tr('\r');
    uart_tr('\n');
}

////////////////////////////////////////////////////////////////////////
uint8_t uart_receive_byte(void){	//Прием одного байта
	#ifdef MKREG
	while ( !(UCSR0A & (1<<RXC0)) );
    return UDR0;
	#else
    while ( !(UCSRA & (1<<RXC)) );
    return UDR;
	#endif
}
/////////////////////////////////////////////////////////////////////////
#if ISR_COUNT
void uart_receive(void){//Прием строки до символа '\r' 
	#ifdef MKREG
	char symbol = UDR0; 
	#else
	char symbol = UDR; 
	#endif
    uart_tr(symbol);
    if(symbol == '\r')//при возврате каретки завершается заполнение buffer_uart
    { uart_tr('\n'); recieve_count = 1; buffer_uart[index_buf] = '\0'; }
	else {	if(symbol=='\b'){ //стирание символа
	uart_tr(' ');
	uart_tr('\b');
        if(index_buf>0) index_buf--; 
            for(uint16_t i = index_buf; i<ELEMENTS; i++) buffer_uart[i] = '\0'; }
	else buffer_uart[index_buf++] = symbol;
        if(index_buf>=ELEMENTS)index_buf = 0; }
		return;
}
#endif