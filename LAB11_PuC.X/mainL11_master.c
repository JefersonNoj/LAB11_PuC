/*
 * File:   mainL11_master.c
 * Author: Jeferson Noj
 *
 * Created on 11 de mayo de 2022, 09:58 AM
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

//VARIABLES --------------------------------------------------------------------
char cont_master = 0;
char cont_slave = 0xFF;
char val_temp = 0;
char POT_valor = 0;

// PROTOTIPO DE FUNCIONES ------------------------------------------------------
void setup(void);

// INTERRUPCIONES --------------------------------------------------------------
void __interrupt() isr (void){
    if (PIR1bits.SSPIF){
        PIR1bits.SSPIF = 0;         // Limpiar bandera de interrupción
    }
    
    if(PIR1bits.ADIF){              // Verificar si ocurrió interrupción del ADC
        if(ADCON0bits.CHS == 0){    // Verificar que se esta leyendo el AN0
            POT_valor = ADRESH;     // Guardar el resultado de la conversión
            PORTD = ADRESH;         // Mostrar resultado en el PORTD
        }
        PIR1bits.ADIF = 0;          // Limpiar bandera de interrupción del ADC
    }
    
    return;
}

//CICLO PRINCIPAL  -------------------------------------------------------------
void main(void) {
    setup();
    while(1){        
        if(ADCON0bits.GO == 0)  // Verificar que no hay proceso de conversión del ADC
            ADCON0bits.GO = 1;  // Inicar proceso de conversión
        
        // cambio en el selector (SS) para generar respuesta del pic
        PORTAbits.RA7 = 1;      // Deshabilitamos el ss del esclavo
        __delay_ms(10);         // Delay para que el PIC pueda detectar el cambio en el pin
        PORTAbits.RA7 = 0;      // habilitamos nuevamente el escalvo

        // Enviar dato
        SSPBUF = POT_valor;   // Cargamos valor del contador al buffer
        while(!SSPSTATbits.BF){}// Esperamos a que termine el envio
        
        // cambio en el selector (SS) para generar respuesta del pic
        PORTAbits.RA6 = 1;      // Deshabilitamos el ss del esclavo
        __delay_ms(10);         // Delay para que el PIC pueda detectar el cambio en el pin
        PORTAbits.RA6 = 0;      // habilitamos nuevamente el escalvo

        PORTB = SSPBUF;         // Mostramos dato recibido en PORTD
        
        __delay_ms(1000);       // Enviamos y pedimos datos cada 1 segundo
    }
    return;
}
// CONFIGURACION ---------------------------------------------------------------
void setup(void){
    ANSEL = 0b1;
    ANSELH = 0;
    
    TRISB = 0;
    PORTB = 0;
    
    TRISD = 0;
    PORTD = 0;
    
    TRISA = 0b1;
    PORTA = 0;
    
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Reloj interno
    
    // Configuración del ACD
    ADCON0bits.ADCS = 0b01;     // FOSC/8
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0b0000;    // Selecionar AN0
    ADCON1bits.ADFM = 0;        // Justificar a la izquierda
    ADCON0bits.ADON = 1;        // Habilitar modulo ADC
    __delay_us(40);             // Tiempo de muestreo
    
    // Configuracion de SPI
    // Configs de Maestro
    
    TRISC = 0b00010000;         // -> SDI entrada, SCK y SD0 como salida
    PORTC = 0;
    
    // SSPCON <5:0>
    SSPCONbits.SSPM = 0b0000;   // -> SPI Maestro, Reloj -> Fosc/4 (250kbits/s)
    SSPCONbits.CKP = 0;         // -> Reloj inactivo en 0
    SSPCONbits.SSPEN = 1;       // -> Habilitamos pines de SPI
    // SSPSTAT<7:6>
    SSPSTATbits.CKE = 1;        // -> Dato enviado cada flanco de subida
    SSPSTATbits.SMP = 1;        // -> Dato al final del pulso de reloj
    SSPBUF = cont_master;       // Enviamos un dato inicial
    
    // Configuración de interrupciones
    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupciones de perifericos
    PIE1bits.ADIE = 1;          // Habilitar interrupción del ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera del ADC
}