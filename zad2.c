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

// --- Definicje efektów ---
#define BINARY_UP  0
#define SNAKE      1
#define POTLEVEL   2

// --- Zmienne globalne ---
volatile uint8_t currentEffect = BINARY_UP;
volatile uint8_t zmienProgram = 0;

// --- Przerwanie przycisków (CN) ---
void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    __delay_ms(20); // debounce

    if (!PORTDbits.RD6) {
        currentEffect++;
        if (currentEffect > POTLEVEL)
            currentEffect = BINARY_UP;
        zmienProgram = 1;
    }
    if (!PORTDbits.RD13) {
        if (currentEffect == BINARY_UP)
            currentEffect = POTLEVEL;
        else
            currentEffect--;
        zmienProgram = 1;
    }
    if (!PORTDbits.RD7) {
        zmienProgram = 1;
    }

    IFS1bits.CNIF = 0;
}

// --- ADC: odczyt z AN5 (potencjometr) ---
uint16_t getPotValue() {
    AD1CHS = 5; // AN5
    AD1CON1bits.SAMP = 1;
    __delay_us(10);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}

uint16_t getDelayFromPotentiometer() {
    uint16_t adc = getPotValue();
    if (adc < 205)  return 100;
    if (adc < 410)  return 300;
    if (adc < 615)  return 600;
    if (adc < 820)  return 900;
    return 1200;
}

// --- Efekty ---
void effectBinaryUp() {
    uint8_t counter = 0;
    while (!zmienProgram) {
        LATA = counter++;
        __delay_ms(getDelayFromPotentiometer());
    }
}

void effectSnake() {
    uint8_t snake = 0b00000111;
    while (!zmienProgram) {
        while (snake != 0b11100000 && !zmienProgram) {
            LATA = snake;
            snake <<= 1;
            __delay_ms(getDelayFromPotentiometer());
        }
        while (snake != 0b00000111 && !zmienProgram) {
            LATA = snake;
            snake >>= 1;
            __delay_ms(getDelayFromPotentiometer());
        }
    }
}


// --- Setup sprzętowy ---
void setup() {
    TRISA = 0x0000;
    LATA = 0;
    TRISDbits.TRISD6 = 1;
    TRISDbits.TRISD7 = 1;
    TRISDbits.TRISD13 = 1;

    AD1PCFG = 0xFFDF; // AN5 jako analog (0 = analog)
    AD1CON1 = 0x00E0;
    AD1CON2 = 0;
    AD1CON3 = 0x1F02;
    AD1CON1bits.ADON = 1;

    CNEN1bits.CN15IE = 1;  // RD6
    CNEN2bits.CN19IE = 1;  // RD13
    CNEN2bits.CN16IE = 1;  // RD7
    IEC1bits.CNIE = 1;
    IFS1bits.CNIF = 0;
    IPC4bits.CNIP = 5;
}

// --- Główna pętla ---
int main(void) {
    setup();

    while (1) {
        zmienProgram = 0;

        switch (currentEffect) {
            case BINARY_UP:  effectBinaryUp();  break;
            case SNAKE:      effectSnake();     break;
            default:         currentEffect = BINARY_UP; break;
        }
    }
    return 0;
}
