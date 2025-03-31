// PIC24FJ128GA010 Configuration Bit Settings
// For more on Configuration Bits, consult your device data sheet
// CONFIG2
#pragma config POSCMOD = XT // XT Oscillator mode selected
#pragma config OSCIOFNC = ON // OSC2/CLKO/RC15 as port I/O (RC15)
#pragma config FCKSM = CSDCMD // Clock Switching and Monitor disabled
#pragma config FNOSC = PRI // Primary Oscillator (XT, HS, EC)
#pragma config IESO = ON // Int Ext Switch Over Mode enabled
// CONFIG1
#pragma config WDTPS = PS32768 // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128 // WDT Prescaler (1:128)
#pragma config WINDIS = ON // Watchdog Timer Window Mode disabled
#pragma config FWDTEN = OFF // Watchdog Timer disabled
#pragma config ICS = PGx2 // Emulator/debugger uses EMUC2/EMUD2
#pragma config GWRP = OFF // Writes to program memory allowed
#pragma config GCP = OFF // Code protection is disabled
#pragma config JTAGEN = OFF // JTAG port is disabled
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <libpic30.h> 

int nrprogramu=0; //numer programu

void binaryUp(){
    unsigned char counter = 0; //lecimy z counterem
    AD1PCFG = 0xFFFF; //formalnosci
    TRISA = 0x0000; 
    while(1) {
        LATA = counter; //licznik na gwiazdkach
        __delay32(1000000); //mam w glowie delay de delay
        counter++; 
        if (counter == 0) //resecik
            counter = 0; 
       
        if(PORTDbits.RD7 == 0) //przyciski, rd6, rd7 ,rd13, jednego nie rozkminilem
        {
            counter=0;//resecik
        }
         if(PORTDbits.RD13 == 0) // ostatni do przodu
        {
            nrprogramu += 1; 
            break; 
        }
        if(PORTDbits.RD6 == 0) 
        {
            nrprogramu -= 1; 
            break; 
        }
    }
}

void binaryDown(){
   unsigned char counter = 0; // tak samo
    AD1PCFG = 0xFFFF; //meh
    TRISA = 0x0000; 
    while(1) {
        LATA = counter; 
        __delay32(1000000); 
        counter--; 
        if (counter == 0) 
            counter = 0; 
        
        if(PORTDbits.RD7 == 0) //przyciski, rd6, rd7 ,rd13, jednego nie rozkminilem
        {
            counter=0;//resecik
        }
         if(PORTDbits.RD13 == 0) // ostatni do przodu
        {
            nrprogramu += 1; 
            break; 
        }
        if(PORTDbits.RD6 == 0) 
        {
            nrprogramu -= 1; 
            break; 
        }

    }
}



int main(void) {
    unsigned char portValue;
    int program = 0;
    AD1PCFG = 0xFFFF; // set to digital I/O (not analog)
    TRISA = 0x0000; // set all port bits to be output
    
    if(nrprogramu < 0)
    {
        nrprogramu = 0;
    }
    
    if(nrprogramu == 0)
    {
        binaryUp();
    }
    
    if(nrprogramu == 1)
    {
        binaryDown();
    }
    if(nrprogramu>1)
    {
        nrprogramu=0;
    }
return 0;
}
