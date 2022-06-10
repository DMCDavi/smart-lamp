/*?????????????????????????????????????????????????????????????????????????????????????????????????????????????
 ??????????????????????????????????????????????????????????????????????????????????????????????????????????????
 ????????????????????                                    ??????????????????????????????????????????????????????
 ????????????????????             PIC18F4550             ??????????????????????????????????????????????????????
 ????????????????????        SENSOR ULTRASONICO          ??????????????????????????????????????????????????????
 ????????????????????             HC-SR04               ??????????????????????????????????????????????????????
 ????????????????????     Canal de youtube: Jorge APC    ??????????????????????????????????????????????????????
 ????????????????????       WhatsApp =: +51921322152     ??????????????????????????????????????????????????????
 ????????????????????                                    ??????????????????????????????????????????????????????
 ??????????????????????????????????????????????????????????????????????????????????????????????????????????????
 ??????????????????????????????????????????????????????????????????????????????????????????????????????????????*/

/*El Sensor ultrasonico HC-SR04 trabaja a una frecuencia de 40KHz, mientras que el oido humano esta entre el rango de 20Hz a 20KHz.
 Rango de medición de 2 cm a 400 cm.
 Velocidad del sonido 340 m/s >>> 0.034cm/us   >>>>> (20*C).
 Formula: d=v*t  >>> Distancia = Velocidad del sonido*Tiempo de ida
 Distancia = (0.034cm/us)*(Tiempo/2)
 Distancia = 0.017*Tiempo de ida. */

/*==========================================================================================================
 ===========================================================================================================*/
#include <xc.h>                       // Incluimos todos los registros del PIC18F4550.
#include <stdint.h>                   // Incluimos esta librería para trabajar con variables enteras.
#include <stdio.h>                    // Incluimos esta librería para trabajar con perifericos de entrada/salida.
#include <string.h>                   // Incluimos esta librería para trabajar con cadenas de caracteres.
#include "bits_Configuration.h"       // Incluimos el archivo de cabecera para los fusibles.
#include "lcd.h"         // Incluimos la librería LCD 16x2.
#include <pic18f4550.h>

#define Pin_Trig    LATAbits.LATA1    // Asignamos el nombre "Pin_Trig" al pin RA0 (salida).
#define Pin_Echo    PORTAbits.RA2     // Asignamos el nombre "Pin_Echo" al pin RA1 (entrada).

#define LED RB0
#define RESOLUTION 5/1023 // (Vref)/(2^N - 1), PIC's Vref maximum = 5 VOLTS, N=10 bits

#define F_CPU 200000/64
#define Baud_value (((float)(F_CPU)/(float)baud_rate)-1)
#define waitingControlTime 10 //durante esse tempo a FSM não funciona

//Variáveis da máquina de estados
unsigned int isBright = 0; // sensor de luminosidade
unsigned int theresMovement = 0; // sensor de movimento
unsigned int contControl = 0;



/*==========================================================================================================
 ===========================================================================================================*/
uint16_t LCD_Buffer [16];              // Arreglo para mostrar las variabes en la pantalla LCD 2x16.
                                       // Variable de tipo flotante 32 bits para el tiempo.
/*==========================================================================================================
 ===========================================================================================================*/
void Configuracion_Registros (void);  // Función para configurar registros de inicio.
void Timer1_Init (void);              // Función para inicializar el Timer 1.
uint16_t Obtener_Distancia (void);    // Función para obtener la distancia.
void Init_AD();
float Read_LDR();
void USART_Init();
void inicioInterrupcoes();
//char USART_ReceiveChar();

void control_LED(char);
void check_light();
void check_movement();
void led_off();
void led_on();
void FSM();
/*==========================================================================================================
 ===========================================================================================================*/
uint16_t Distancia;               // Variable Distancia.
float LDR_value;

void main(void)                       // Función Principal.
{
    
    Configuracion_Registros();        // Llamamos a la función de configuración de registros.
    Timer1_Init();                    // Inicializamos la configuración del Timer1.
    lcd_init();                       // Inicializamos la pantalla LCD 2x16.
    Init_AD();
    
    USART_Init();
    inicioInterrupcoes();
 
    while(1){
        LATB = 0x0;
        Distancia=Obtener_Distancia();// Cargamos la variable "Distancia" con el valor de distancia capturado por el sensor HC-SR04.;;
        LDR_value=Read_LDR();
        lcd_gotoxy(1,1);              // Posicionamos el cursor en fila 1, columna 1.
        sprintf(LCD_Buffer,"LDR: %.2f", LDR_value);//Cargamos variable "Distancia" con formato en "LCD_Buffer".
        lcd_putc(LCD_Buffer);         //Mostramos el valor de buffer_lcd
        sprintf(LCD_Buffer,"Distancia: %03dcm", Distancia);//Cargamos variable "Distancia" con formato en "LCD_Buffer".
        lcd_gotoxy(2,1);              //Ubicamos el cursor en fila 2, columna 1
        lcd_putc(LCD_Buffer);         //Mostramos el valor de buffer_lcd;
        //RCIE = 1;
    }
   
    return;
}

// verificando as variáveis da máquina de estados
void check_light(){
    
}

void check_movement(){
    
}
// controle do led
void led_off(){
    LATB = 0x00;
}
void led_on(){
    LATB = 0x01;
}
// implementação da máquina de estados

void FSM(){
    //if(contControl == waitingControlTime){
        if(theresMovement && isBright){
            led_off();
        }else if(!theresMovement && !isBright){
            led_off();
         }else if(!theresMovement && isBright){
            led_off();
        }else if(theresMovement && !isBright){
            led_on();
        }
    //}
}

//controle do LED  pelo celular

void control_LED(char data){    
    if(data == 170){
        led_on();
    }else if(data == 174){
        led_off();
    }
    //timer para desconsiderar a máquina de estados
}

/*==========================================================================================================
 ===========================================================================================================*/
// Transmitindo os dados do ESP para o PIC

void USART_Init(){
    long int baud_rate = 115200;
    float temp;
    TRISC6=0;		/* Make Tx pin as output*/
    TRISC7=1;  		/* Make Rx pin as input*/

    /* Baud rate=9600, SPBRG = (F_CPU /(64*9600))-1*/
    temp= (( (float) (F_CPU) / (float) baud_rate ) - 1);     ;
    SPBRG = (int) temp;	

    //TXSTA = 0x20;  	/* Enable Transmit(TX) */ 
    RCSTA = 0x90;  	/* Enable Receive(RX) & Serial */
}

//char USART_ReceiveChar();
//{
//    while(RCIF==0);      /*wait for receive interrupt flag*/
//        if(RCSTAbits.OERR)
//        {           
//            CREN = 0;
//            NOP();
//            CREN=1;
//        }
//        return(RCREG);       /*received in RCREG register and return to main program */
//    
//}

void inicioInterrupcoes()
{
 IPEN = 1; //por prioridade
 ADIE = 1; // habilitação do fim de conversão A/D (interrupção analógica habilitada)
 RCIE = 1; // controle de habilitação da recepção usart
 TMR0IE = 1; // controle de habilitação do OVERFLOW do TMR0
 GIEH = 1; //habilita interrupções globais de alta prioridade, bit do registrado INTCON.
 GIEL = 1; //habilita interrupções globais de baixa prioridade, bit do registrador INTCON.
 ADIP = 0; // seleção da prioridade da interrupção de fim de conversão A/D (BAIXA prioridade)
 TMR0IP = 0; // seleção da prioridade da interrupção do timer0 (BAIXA prioridade)
 RCIP = 1; //seleção da prioridade da interrupção do recebimento da USART (ALTA prioridade)
}

void interrupt low_priority interrupcaoLOW(void)
{

        Distancia=Obtener_Distancia();// Cargamos la variable "Distancia" con el valor de distancia capturado por el sensor HC-SR04.;;
        LDR_value=Read_LDR();
        lcd_gotoxy(1,1);              // Posicionamos el cursor en fila 1, columna 1.
        sprintf(LCD_Buffer,"LDR: %.2f", LDR_value);//Cargamos variable "Distancia" con formato en "LCD_Buffer".
        lcd_putc(LCD_Buffer);         //Mostramos el valor de buffer_lcd
        sprintf(LCD_Buffer,"Distancia: %03dcm", Distancia);//Cargamos variable "Distancia" con formato en "LCD_Buffer".
        lcd_gotoxy(2,1);              //Ubicamos el cursor en fila 2, columna 1
        lcd_putc(LCD_Buffer);         //Mostramos el valor de buffer_lcd;
//        __delay_ms(200);;
    
//        check_light();
//        check_movement();
//        FSM();
                
}

void interrupt high_priority interrupcaoHIGH(void) //ok
{
    char esp_server_data;
    if (RCIF == 1)
    {
        esp_server_data = RCREG;
        control_LED(esp_server_data);
        RCIF =0 ;
//        RCIE = 0;
    }
}

/*==========================================================================================================
 ===========================================================================================================*/
void Configuracion_Registros (void)   //Función para configurar registros de inicio. 
{
    ADCON1bits.PCFG=0b1111;           // Deshabilitamos las entradas analógicas de los puerto A y B
    TRISA&=~(1<<1);                   // Configuramos el pin RA0 como salida .
    TRISA|=(1<<2);                    // Configuramos el pin RA1 como entrada .
//    TRISA|=(1<<2);                    // Configuramos el pin RA2 como entrada .
    TRISB = 0; //Port D is output LED -- it controls B section
}

/*==========================================================================================================
 ===========================================================================================================*/
void Timer1_Init (void)               // Función de configuración del Timer 1.
{
    T1CONbits.RD16=1;                 // Timer1 trabajando a 16 bits.
    T1CONbits.T1CKPS=0b00;            // Timer1  Pre-escaler=0.
    T1CONbits.TMR1CS=0;               // Internal clock (Fosc/4).
    TMR1=0;                           // Cargamos el registro TMR1 con el valor de 0.
    TMR1ON=0;                         // Temporizador Timer1 detenido.
}

/*==========================================================================================================
 ===========================================================================================================*/
uint16_t Obtener_Distancia (void) // Función para obtener la distancia.
{
    uint16_t Duracion;         
    uint16_t Distancia;
    uint16_t Timer_1;
    
    Pin_Trig=1;                       // Ponemos a nivel alto el pin RA0.
    __delay_us(10);                   // Retardo de 10 us.
    Pin_Trig=0;                       // Ponemos a nivel bajo el pin RA0.
    while(Pin_Echo==0);               // Esperamos mientras el pin RA1 sea igual a 0.
    T1CONbits.TMR1ON=1;               // Activamos el temporizador del Timer1.
    while(Pin_Echo==1);               // Esperamos mientras el pin RA1 sea igual a 1.
    T1CONbits.TMR1ON=0;               // Detenemos el temporizador del Timer1.
    Timer_1=TMR1;                     // Cargamos la variable "Timer_1" con el valor del registro TMR1. (Tiempo en us)
    Duracion=Timer_1/2;             // Cargamos el valor de la variable "Timer_1"/2 (debido que con 8MHz se generan 0.5us) en la variable "Duracion".
    if(Duracion<=23200)               // 23529.4 us equivalen a 400cm.
    {
        Distancia=Duracion/58;     // Valor de la distancia en cm. (formula aplicada para us)
    }
    else if(Duracion<116)           // 117.6 us equivalen a 2 cm.
    {
        Distancia=0;                // Valor de distancia en cm.
    }
    else
    {
        Distancia=0;                // Valor de distancia en cm.
    }
    Duracion=0;                     // Reiniciamos el valor de la variable "Duracion".
    TMR1=0;                           // Reiniciamos el valor del registro TMR1.
    
    return Distancia;                 // Retornamos el valor de distancia.
}

void Init_AD() {
    TRISA0 = 1; //RA0 is input (ADC) -- it controls A section
    ADCON1 = 0x0e;  		/*Ref vtg is VDD & Configure pin as analog pin*/    
    ADCON2 = 0x92;  		/*Right Justified, 4Tad and Fosc/32. */
    ADRESH=0;  			/*Flush ADC output Register*/
    ADRESL=0;
}

float Read_LDR() {
    float analog_reading = 0;
    ADCON0 |= ((1 << ADON) | (1 << GO)); /*Enable ADC and start conversion*/
    while (ADCON0bits.GO_nDONE == 1); /*wait for End of conversion i.e. Go/done'=0 conversion completed*/
    analog_reading = (ADRESH * 256) + (ADRESL); /*Combine 8-bit LSB and 2-bit MSB*/
    analog_reading = analog_reading*RESOLUTION;  // multiply the conversion bits by the conversion's resolution
    return analog_reading;
}
