#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 20000000 //Frequ�ncia de oscila��o igual a 20MHz
#define TAM_MAX 60//Tamanho m�ximo de bytes para a recep��o USART
#define RS LATEbits.LE0 //Bit de sele��o de dados e comando para o LCD ? conectado ao portE,0
#define RW LATEbits.LE2 //Bit de leitura/escrita do LCD - conectado ao portE,2
#define EN LATEbits.LE1//Bit de habilita��o (enable) do LCD ? conectado ao porte,0
unsigned char Linha1[]="CEFET-MG";
unsigned char Linha2[]="TCC Adriane";

/*Nome: Delay1Second
Fun��o: Espera de um segundo multiplicado pelo par�metro recebido
Par�metro de entrada: n�mero de segundos desejados (pode ser do tipo ponto flutuante ex: 0.5 ou 0.3)
Par�metro de sa�da: n�o tem.*/
void Delay1Second(float time)
{
 int i=0;
 time=time*100;
 for(i=0;i<time;i++)
 {
 __delay_ms(10); //for de 1 segundo [(10*ms)*100] vezes a entrada time
 }
}

//Fun��es associadas ao LCD
/*Nome: enviadados
Fun��o: Envia um caractere ao LCD.
Par�metro de entrada: caractere a ser enviado ao LCD
Par�metro de sa�da: n�o tem.*/
void enviadados(char dado)
{
 PORTD=dado; //envia dado pro PORT associado ao LCD
 RS = 1; //ativa o modo de sele��o como dados
 EN = 1; //ativa o LCD
 Delay1Second(0.01); //aguarda 0.01 segundo para garantir o envio
 EN = 0; //desativa o LCD
 Delay1Second(0.01); //aguarda 0.01 segundo
}
/*Nome: texto
Fun��o: Envia um texto ao LCD caractere por caractere utilizando a fun��o enviadado.
Par�metro de entrada: texto a ser enviado ao LCD e seu tamanho
Par�metro de sa�da: n�o tem.*/
void texto(char *apontador,char tamanho)
{
 int i = 0;
 while(i<tamanho-1 ) //como come�a em zero o �ltimo caractere � tamanho-1
{
 enviadados(apontador[i]); //vai enviando caractere por caractere
 i=i++;
 }
}
/*Nome: comando
Fun��o: Envia byte de comando ao LCD.
Par�metro de entrada: comando a ser enviado ao LCD
Par�metro de sa�da: n�o tem.*/
void comando(char dado)
{
 PORTD=dado; // envia dado para o PORT associado ao LCD
 RS = 0; //ativa o modo de sele��o como comandos
 EN = 1; //ativa o LCD
 Delay1Second(0.01); //aguarda 0.01 segundo para garantir o envio
 EN = 0; //desativa o LCD
 Delay1Second(0.01); //aguarda 0.01 segundo para garantir o envio
}

void inicioLCD()
{
 TRISD = 0x00; //configura os pinos do LCD conectados ao port D como saida
 TRISE = 0x00; //configura os pinos do LCD conectados ao port E saida
 LATD = 0x00; //envia zero para D
 LATE = 0; //envia zero para E
 RS = 0; //ativa o modo de sele��o como comandos
 RW = 0; //desativa modo leitura e escrita
 EN = 0; //desativa o LCD
 //envio da primeira mensagem
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor
 comando(0b00111100); //0X3C defini��o do tipo do display
 comando (0b00000110);// 0x06 Sentido de deslocamento do cursor
 //na entrada de um novo caractere para a direita
 comando (0b00001100); //0x0C Controle do Cursor: inativo
 comando (0b10000100); //0x84 sele��o do endere�o como quinto caractere da primeira linha
 texto(&Linha1,sizeof(Linha1)); //envia mensagem para a primeira linha
 comando (0b11000010); //0xC2 sele��o do endere�o como terceiro caractere da segunda linha
 texto(&Linha2,sizeof(Linha2)); //envia mensagem para a segunda linha
 Delay1Second(2);//aguarda 2 segundos
}

float analog_reading = 0;

void Init_AD() {
  ADCON0 = 0x01; //select channel AN0, enable A/D module
  ADCON1 = 0x0E; //use VDD, VSS as reference and configure AN0 for analog
  ADCON2 = 0xA6; //result right justified, acquisition time = 8 TAD, FOSC/64
  ADRESH = 0; /* Flush ADC output Register */
  ADRESL = 0;
}

void Config_Ports() {
  TRISA = 0xFF; //sets PORTA as all inputs, bit0 is AN0
  TRISB = 0x00; //sets PORTB as all outputs
  PORTB = 0x00; //sets all outputs of PORTB off to begin with
}

void main() {
  Init_AD();
  Config_Ports();
  inicioLCD();
  while (1) {
    ADCON0 |= ((1<<ADON)|(1<<GO)); /*Enable ADC and start conversion*/
    while(ADCON0bits.GO_nDONE==1); /*wait for End of conversion i.e. Go/done'=0 conversion completed*/
    analog_reading = (ADRESH * 256) + (ADRESL); /*Combine 8-bit LSB and 2-bit MSB*/

    if (analog_reading > 20 && analog_reading < 30) {
      LATB = 0x80;
    } else if (analog_reading >= 30 && analog_reading < 40) {
      LATB = 0xC0;
    } else if (analog_reading >= 40 && analog_reading < 50) {
      LATB = 0xE0;
    } else if (analog_reading >= 50 && analog_reading < 60) {
      LATB = 0xF0;
    } else if (analog_reading >= 60 && analog_reading < 70) {
      LATB = 0xF8;
    } else if (analog_reading >= 70 && analog_reading < 80) {
      LATB = 0xFC;
    } else if (analog_reading >= 80 && analog_reading < 90) {
      LATB = 0xFE;
    } else if (analog_reading >= 90) {
      LATB = 0xFF;
    } else {
      LATB = 0x00;
    }

  }
}