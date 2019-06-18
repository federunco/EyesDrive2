/*
 * File:   main.c
 * Author: federicorunco
 *
 * Created on June 16, 2019, 5:20 PM
 */

#include <avr/io.h>
#include <avr/delay.h>

// Configurazione comm seriale
#define BAUDRATE        9600
#define STOP_BITS       1

// Tempistica della cpu
#define F_CPU           8000000 // 8MHz xtal
#define CYCLES_PER_BIT  (F_CPU / BAUDRATE)
#if (CYCLES_PER_BIT > 255)
#define DIVISOR             8
#define PRESCALE            2
#else
#define DIVISOR             1
#define PRESCALE            1
#endif
#define FULL_BIT_TICKS      (CYCLES_PER_BIT / DIVISOR)

// Prototipi funzioni
static uint8_t reverse_byte (uint8_t x);
void usi_sendb(uint8_t data);
void usi_sends(char* buffer);

// Variabili di stato dell'USI
enum _usistate_t { AVAILABLE, FIRST, SECOND } usi_state;
static volatile uint8_t usi_data;

// Variabili frontend
uint8_t eog_raw;
uint8_t pacchetto[7] = { 'E', 'D', '2', 'P', 'D', 0, 0 };
uint8_t checksum = 0;

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
        // Acquisizione dall'ADC
        ADCSRA |= (1 << ADSC); // Abilita l'ADC
        while (ADCSRA & (1 << ADSC)); // Aspetta che il flag sia stato svuotato
        eog_raw = ADCH;
        
        // Costruzione pacchetto
        pacchetto[4] = eog_raw;
        while (*pacchetto) checksum += *pacchetto++;
        pacchetto[5] = checksum;
        
        // Invio pacchetto
        usi_sends(pacchetto);
        
        //Reset variabili
        checksum = 0;
        pacchetto[5] = 0;
        
        _delay_ms(50);
    }
}

void usi_sends(char* buffer){
    while(*buffer)
    {
        // aspetta fino a quando la porta è disponibile
        while (!(usi_state==AVAILABLE));
        usi_sendb(*buffer++);
        _delay_ms(10);
    }
}

static uint8_t reverse_byte (uint8_t x)
{
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}

void usi_sendb(uint8_t data)
{
    usi_state = FIRST;
    usi_data = reverse_byte(data);
    TCCR0A = 2<<WGM00;      // modalità CTC
    TCCR0B = PRESCALE;      // Imposta il prescaler del timer
    GTCCR |= 1 << PSR0;     // Resetta il prescaler
    TCNT0 = 0;              // Resetta il timer a 0
    OCR0A = FULL_BIT_TICKS; // Richiama l'interrupt ogni full_bit_ticks (compara)
    USICR = (0<<USIWM1)|(1<<USIWM0)|        // Comunicazione 3 wire su PB1
    (1<<USIOIE)|                        // Abilita il flag di overflow dell'USI
    (0<<USICS1)|(1<<USICS0)|(0<<USICLK);// Timer
    USIDR = 0x00 |          // Bit di start
    usi_data >> 1;    // 7 bit di dato (il dato rimane + stop mandati dopo)
    USISR = 1<<USIOIF |     // Resetta il flag di overflow dell'USI
    (16 - 8);         // Imposta il contatore dell'USI ad 8 (dim dato)
}

ISR (USI_OVF_vect)
{
    if (usi_state == FIRST)
    {
        usi_state = SECOND;
        USIDR = usi_data << 7 | 0x7F;       // Manda l'ultimo bit + un megabuffer di stop
        USISR = 1<<USIOIF |                 // Risvuota il flag dell'USI
        (16 - (1 + (STOP_BITS)));   // Imposta il contatore dell'USI ad 1 + i bit di stop
    }
    else
    { // secondo stadio
        PORTB |= 1 << PB1;  // Butta su PB1 livello alto
        USICR = 0;          // Disabilita l'usi
        USISR |= 1<<USIOIF; // Svuota di nuovo il flag
        usi_state = AVAILABLE;
    }
}
