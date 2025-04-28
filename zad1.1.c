// Konfiguracja bitów
#pragma config POSCMOD = XT
#pragma config OSCIOFNC = ON
#pragma config FCKSM = CSDCMD
#pragma config FNOSC = PRI
#pragma config IESO = ON

#pragma config WDTPS = PS32768
#pragma config FWPSA = PR128
#pragma config WINDIS = ON
#pragma config FWDTEN = OFF
#pragma config ICS = PGx2
#pragma config GWRP = OFF
#pragma config GCP = OFF
#pragma config JTAGEN = OFF

#include <xc.h>
#define FCY 4000000UL
#include <libpic30.h>
#include <stdint.h>

// --- Definicje programów ---
#define BINARY_UP    0
#define BINARY_DOWN  1
#define GRAY_UP      2
#define GRAY_DOWN    3
#define BCD_UP       4
#define BCD_DOWN     5
#define SNAKE        6
#define QUEUE        7
#define RANDOM       8

// --- Zmienne globalne ---
volatile uint8_t currentProgram = BINARY_UP;
volatile uint8_t zmienProgram = 0;
volatile uint8_t counter = 0;

// --- Funkcja pomocnicza ---
unsigned char binaryToGray(unsigned char binary) {
    return binary ^ (binary >> 1);
}

// --- Przerwanie zmiany stanu przycisków ---
void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    __delay_ms(20); // debounce

    if (!PORTDbits.RD6) { // RD6 - zmiana programu w przód
        currentProgram++;
        if (currentProgram > RANDOM) currentProgram = BINARY_UP;
        zmienProgram = 1;
    }
    if (!PORTDbits.RD13) { // RD13 - zmiana programu w tył
        if (currentProgram == BINARY_UP)
            currentProgram = RANDOM;
        else
            currentProgram--;
        zmienProgram = 1;
    }
    if (!PORTDbits.RD7) { // RD7 - reset aktualnego programu
        zmienProgram = 1;
    }

    IFS1bits.CNIF = 0; // wyczyść flagę przerwania
}

// --- Programy LED ---
void binaryUp() {
    counter = 0;
    while (!zmienProgram) {
        LATA = counter++;
        __delay_ms(250); // wcześniej ~1000000 cykli
    }
}

void binaryDown() {
    counter = 255;
    while (!zmienProgram) {
        LATA = counter--;
        __delay_ms(250);
    }
}

void grayUp() {
    counter = 0;
    while (!zmienProgram) {
        LATA = binaryToGray(counter++);
        __delay_ms(250);
    }
}

void grayDown() {
    counter = 255;
    while (!zmienProgram) {
        LATA = binaryToGray(counter--);
        __delay_ms(250);
    }
}

void bcdUp() {
    uint8_t portValue = 0;
    while (!zmienProgram) {
        uint8_t bcd = ((portValue / 10) << 4) | (portValue % 10);
        LATA = bcd;
        __delay_ms(250);
        portValue++;
        if (portValue > 99) portValue = 0;
    }
}

void bcdDown() {
    uint8_t portValue = 99;
    while (!zmienProgram) {
        uint8_t bcd = ((portValue / 10) << 4) | (portValue % 10);
        LATA = bcd;
        __delay_ms(250);
        if (portValue == 0) portValue = 99;
        else portValue--;
    }
}

void Snake() {
    uint8_t snake = 0b00000111;
    while (!zmienProgram) {
        while (snake != 0b11100000 && !zmienProgram) {
            LATA = snake;
            snake <<= 1;
            __delay_ms(125); // wcześniej ~500000
        }
        while (snake != 0b00000111 && !zmienProgram) {
            LATA = snake;
            snake >>= 1;
            __delay_ms(125);
        }
    }
}

void Queue() {
    unsigned char portValue = 0;
    while (!zmienProgram) {
        for (uint16_t i = 0; i < 8 && !zmienProgram; i++) {
            uint16_t temp = 1;
            for (uint16_t j = i + 1; j < 8 && !zmienProgram; j++) {
                LATA = portValue | temp;
                temp <<= 1;
                __delay_ms(250);
            }
            portValue |= temp;

            LATA = portValue;
            __delay_ms(375); // pełne zapalenie - większa pauza
        }
        portValue = 0;
    }
}

void Random() {
    uint8_t lcg = 0x2A; // Startowa wartość (np. 42)
    while (!zmienProgram) {
        lcg = (lcg * 17 + 43) % 64; // przykładowe dobre parametry
        LATA = lcg; // wysyłamy na LEDy (tylko 6 bitów)
        __delay_ms(250);
    }
}



// --- Funkcja main ---
int main(void) {
    AD1PCFG = 0xFFFF;
    TRISA = 0x0000;
    TRISD = 0xFFFF;

    // Konfiguracja przerwań
    CNEN1bits.CN15IE = 1;  // RD6
    CNEN2bits.CN19IE = 1;  // RD13
    CNEN2bits.CN16IE = 1;  // RD7
    IEC1bits.CNIE = 1;     // Włącz przerwania CN
    IFS1bits.CNIF = 0;     // Wyczyszczenie flagi
    IPC4bits.CNIP = 5;     // Priorytet przerwań CN

    while (1) {
        zmienProgram = 0;

        switch (currentProgram) {
            case BINARY_UP:   binaryUp();    break;
            case BINARY_DOWN: binaryDown();  break;
            case GRAY_UP:     grayUp();      break;
            case GRAY_DOWN:   grayDown();    break;
            case BCD_UP:      bcdUp();       break;
            case BCD_DOWN:    bcdDown();     break;
            case SNAKE:       Snake();       break;
            case QUEUE:       Queue();       break;
            case RANDOM:      Random();      break;
            default:          currentProgram = BINARY_UP; break;
        }
    }

    return 0;
}
