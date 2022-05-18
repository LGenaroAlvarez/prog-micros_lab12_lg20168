/*
 * File:   lab12_main-20168.c
 * Author: Luis Genaro Alvarez Sulecio 20168
 * 
 * Programa: lector de potenciometro con ADC a contador en PORTC y pushbuttons
 * en RB1 y RB0 para activar y desactivar el modo sleep del pic.
 * 
 * Created on May 18, 2022, 4:36 AM
 */
// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF           // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF          // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF          // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF             // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF            // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF          // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF           // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF          // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF            // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V       // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF            // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

//DEFINICION DE FRECUENCIA PARA DELAY
#define _XTAL_FREQ 1000000          // FRECUENCIA PARA DELAYS (1MHz) 

//VARIABLES GLOBALES
uint8_t pot_in = 0;                 // VARIABLE PARA ALMACENAR VALOR DEL POT
int pic_sleep = 0;                  // BANDERA PARA SABER SI SE ENCUENTRA EN MODO SLEEP

//PROTO FUNCIONES
void setup(void);                   // FUNCION DE SETUP

//INTERRUPCIONES
void __interrupt() isr(void){
   
    if (PIR1bits.ADIF){             // REVISAR SI HAY INTERRUPCION DE ADC
        if (ADCON0bits.CHS == 0){   // REVISAR SI HAY INTERRUPCION EN EL CANAL 1
           pot_in = ADRESH;         // CARGAR VALOR DEL ADRESH A VARIABLE DE POTENCIOMETRO
           PORTC = pot_in;
        }
        PIR1bits.ADIF = 0;          // LIMPIAR BANDERA DE INTERRUPCION DE ADC
    }
    
    if (INTCONbits.RBIF){           // REVISAR SI HAY INTERRUPCION PORTB
        if (PORTBbits.RB0 == 0){    // INTERRUPCION EN RB0
            pic_sleep = 0;          // SI HUBO INTERRUPCION, APAGAR BANDERA DE SLEEP
            PORTEbits.RE0 = 0;      // LIMPIAR PIN DE LED INDICADOR DE SLEEP
        }
        else if (PORTBbits.RB1 == 0){   // INTERRUPCION EN RB1
            pic_sleep = 1;          // ACTIVAR BANDERA DE SLEEP
            PORTEbits.RE0 = 1;      // ENCENDER PIN DE LED INDICADOR DE SLEEP
            SLEEP();                // COLOCAR AL PIC EN MODO SLEEP
        }                   
        INTCONbits.RBIF = 0;        // LIMPIAR BANDERA DE INTERRUPCION EN PORTB
    }
    return;
}

void main(void) {
    //EJECUCION CONFIG
    setup();

    while(1){
        if (pic_sleep == 0){            // REVISAR SI EL PIC SE HA DORMIDO 
            if (ADCON0bits.GO == 0){    // REVISAR SI SE HA INICIADO LA CONVERSION DEL ADC
                ADCON0bits.GO = 1;      // INICIAR PROCESO DE CONVERSION
                __delay_us(40);         // TIEMPO DE PROCESAMIENTO
            }
        }
    }
}

//CONFIGURACION PRINCIPAL
void setup(void){
    ANSEL = 0b00000001;             // AN1 COMO ENTRADA ANALOGICA
    ANSELH = 0;                     // I/O DIGITALES

    TRISA = 0b00000001;             // RA1 COMO ENTRADA
    PORTA = 0;                      // LIMPIEZA DE PORTA

    TRISC = 0;                      // PORTC COMO SALIDA
    PORTC = 0;                      // LIMPIEZA DE PORTC
    
    TRISD = 0;                      // PORTD COMO SALIDA
    PORTD = 0;                      // LIMPIEZA DE PORTD
    
    TRISE = 0;                      // PORTE COMO SALIDA
    PORTE = 0;                      // LIMPIEZA DE PORTE

    //OSCCONFIC
    OSCCONbits.IRCF = 0b0100;       // FRECUENCIA DE OSCILADOR INTERNO (1MHz)
    OSCCONbits.SCS  = 1;            // RELOJ INTERNO
    
    //CONFIG DE INTERRUPCIONES
    INTCONbits.GIE = 1;             // HABILITAR INTERRUPCIONES GLOBALES
    INTCONbits.PEIE = 1;            // HABILITAR INTERRUPCIONES EN PERIFERICOS
    INTCONbits.RBIE = 1;            // HABILITAR INTERRUPCIONES EN PORTB
    IOCBbits.IOCB0 = 1;             // HABILITAR INTERRUPCION EN CAMBIO PARA RB0
    IOCBbits.IOCB1 = 1;             // HABILITAR INTERRUPCION EN CAMBIO PARA RB1
    INTCONbits.RBIF = 0;            // LIMPIAR BANDERA DE INTERRUPCION EN PORTB
    PIR1bits.ADIF = 0;              // LIMPIEZA DE BANDERA DE INTERRUPCION DE ADC
    PIE1bits.ADIE = 1;              // HABILITAR INTERRUPCION DE ADC

    //CONFIG PUSHBUTTONS EN PORTB
    TRISBbits.TRISB0 = 1;           // RB0 COMO INPUT
    TRISBbits.TRISB1 = 1;           // RB1 COMO INPUT
    OPTION_REGbits.nRBPU = 0;       // HABILITAR WEAK PULLUP EN PUERTO B
    WPUBbits.WPUB0 = 1;             // HABILITAR RESISTENCIA EN RB0
    WPUBbits.WPUB1 = 1;             // HABILITAR RESISTENCIA EN RB1
    
    //ADC CONFIG
    ADCON0bits.ADCS = 0b01;         // FOSC/8
    ADCON1bits.VCFG0 = 0;           // USO DE VDD COMO VOLTAJE DE REFERENCIA INTERNO
    ADCON1bits.VCFG1 = 0;           // USO DE VSS COMO VOLTAJE DE REFERENCIA INTERNO

    ADCON0bits.CHS = 0b0000;        // SELECCION DE PORTA PIN0 (AN0) COMO ENTRADA DE ADC
    ADCON1bits.ADFM = 0;            // FORMATO DE BITS JUSTIFICADOS A LA IZQUIERDA
    ADCON0bits.ADON = 1;            // HABILITACION DE MODULO DE ADC
    __delay_us(40);                 // TIEMPO DE LECTURA
}