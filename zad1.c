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

int nrprogramu = 0;
unsigned char counter = 0;
int zmienProgram = 0;
unsigned char seed = 0xAA;

void sprawdzWyjscie() {
    if (PORTDbits.RD7 == 0 || PORTDbits.RD13 == 0 || PORTDbits.RD6 == 0)
        zmienProgram = 1;
}

unsigned char pseudoRandom() {
    seed = (seed >> 1) ^ (-(seed & 1) & 0xB8);
    return seed;
}

void binaryUp() {
    while (!zmienProgram) {
        LATA = counter;
        __delay32(1000000);
        counter++;
        sprawdzWyjscie();
    }
}

void binaryDown() {
    while (!zmienProgram) {
        LATA = counter;
        __delay32(1000000);
        counter--;
        sprawdzWyjscie();
    }
}

void grayUp() {
    unsigned char portValue = 0;
    unsigned char gray;
    while (!zmienProgram) {
        gray = (portValue >> 1) ^ portValue;
        LATA = gray;
        __delay32(1000000);
        portValue++;
        sprawdzWyjscie();
    }
}

void grayDown() {
    unsigned char portValue = 255;
    unsigned char gray;
    while (!zmienProgram) {
        gray = (portValue >> 1) ^ portValue;
        LATA = gray;
        __delay32(1000000);
        portValue--;
        sprawdzWyjscie();
    }
}

void bcdUp() {
    unsigned char portValue = 0;
    unsigned char bcd;
    while (!zmienProgram) {
        bcd = ((portValue / 10) << 4) | (portValue % 10);
        LATA = bcd;
        __delay32(1000000);
        portValue++;
        if (portValue > 99) portValue = 0;
        sprawdzWyjscie();
    }
}

void bcdDown() {
    unsigned char portValue = 99;
    unsigned char bcd;
    while (!zmienProgram) {
        bcd = ((portValue / 10) << 4) | (portValue % 10);
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
    for (int i = 0; i < 8 && !zmienProgram; i++) {
        int temp = 1;
        for (int j = i + 1; j < 8 && !zmienProgram; j++) {
            LATA = portValue + temp;
            temp <<= 1;
            __delay32(1000000);
            sprawdzWyjscie();
        }
        portValue += temp;
    }
}

void RandomBlink() {
    while (!zmienProgram) {
        LATA = pseudoRandom();
        __delay32(700000);
        sprawdzWyjscie();
    }
}

int main(void) {
    AD1PCFG = 0xFFFF;
    TRISA = 0x0000;
    TRISD = 0xFFFF;

    while (1) {
        zmienProgram = 0;

        switch (nrprogramu) {
            case 0: binaryUp(); break;
            case 1: binaryDown(); break;
            case 2: grayUp(); break;
            case 3: grayDown(); break;
            case 4: bcdUp(); break;
            case 5: bcdDown(); break;
            case 6: Snake(); break;
            case 7: Queue(); break;
            case 8: RandomBlink(); break;
            default: nrprogramu = 0; break;
        }

        if (PORTDbits.RD7 == 0) {
            counter = 0;
        }
        if (PORTDbits.RD13 == 0) {
            nrprogramu++;
            if (nrprogramu > 8) nrprogramu = 0;
            counter = 0;
        }
        if (PORTDbits.RD6 == 0) {
            if (nrprogramu == 0)
                nrprogramu = 8;
            else
                nrprogramu--;
            counter = 0;
        }
    }

    return 0;
}
