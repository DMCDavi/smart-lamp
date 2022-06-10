
/*==========================================================================================================
 ===========================================================================================================*/
#include <xc.h>                       // Incluimos todos los registros del PIC18F4550.
#include <stdint.h>                   // Incluimos esta librería para trabajar con variables enteras.
#include <stdio.h>                    // Incluimos esta librería para trabajar con perifericos de entrada/salida.
#include <string.h>                   // Incluimos esta librería para trabajar con cadenas de caracteres.
#include "bits_Configuration.h"       // Incluimos el archivo de cabecera para los fusibles.
#include "lcd.h"                      // Incluimos la librería LCD 16x2.
#include <pic18f4550.h>

#define Pin_Trig    LATAbits.LATA1    // Da-se o nome "Pin_Trig" ao pino RA0 (saida).
#define Pin_Echo    PORTAbits.RA2     // Da-se o nome "Pin_Echo" ao pino RA1 (entrada).


#define out2 RB2 //controle da saída 2
#define LED RB0


#define RESOLUTION 5/1023 // (Vref)/(2^N - 1), PIC's Vref maximum = 5 VOLTS, N=10 bits

#define F_CPU 200000/64
#define Baud_value (((float)(F_CPU)/(float)baud_rate)-1)

/*==========================================================================================================
 ===========================================================================================================*/
uint16_t LCD_Buffer [16];              // Arreglo para mostrar las variabes en la pantalla LCD 2x16.
                                       // Variavel de tipo flutuante 32 bits para o tempo.
/*==========================================================================================================
 ===========================================================================================================*/
void Configuracion_Registros (void);  // Función para configurar registros de inicio.
void Timer1_Init (void);   
void Timer3_Init (void);   // Función para inicializar el Timer 1.
uint16_t Obtener_Distancia (void);    // Función para obtener la distancia.

void Time_Bases(uint16_t, float);
void Init_AD();
float Read_LDR();
void USART_Init();
void USART_TransmitChar(char);
char USART_ReceiveChar();
void states_LED(char);


unsigned baseT1 = 0x00, //auxiliar para base de tempo 1
  baseT2 = 0x00, //auxiliar para base de tempo 2
  buttonClicked = 0,
  cycles[2] = {
    10,
    5
  },
  numBase = 0;
/*==========================================================================================================
 ===========================================================================================================*/
void main(void)                       // Función Principal.
{
    uint16_t Distancia;               // Variable Distancia.
    float LDR_value;
  
    Configuracion_Registros();        // Llamamos a la función de configuración de registros.
    Timer1_Init();
    Timer3_Init(); // Inicializamos la configuración del Timer1.
    lcd_init();                       // Inicializamos la pantalla LCD 2x16.
    Init_AD();
    
    USART_Init();
    char esp_server_data = 'a';
    
    while(1)
    {   

        //sprintf(LCD_Buffer,"Distancia: %03dcm", Distancia);//Cargamos variable "Distancia" con formato en "LCD_Buffer".
        //lcd_gotoxy(2,1);              //Ubicamos el cursor en fila 2, columna 1
        //lcd_putc(LCD_Buffer);         //Mostramos el valor de buffer_lcd
//        __delay_ms(200);
        
        //esp_server_data=USART_ReceiveChar();
        //sprintf(LCD_Buffer,"maria: %c", esp_server_data);
        //states_LED(esp_server_data);
        
        //Distancia=Obtener_Distancia();// Cargamos la variable "Distancia" con el valor de distancia capturado por el sensor HC-SR04.
        //LDR_value=Read_LDR();
      //Mostramos el valor de buffer_lcd
        if (INTCONbits.TMR0IF) //Houve estouro do TMR0?;;;
        { //Sim...
            INTCONbits.TMR0IF = 0x00; //Limpa flag
            TMR0H = 0xD8; //Reinicializa TMR0H 3c
            TMR0L = 0xF0; //Reinicializa TMR0L
            baseT1 += 1; //Incrementa baseT1
            baseT2 += 1; //Incrementa baseT2
            Time_Bases(Distancia, LDR_value); //Chama função timeBase
        }
    }
    return;
}



void Time_Bases(uint16_t Distancia, float LDR_value){
    if (baseT1 >= cycles[0]) //baseT1 igual a 2?
    { //sim...
        baseT1 = 0x00; //reinicia
        //LDR_value=Read_LDR();
        //lcd_gotoxy(1,1);              // Posicionamos el cursor en fila 1, columna 1.
        //sprintf(LCD_Buffer,"LDR: %.2f", LDR_value);//Cargamos variable "Distancia" con formato en "LCD_Buffer".
        //lcd_putc(LCD_Buffer); 
    } //end if baseT1
    if (baseT2 >= cycles[1]) //baseT1 igual a 2?
    { //sim...
        LED=~LED;
        baseT2 = 0x00; //reinicia
        //Distancia=Obtener_Distancia();// Cargamos la variable "Distancia" con el valor de distancia capturado por el sensor HC-SR04.
        //sprintf(LCD_Buffer,"Distancia: %03dcm", Distancia);//Cargamos variable "Distancia" con formato en "LCD_Buffer".
        //lcd_gotoxy(2,1);              //Ubicamos el cursor en fila 2, columna 1
        //lcd_putc(LCD_Buffer);         //Mostramos el valor de buffer_lcd
        //__delay_ms(200);
    } //end if baseT1
     
    //base de tempo de 125ms  (5 x 25ms)
}


void states_LED(char data){
    if(data){
        LATB = 0x01;
        lcd_gotoxy(2,1);              //Ubicamos el cursor en fila 2, columna 1
        lcd_putc(LCD_Buffer); 
    }
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

void USART_TransmitChar(char out)
{
   while (TXIF == 0);	/* Wait for transmit interrupt flag*/
   TXREG = out;  	/* Write char data to transmit register */    
}

char USART_ReceiveChar()
{
    while(RCIF==0);      /*wait for receive interrupt flag*/
        if(RCSTAbits.OERR)
        {           
            CREN = 0;
            NOP();
            CREN=1;
        }
        return(RCREG);       /*received in RCREG register and return to main program */
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

void Timer3_Init (void)               // Función de configuración del Timer 2.
{
    TMR0H = 0xD8; //Inicializa o TMR0H em 9Eh
    TMR0L = 0xF0; //Inicializa o TMR0L em 58h
    T0CONbits.T08BIT=0;   
    T0CONbits.T0PS0=1;
    T0CONbits.T0PS1=1;
    T0CONbits.T0PS2=0;
    // Timer1  Pre-escaler=8.
    //T0CONbits.T0SE=1;
    T0CONbits.T0CS=0;               // Internal clock (Fosc/4).// Cargamos el registro TMR1 con el valor de 0.// Temporizador Timer1 detenido.
    T0CONbits.TMR0ON=1;
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
