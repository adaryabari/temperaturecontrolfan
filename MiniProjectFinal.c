// CONFIG1
#pragma config FEXTOSC = OFF       // FEXTOSC External Oscillator mode Selection bits->Oscillator not enabled
#pragma config RSTOSC = HFINT1     // Power-up default value for COSC bits->HFINTOSC
#pragma config CLKOUTEN = OFF      // Clock Out Enable bit->CLKOUT function is disabled; I/O or oscillator function on OSC2
#pragma config CSWEN = ON          // Clock Switch Enable bit->Writing to NOSC and NDIV is allowed
#pragma config FCMEN = ON          // Fail-Safe Clock Monitor Enable->Fail-Safe Clock Monitor is enabled

// CONFIG2
#pragma config MCLRE = ON           // Master Clear Enable bit->MCLR/VPP pin function is MCLR; Weak pull-up enabled
#pragma config PWRTE = OFF          // Power-up Timer Enable bit->PWRT disabled
#pragma config WDTE = OFF           // Watchdog Timer Enable bits->WDT disabled; SWDTEN is ignored
#pragma config LPBOREN = OFF        // Low-power BOR enable bit->ULPBOR disabled
#pragma config BOREN = ON           // Brown-out Reset Enable bits->Brown-out Reset enabled, SBOREN bit ignored
#pragma config BORV = LOW           // Brown-out Reset Voltage selection bit->Brown-out voltage (Vbor) set to 2.45V
#pragma config PPS1WAY = ON         // PPSLOCK bit One-Way Set Enable bit->The PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle
#pragma config STVREN = ON          // Stack Overflow/Underflow Reset Enable bit->Stack Overflow or Underflow will cause a Reset
#pragma config DEBUG = OFF          // Debugger enable bit->Background debugger disabled

// CONFIG3
#pragma config WRT = OFF            // User NVM self-write protection bits->Write protection off
#pragma config LVP = ON             // Low Voltage Programming Enable bit->Low Voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored.

// CONFIG4
#pragma config CP = OFF             // User NVM Program Memory Code Protection bit->User NVM code protection disabled
#pragma config CPD = OFF            // Data NVM Memory Code Protection bit->Data NVM code protection disabled

//****************************************************************************
// Includes
//****************************************************************************
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

//****************************************************************************
// Defines
//****************************************************************************
#define _XTAL_FREQ  4000000

//****************************************************************************
// Global Variables
//****************************************************************************
uint32_t count = 0;

//****************************************************************************
// Main
//****************************************************************************
void main(void)
{
    OSCCON1 = 0x60;           // HFINTOSC
    OSCFRQ = 0x03;            // HFFRQ 4_MHz;

    // TIMER0 Config
    T0CON0 = 0b10000000;      // Enables TIMER0 for internal clock
    T0CON1 = 0b01000110;      /* 010 - Fosc/4, 0 - input to TMR0 synchronized
                                 w/ Fosc/4, 0110 - 1:64 prescalar rate select bit */
    TMR0H = 155;              /* TIMER0 period register,
                                 Set TMR0H value for interrupt flag */

    // Analog config
    TRISA4 = 1;               // RA4 pin configured as input
    ANSA4 = 1;                // RA4 pin assigned as analog input
    ADCON0 = 0b00010001;      /* 000100 - ADC channel ANA4 enabled,
                                 0 - GODONE=0 (ADC conversion status), 1 - ADON=1 (ADC is enabled) */
    ADCON1 = 0b01010000;      /* 0 - Left justified, 101 - Fosc/16
                                 0 - Unimplemented, 0 - VREF- connected to AVss,
                                 00 - VREF+ connected to VDD */
    ADGO = 1;                 // ADC conversion begins

    // PWM config
    TRISC = 0b11011111;       // Set RC5 pin as output (1 = input, 0 = output)
    RC5PPS = 0b00010;         // Sets Rxy source to PWM5 for RC5 pin
    PPSLOCK = 1;              // Locks pins. PPS selections can't be changed
    PWM5CON = 0;              // Clears PWM5CON register
    PWM5EN = 1;               // Enables PWM5
    PR2 = 255;                // Sets PWM wave frequency to max (1.22kHz)
    TMR2ON = 1;               // Enables TIMER2 for PWM clock
    PWM5DCL = 0b11000000;

    // Enable interrupts
    TMR0IE = 1;               // Enables the TMR0 interrupt
    PEIE = 1;                 // Enables all active peripheral interrupts
    GIE = 1;                  // Enables all active interrupts


    while (count < 300000)    // 3 seconds burst mode
    {
        PWM5DCH = 255;
        count++;
    }

    count = 0;

// ADRESH register gets ADC conversion value
// PW5DCH register sets the duty cycle
    while (1)
    {
       if(ADRESH < 37) PWM5DCH = 79;                  // 76 @ 31% (Idle)
       else if (ADRESH < 46) PWM5DCH = 89;            // 80 @ 35%
       else if (ADRESH < 56) PWM5DCH = 100;           // 83 @ 39%
       else if (ADRESH < 66) PWM5DCH = 110;           // 85 @ 43%
       else if (ADRESH < 76) PWM5DCH = 120;           // 88 @ 47%
       else if (ADRESH < 86) PWM5DCH = 131;           // 90 @ 51%
       else if (ADRESH < 96) PWM5DCH = 141;           // 93 @ 55%
       else if (ADRESH < 106) PWM5DCH = 151;          // 95 @ 59%
       else if (ADRESH < 115) PWM5DCH = 162;          // 98 @ 63%
       else if (ADRESH < 125) PWM5DCH = 172;          // 100 @ 68%
       else if (ADRESH < 135) PWM5DCH = 182;          // 103 @ 72%
       else if (ADRESH < 145) PWM5DCH = 193;          // 105 @ 76%
       else if (ADRESH < 155) PWM5DCH = 203;          // 108 @ 80%
       else if (ADRESH < 165) PWM5DCH = 214;          // 110 @ 84%
       else if (ADRESH < 175) PWM5DCH = 224;          // 113 @ 88%
       else if (ADRESH < 184) PWM5DCH = 234;          // 115 @ 92%
       else if (ADRESH < 194) PWM5DCH = 245;          // 118 @ 96%
       else if (ADRESH < 204) PWM5DCH = 255;          // 120 @ 100%
       else PWM5DCH = 255;                            // Burst mode (100%)
    }
}

// TMR0IF is the TMR0 overflow interrupt flag bit (PIR0 register)
void interrupt my_isr(void)
{
 if (TMR0IF && TMR0IE)        // Check for overflow when TMR0 interrupt is enabled
 {
    TMR0IF = 0;               // Clear TIMER0 flag if there was overflow
    ADGO = 1;                 // Begin ADC conversion again
 }
}
