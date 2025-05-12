// --- Konfiguracja bitów konfiguracyjnych mikrokontrolera PIC24FJ128GA010 ---

#pragma config POSCMOD = XT          // Tryb oscylatora: XT (kwarc standardowy)
#pragma config OSCIOFNC = ON         // OSC2/CLKO jako I/O, a nie wyjście zegara
#pragma config FCKSM = CSDCMD        // Wyłączenie możliwości zmiany zegara i monitora
#pragma config FNOSC = PRI           // Główny oscylator jako źródło zegara
#pragma config IESO = ON             // Włączone przełączanie wewnętrznego i zewnętrznego oscylatora

#pragma config WDTPS = PS32768       // Preskaler Watchdog Timer’a (1:32768)
#pragma config FWPSA = PR128         // Preskaler samego WDT (1:128)
#pragma config WINDIS = ON           // Watchdog Timer nie pracuje w trybie okienkowym
#pragma config FWDTEN = OFF          // Watchdog Timer wyłączony
#pragma config ICS = PGx2            // Debugger używa kanału EMUC2/EMUD2
#pragma config GWRP = OFF            // Możliwość zapisu do pamięci programu
#pragma config GCP = OFF             // Ochrona kodu wyłączona
#pragma config JTAGEN = OFF          // Port JTAG wyłączony

#include <xc.h>
#define FCY 4000000UL                // Częstotliwość systemowa dla delay
#include <libpic30.h>
#include <stdint.h>

// --- Definicje trybów działania efektów LED ---
#define BINARY_UP  0                 // Licznik binarny w górę
#define SNAKE      1                 // Efekt Snake
#define POTLEVEL   2                 // Tryb bazujący na potencjometrze (nieużywany)

// --- Zmienne globalne ---
volatile uint8_t currentEffect = BINARY_UP; // Aktywny efekt
volatile uint8_t zmienProgram = 0;          // Flaga zmiany programu

// --- Przerwanie dla przycisków (Change Notification CN) ---
void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    __delay_ms(20); // Debounce – eliminacja drgań styków

    if (!PORTDbits.RD6) {           // Przycisk RD6 – następny efekt
        currentEffect++;
        if (currentEffect > POTLEVEL)
            currentEffect = BINARY_UP;
        zmienProgram = 1;
    }

    if (!PORTDbits.RD13) {          // Przycisk RD13 – poprzedni efekt
        if (currentEffect == BINARY_UP)
            currentEffect = POTLEVEL;
        else
            currentEffect--;
        zmienProgram = 1;
    }

    if (!PORTDbits.RD7) {           // Przycisk RD7 – reset aktualnego efektu
        zmienProgram = 1;
    }

    IFS1bits.CNIF = 0;              // Czyszczenie flagi przerwania
}

// --- Odczyt wartości z potencjometru (AN5) ---
uint16_t getPotValue() {
    AD1CHS = 5;                     // Wybór kanału analogowego AN5
    AD1CON1bits.SAMP = 1;          // Rozpoczęcie próbkowania
    __delay_us(10);
    AD1CON1bits.SAMP = 0;          // Zatrzymanie próbkowania
    while (!AD1CON1bits.DONE);     // Czekaj na zakończenie konwersji
    return ADC1BUF0;               // Zwróć wartość
}

// --- Konwersja wartości ADC na opóźnienie w milisekundach ---
uint16_t getDelayFromPotentiometer() {
    uint16_t adc = getPotValue();
    if (adc < 205)  return 50;    // Najszybsze tempo
    if (adc < 410)  return 200;
    if (adc < 615)  return 400;
    if (adc < 820)  return 600;
    return 800;                   // Najwolniejsze tempo
}

// --- Efekt: Licznik binarny w górę ---
void effectBinaryUp() {
    uint8_t counter = 0;
    while (!zmienProgram) {
        LATA = counter++;                          // Wyświetlenie liczby binarnej na diodach
        __delay_ms(getDelayFromPotentiometer());   // Opóźnienie zależne od potencjometru
    }
}

// --- Efekt: Snake (wąż przesuwający się po LED) ---
void effectSnake() {
    uint8_t snake = 0b00000111;    // Startowa maska (3 zapalone diody)
    while (!zmienProgram) {
        while (snake != 0b11100000 && !zmienProgram) {
            LATA = snake;                          // Wyświetl efekt
            snake <<= 1;                           // Przesunięcie w lewo
            __delay_ms(getDelayFromPotentiometer());
        }
        while (snake != 0b00000111 && !zmienProgram) {
            LATA = snake;
            snake >>= 1;                           // Przesunięcie w prawo
            __delay_ms(getDelayFromPotentiometer());
        }
    }
}

// --- Inicjalizacja sprzętu ---
void setup() {
    TRISA = 0x0000;              // Port A jako wyjście (LED)
    LATA = 0;

    TRISDbits.TRISD6  = 1;       // Przycisk RD6 jako wejście
    TRISDbits.TRISD7  = 1;       // Przycisk RD7 jako wejście
    TRISDbits.TRISD13 = 1;       // Przycisk RD13 jako wejście

    AD1PCFG = 0xFFDF;            // AN5 jako wejście analogowe
    AD1CON1 = 0x00E0;            // Konfiguracja ADC
    AD1CON2 = 0;
    AD1CON3 = 0x1F02;
    AD1CON1bits.ADON = 1;        // Włączenie ADC

    // Konfiguracja przerwań przycisków (CN)
    CNEN1bits.CN15IE = 1;        // RD6
    CNEN2bits.CN19IE = 1;        // RD13
    CNEN2bits.CN16IE = 1;        // RD7
    IEC1bits.CNIE = 1;           // Włącz globalne przerwania CN
    IFS1bits.CNIF = 0;           // Wyczyść flagę
    IPC4bits.CNIP = 5;           // Priorytet przerwania
}

// --- Funkcja główna ---
int main(void) {
    setup(); // Inicjalizacja

    while (1) {
        zmienProgram = 0; // Reset flagi

        switch (currentEffect) {
            case BINARY_UP:
                effectBinaryUp();
                break;
            case SNAKE:
                effectSnake();
                break;
            default:
                currentEffect = BINARY_UP;
                break;
        }
    }

    return 0;
}

