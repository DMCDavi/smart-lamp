
/*O Sensor ultrasonico HC-SR04 trabalha a uma frequência de 40KHz
 Alcance de 2 cm a 400 cm.
 Velocidade do som 340 m/s >>> 0.034cm/us   >>>>> (20*C).
 Formula: d=v*t  >>> Distancia = Velocidade do som*Tempo de ida
 Distancia = (0.034cm/us)*(Tempo/2)
 Distancia = 0.017*Tempo de ida. */

/*==========================================================================================================
 ===========================================================================================================*/
#include <xc.h>                       // Incluimos todos os registradores do PIC18F4550.
#include <stdint.h>                   // Incluimos esta biblioteca para trabalhar com variaveis innteiras
#include <stdio.h>                    // Incluimos esta biblioteca para trabalhar com perifericos de entrada/saída.
#include <string.h>                   // Incluimos esta biblioteca para trabalhar com correntes de caracteres.
#include "bits_Configuration.h"       // Incluimos o arquivo header 
#include "lcd.h"         // Incluimos a biblioteca do LCD 16x2.
#include <pic18f4550.h>

#define Pin_Trig    LATAbits.LATA1    // define "Pin_Trig" ao RA0 (saída).
#define Pin_Echo    PORTAbits.RA2     //define "Pin_Echo" ao RA1 (entrada).

#define LED RB0
#define RESOLUTION 5/1023 // (Vref)/(2^N - 1), PIC's Vref maximo = 5 VOLTS, N=10 bits

#define F_CPU 200000/64
#define Baud_value (((float)(F_CPU)/(float)baud_rate)-1)
#define waitingControlTime 10 //durante esse tempo a FSM não funciona

//Variáveis da máquina de estados
unsigned int isBright = 0; // sensor de luminosidade
unsigned int theresMovement = 0; // sensor de movimento
unsigned int controlOn = 0;


/*==========================================================================================================
 ===========================================================================================================*/
uint16_t LCD_Buffer [16];              // Arranjo para mostrar as variaveis na tela LCD 2x16.
                                       // Variavel de tempo flot 32 bits para o tempo.
/*==========================================================================================================
 ===========================================================================================================*/
void Configuracion_Registros (void);  // função para configurar registros de inicio.
void Timer1_Init (void);              // função para inicializar el Timer 1.
void Timer0_Init (void);
uint16_t Obtener_Distancia (void);    // função para obter a distancia.
void Init_AD();
float Read_LDR();
void USART_Init();
void inicioInterrupcoes();
void Time_Bases();

void control_LED(char);
void check_light();
void check_movement();
void led_off();
void led_on();
void FSM();
void mainLoop();
/*==========================================================================================================
 ===========================================================================================================*/
uint16_t Distancia;               // variavel Distancia.
float LDR_value;

unsigned baseT1 = 0x00, //auxiliar para base de tempo 1
  baseT2 = 0x00, //auxiliar para base de tempo 2
  buttonClicked = 0,
  cycles[2] = {
    10,
    5
  },
  numBase = 0;

void main(void)                       // função Principal.
{
    
    Configuracion_Registros();        // configuraçaõ de registros.
    Timer1_Init();                    // inicia Timer1.
    Timer0_Init();
    lcd_init();                       // inicia LCD 2x16.
    Init_AD();						//inicia conversao AD
    Timer0_Init();
    
    USART_Init();				//inicia comunicação USART
    inicioInterrupcoes();		//habilita interrupções
 
    while(1){
        mainLoop();
    }
   
    return;
}

void mainLoop(){
    
        if (INTCONbits.TMR0IF) //Houve estouro do TMR0?;;;
        { //Sim...
            INTCONbits.TMR0IF = 0x00; //Limpa flag
            TMR0H = 0xD8; //Reinicializa TMR0H 3c
            TMR0L = 0xF0; //Reinicializa TMR0L
            baseT1 += 1; //Incrementa baseT1
            baseT2 += 1; //Incrementa baseT2
            Time_Bases(); //Chama função timeBase
        }
        
        check_light(LDR_value);;
        check_movement(Distancia);
        FSM();
}

void Time_Bases(){
    if (baseT1 >= cycles[0]) //baseT1 igual a 2?
    { //sim...
        baseT1 = 0x00; //reinicia;
        LDR_value=Read_LDR();
        lcd_gotoxy(1,1);              // posiciona na fila 1, columna 1.
        sprintf(LCD_Buffer,"LDR: %.2f", LDR_value);//carrega a variavel no "LCD_Buffer".
        lcd_putc(LCD_Buffer); 
    } //end if baseT1
    if (baseT2 >= cycles[1]) //baseT1 igual a 2?
    { //sim...
        baseT2 = 0x00; //reinicia
        Distancia=Obtener_Distancia();// guarda na variavel o valor da distancia capturada pelo sensor ultrassonico
        sprintf(LCD_Buffer,"Distancia: %03dcm", Distancia);//carrega variavel "Distancia" em "LCD_Buffer".
        lcd_gotoxy(2,1);              //posiciona fila 2, columna 1
        lcd_putc(LCD_Buffer);         //Mostramos os valores de buffer_lcd
        //__delay_ms(200);
    } //end if baseT1

    //base de tempo de 125ms  (5 x 25ms)
}

void Timer0_Init (void)               // função de configuração do timer Timer 0.
{
    TMR0H = 0xD8; //Inicializa o TMR0H em 9Eh
    TMR0L = 0xF0; //Inicializa o TMR0L em 58h
    T0CONbits.T08BIT=0;
    T0CONbits.T0PS0=1;
    T0CONbits.T0PS1=1;
    T0CONbits.T0PS2=0;
    // Timer1  Pre-escaler=8.
    //T0CONbits.T0SE=1;
    T0CONbits.T0CS=0;               // clock interno (Fosc/4).// carregamos TMR1 com valor de 0
    T0CONbits.TMR0ON=1;
}

// verificando as variáveis da máquina de estados
void check_light(){
    if (LDR_value > 4.0) {
        isBright = 0;
    } else {
        isBright = 1;
    }
}
void check_movement(){
    if(Distancia > 110) {
        theresMovement = 0;
    } else {
        theresMovement = 1;
    }
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
   
    if(controlOn){
        if(theresMovement && isBright){
            led_off();
        }else if(!theresMovement && !isBright){
            led_off();
         }else if(!theresMovement && isBright){
            led_off();
        }else if(theresMovement && !isBright){
            led_on();
        }
    }
}

//controle do LED  pelo celular

void control_LED(char data){    
    if(data == 174){
        led_on();
        controlOn = 1;
        //FSM();
    }else if(data == 170){
        led_off();
        controlOn = 0;
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
 
    RCSTA = 0x90;  	/* Enable Receive(RX) & Serial */
}

void inicioInterrupcoes()
{
 IPEN = 1; //por prioridade
 ADIE = 1; // habilitação do fim de conversão A/D (interrupção analógica habilitada)
 RCIE = 1; // controle de habilitação da recepção usart
 //TMR0IE = 1; // controle de habilitação do OVERFLOW do TMR0
 GIEH = 1; //habilita interrupções globais de alta prioridade, bit do registrado INTCON.
 GIEL = 1; //habilita interrupções globais de baixa prioridade, bit do registrador INTCON.
 ADIP = 0; // seleção da prioridade da interrupção de fim de conversão A/D (BAIXA prioridade)
 //TMR0IP = 0; // seleção da prioridade da interrupção do timer0 (BAIXA prioridade)
 RCIP = 1; //seleção da prioridade da interrupção do recebimento da USART (ALTA prioridade)
}

void interrupt low_priority interrupcaoLOW(void)
{
   mainLoop();         
}

void interrupt high_priority interrupcaoHIGH(void) //ok
{
    char esp_server_data;
    if (RCIF == 1)
    {
        esp_server_data = RCREG;
        control_LED(esp_server_data);
        RCIF =0 ;
    }
}

/*==========================================================================================================
 ===========================================================================================================*/
void Configuracion_Registros (void)   //função para configurar registradores de inicio. 
{
    ADCON1bits.PCFG=0b1111;           // desabilita as entrada analógicas da PORTA A e B
    TRISA&=~(1<<1);                   // configura pin RA1 como saída .
    TRISA|=(1<<2);                    // Configuramos pin RA2 como entrada .
//    TRISA|=(1<<2);                    // Configuramos  pin RA2 como entrada .
    TRISB = 0; //Port D is output LED -- it controls B section
}

/*==========================================================================================================
 ===========================================================================================================*/
void Timer1_Init (void)               // função de configuração Timer 1.
{
    T1CONbits.RD16=1;                 // Timer1 trabalhando a 16 bits.
    T1CONbits.T1CKPS=0b00;            // Timer1  Pre-escaler=0.
    T1CONbits.TMR1CS=0;               // Internal clock (Fosc/4).
    TMR1=0;                           // carrega TMR1 com valor de 0.
    TMR1ON=0;                         // Temporizador Timer1 definido.
}

/*==========================================================================================================
 ===========================================================================================================*/
uint16_t Obtener_Distancia (void) // função para obter distancia.
{
    uint16_t Duracion;         
    uint16_t Distancia;
    uint16_t Timer_1;
    
    Pin_Trig=1;                       //  pin RA0 em nível alto.
    __delay_us(10);                   // delay de 10 us.
    Pin_Trig=0;                       // pin RA0 em nível baixo.
    while(Pin_Echo==0);               // loop enquanto RA1 seja igual a 0.
    T1CONbits.TMR1ON=1;               // ativa temporizador do Timer1.
    while(Pin_Echo==1);               // loop enquanto RA1 seja igual a 1.
    T1CONbits.TMR1ON=0;               //  para a contagem do Timer1.
    Timer_1=TMR1;                     // guarda na variavel "Timer_1" o valor da contagem do registrador do TMR1. (Tempo em us)
    Duracion=Timer_1/2;             // guarda o valor "Timer_1"/2.
    if(Duracion<=23200)               // 23529.4 us equivale a 400cm.
    {
        Distancia=Duracion/58;     // Valor da distancia em cm. (formula aplicada para us)
    }
    else if(Duracion<116)           // 117.6 us equivale a 2 cm.
    {
        Distancia=0;                // Valor de distancia em cm.
    }
    else
    {
        Distancia=0;                // Valor de distancia em cm.
    }
    Duracion=0;                     // zera o valor da variavel "Duracion".
    TMR1=0;                           // Reinicia o TMR1 com valor 0.
    
    return Distancia;                 // Retornamos o valor de distancia.
}

void Init_AD() {
    TRISA0 = 1; //RA0 is input (ADC) -- it controls A section
    ADCON1 = 0x0e;  		/*Ref vtg is VDD & configura como pin analogico*/    
    ADCON2 = 0x92;  		/*justificado à direita, 4Tad and Fosc/32. */
    ADRESH=0;  			/*Flush ADC output Register*/
    ADRESL=0;
}

float Read_LDR() {
    float analog_reading = 0;
    ADCON0 |= ((1 << ADON) | (1 << GO)); /*ativa ADC e começa conversão*/
    while (ADCON0bits.GO_nDONE == 1); /*espera pelo fim da conversão i.e. Go/done'=0 conversão completada*/
    analog_reading = (ADRESH * 256) + (ADRESL); /*Combina 8-bit LSB e 2-bit MSB*/
    analog_reading = analog_reading*RESOLUTION;  // multiplica os bits de conversão pelo resolução.
    return analog_reading;
}
