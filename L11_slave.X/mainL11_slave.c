/* 
 * File:   mainL11_slave.c
 * Author: Jeferson Noj
 *
 * Created on 11 de mayo de 2022, 10:01 AM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

// CONSTANTES ------------------------------------------------------------------
#define _XTAL_FREQ 1000000
#define FLAG_SPI 0xFF
#define INC_B PORTBbits.RB0     // Asignar identificador a RB0
#define DEC_B PORTBbits.RB1     // Asignar identificador a RB1

// VARIABLES -------------------------------------------------------------------
char cont_master = 0;
char cont_slave = 0xFF;
char val_temp = 0;

// PROTOTIPO DE FUNCIONES ------------------------------------------------------
void setup(void);

// INTERRUPCIONES --------------------------------------------------------------
void __interrupt() isr (void){
    if (PIR1bits.SSPIF){
        val_temp = SSPBUF;
        PORTD = val_temp;       // guardamos el dato en PORTD
        SSPBUF = cont_master;
        PIR1bits.SSPIF = 0;             // Limpiamos bandera de interrupci n?
    }
    
    if(INTCONbits.RBIF){        // Evaluar bandera de interrupción del PORTB
        if(!INC_B)          // Evaluar boton de incremento
            cont_master++;            // Aumentar PORTA si el boton de incremento se presionó 
        else if (!DEC_B)       // Evaluar boton de decremento (solo si no se presionó el de incrmento)
            cont_master--;            // Disminuir PORTA si el boton de decremento se presionó
        INTCONbits.RBIF = 0;    // Limpiar bandera de interrupción del PORTB
    }
    return;
}

// CICLO PRINCIPAL -------------------------------------------------------------
void main(void) {
    setup();
    while(1){        
        // Envio y recepcion de datos en maestro
        //PORTD = cont_master;
    }
    return;
}
// CONFIGURACION ---------------------------------------------------------------
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    
    TRISD = 0;
    PORTD = 0;
    
    TRISA = 0b00100001;
    PORTA = 0;
    
    TRISBbits.TRISB0 = 1;       // RB0 como entrada
    TRISBbits.TRISB1 = 1;       // RB1 como entrada
    OPTION_REGbits.nRBPU = 0;   // Habilitar resistencias pull-up del PORTB
    WPUBbits.WPUB0 = 1;         // Habilitar pull_up para RB0
    WPUBbits.WPUB1 = 1;         // Habilitar pull_up para RB1 
    
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Reloj interno
    
    // Configuracion de SPI
    // Configs del esclavo
    TRISC = 0b00011000; // -> SDI y SCK entradas, SD0 como salida
    PORTC = 0;

    // SSPCON <5:0>
    SSPCONbits.SSPM = 0b0100;   // -> SPI Esclavo, SS hablitado
    SSPCONbits.CKP = 0;         // -> Reloj inactivo en 0
    SSPCONbits.SSPEN = 1;       // -> Habilitamos pines de SPI
    // SSPSTAT<7:6>
    SSPSTATbits.CKE = 1;        // -> Dato enviado cada flanco de subida
    SSPSTATbits.SMP = 0;        // -> Dato al final del pulso de reloj

    PIR1bits.SSPIF = 0;         // Limpiamos bandera de SPI
    PIE1bits.SSPIE = 1;         // Habilitamos int. de SPI
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    INTCONbits.RBIE = 1;        // Habilitar interrupciones del PORTB
    IOCBbits.IOCB0 = 1;         // Habilitar interrpción On_change de RB0
    IOCBbits.IOCB1 = 1;         // Habilitar interrpción On_change de RB1
    INTCONbits.RBIF = 0;        // Limpiar bandera de interrupción del PORTB
}