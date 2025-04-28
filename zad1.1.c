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
#include <libpic30.h>
#include <stdint.h>

// --- Zmienne globalne ---
uint16_t nrprogramu = 0;
uint16_t zmienProgram = 0;
unsigned char counter = 0;

// --- Funkcja pomocnicza: sprawdzanie przycisków ---
void sprawdzWyjscie() {
    if (PORTDbits.RD7 == 0) {
        counter = 0;      // Reset licznika
        zmienProgram = 1; // Ustaw wyjście z funkcji
    }
    if (PORTDbits.RD13 == 0) {
        nrprogramu++;
        if (nrprogramu > 8) nrprogramu = 0;
        counter = 0;
        zmienProgram = 1;
    }
    if (PORTDbits.RD6 == 0) {
        if (nrprogramu == 0)
            nrprogramu = 8;
        else
            nrprogramu--;
        counter = 0;
        zmienProgram = 1;
    }
}

// --- Programy działania LED ---

void binaryUp() {
    while (!zmienProgram) {
        LATA = counter++;
        __delay32(1000000);
        sprawdzWyjscie();
    }
}

void binaryDown() {
    while (!zmienProgram) {
        LATA = counter--;
        __delay32(1000000);
        sprawdzWyjscie();
    }
}

void grayUp() {
    unsigned char portValue = 0;
    while (!zmienProgram) {
        unsigned char gray = (portValue >> 1) ^ portValue;
        LATA = gray;
        __delay32(1000000);
        portValue++;
        sprawdzWyjscie();
    }
}

void grayDown() {
    unsigned char portValue = 255;
    while (!zmienProgram) {
        unsigned char gray = (portValue >> 1) ^ portValue;
        LATA = gray;
        __delay32(1000000);
        portValue--;
        sprawdzWyjscie();
    }
}

void bcdUp() {
    unsigned char portValue = 0;
    while (!zmienProgram) {
        unsigned char bcd = ((portValue / 10) << 4) | (portValue % 10);
        LATA = bcd;
        __delay32(1000000);
        portValue++;
        if (portValue > 99) portValue = 0;
        sprawdzWyjscie();
    }
}

void bcdDown() {
    unsigned char portValue = 99;
    while (!zmienProgram) {
        unsigned char bcd = ((portValue / 10) << 4) | (portValue % 10);
        LATA = bcd;
        __delay32(1000000);
        if (portValue == 0) portValue = 99;
        else portValue--;
        sprawdzWyjscie();
    }
}

void Snake() {
    unsigned char snake = 0b00000111;
    while (!zmienProgram) {
        while (snake != 0b11100000 && !zmienProgram) {
            LATA = snake;
            snake <<= 1;
            __delay32(500000);
            sprawdzWyjscie();
        }
        while (snake != 0b00000111 && !zmienProgram) {
            LATA = snake;
            snake >>= 1;
            __delay32(500000);
            sprawdzWyjscie();
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
                __delay32(1000000); // standardowe opóźnienie
                sprawdzWyjscie();
            }
            portValue |= temp;

            // Po pełnym zapaleniu - pauza
            LATA = portValue;
            __delay32(1500000);
            sprawdzWyjscie();
        }

        portValue = 0; // reset kolejki
    }
}

void Random() {
    unsigned char x = 0x2A;
    unsigned char y = 0x15;
    unsigned char z;

    while (!zmienProgram) {
        x ^= (x << 3);
        x ^= (x >> 5);
        y += 13;
        z = (x ^ y) & 0x3F; // tylko 6 bitów
        LATA = z;
        __delay32(900000);
        sprawdzWyjscie();
    }
}

// --- Funkcja main ---
int main(void) {
    AD1PCFG = 0xFFFF; // ustawienie wszystkich pinów na cyfrowe
    TRISA = 0x0000;   // port A jako wyjście (diody)
    TRISD = 0xFFFF;   // port D jako wejście (przyciski)

    while (1) {
        zmienProgram = 0;

        switch (nrprogramu) {
            case 0: binaryUp();    break;
            case 1: binaryDown();  break;
            case 2: grayUp();      break;
            case 3: grayDown();    break;
            case 4: bcdUp();       break;
            case 5: bcdDown();     break;
            case 6: Snake();       break;
            case 7: Queue();       break;
            case 8: Random();      break;
            default: nrprogramu = 0; break;
        }

        // 🛑 WAŻNE: Nie sprawdzamy tu przycisków! wszystko w sprawdzWyjscie()
    }

    return 0;
}
