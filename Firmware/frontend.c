/*
 * File:   main.c
 * Author: federicorunco
 *
 * Created on June 16, 2019, 5:20 PM
 */

#include <avr/io.h>
#include <avr/delay.h>

#define F_CPU 8000000 // 8MHz int xtal

uint8_t adc_lobyte;
uint16_t adc;

int main(void) 
{
    ADMUX =
            (1 << ADLAR) |      // risultato da sx a dx
            (0 << REFS1) |      // AREF a VCC b1
            (0 << REFS0) |      // AREF a VCC b1
            (0 << MUX3)  |      // ADC2 (PB4) B3
            (0 << MUX2)  |      // ADC2 (PB4) B2
            (1 << MUX1)  |      // ADC2 (PB4) B1
            (0 << MUX0)         // ADC2 (PB4) B0
    ;       
    ADCSRA = 
            (0 << ADEN)  |      // Abilitazione dell'ADC
            (1 << ADPS2) |      // Prescaler a 64 B2
            (1 << ADPS1) |      // Prescaler a 64 B1
            (0 << ADPS0)        // Prescaler a 64 B0
    ;      
    
    while (1) 
    {
        ADCSRA |= (1 << ADSC); // Abilita l'ADC
        while (ADCSRA & (1 << ADSC)); // Aspetta che il flag sia stato svuotato
        adc_lobyte = ADCL;            
        adc = ADCH<<8 | adc_lobyte;
    }
}

