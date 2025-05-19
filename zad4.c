// --- Konfiguracja mikrokontrolera ---
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
#include <stdio.h>
#include "lcd.h"

// --- Zmienne globalne ---
unsigned char minutes = 0;
unsigned char seconds = 0;
unsigned char isRunning = 0;

// --- Prototypy ---
void updateLCD();
void resetTime();

// --- Funkcja aktualizująca LCD ---
void updateLCD() {
    char buffer[17];
    LCD_ClearScreen();
    sprintf(buffer, "Czas: %02d:%02d", minutes, seconds);
    LCD_PutString(buffer, 16);
    LCD_SetCursor(1, 0);
    if (isRunning)
        LCD_PutString("Gotowanie...", 13);
    else
        LCD_PutString("Oczekiwanie", 11);
}

// --- Reset ---
void resetTime() {
    minutes = 0;
    seconds = 0;
    isRunning = 0;
    updateLCD();
}

// --- Główna funkcja ---
int main(void) {
    TRISAbits.TRISA7 = 1; // RA7 - start/stop
    TRISDbits.TRISD6 = 1; // RD6 - +1 minuta
    TRISDbits.TRISD7 = 1; // RD7 - +10 sek
    TRISDbits.TRISD13 = 1; // RD13 - reset

    LCD_Initialize();
    updateLCD();

    int prev6 = 1, prev7 = 1, prev8 = 1, prev9 = 1;
    int current6, current7, current8, current9;

    while (1) {
        // Zmienne pomocnicze do przycisków
        prev6 = PORTDbits.RD6;      //scanning for a change of buttons' state
        prev7 = PORTDbits.RD7;
        prev8 = PORTAbits.RA7;
        prev9 = PORTDbits.RD13;

        __delay32(150000);

        current6 = PORTDbits.RD6;
        current7 = PORTDbits.RD7;
        current8 = PORTAbits.RA7;
        current9 = PORTDbits.RD13;

        // Dodaj 1 minutę
        if (prev6 == 1 && current6 == 0 && !isRunning) {
            minutes++;
            if (minutes > 99) minutes = 99;
            updateLCD();
        }

        // Dodaj 10 sekund
        if (prev7 == 1 && current7 == 0 && !isRunning) {
            seconds += 10;
            if (seconds >= 60) {
                seconds -= 60;
                if (minutes < 99) minutes++;
            }
            updateLCD();
        }

        // Start/Stop
        if (prev8 == 1 && current8 == 0) {
            isRunning = !isRunning;
            updateLCD();
        }

        // Reset
        if (prev9 == 1 && current9 == 0) {
            resetTime();
        }

        // Odliczanie
        if (isRunning && (minutes > 0 || seconds > 0)) {
            __delay_ms(1000);
            if (seconds == 0) {
                if (minutes > 0) {
                    minutes--;
                    seconds = 59;
                }
            } else {
                seconds--;
            }
            updateLCD();
        }
    }
    return 0;
}
