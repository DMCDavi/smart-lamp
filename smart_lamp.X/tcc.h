/* C�digo de Controle de Luminosidade via Wi-Fi e Web Page by Adriane Rodange Marteletto */
//Inclus�o das bibliotecas necess�rias.
//Defini��o das configura��es padr�es do PIC18F4550 para frequ�ncia de oscila��o em 20Mhz
#include <xc.h>
#include <stdio.h>

#pragma configPLLDIV = 5//Divide por 5 para obter 20MHz
#pragma configCPUDIV = OSC1_PLL2//Seleciona o fator de divis�o do clock por 1
#pragma configFOSC = HS //Define os osciladores de CPU e USB como Fosc =20MHz Tcy = 200ns
#pragma configWDT = OFF// Watchdog desativado
#pragma configPBADEN = OFF// Define pinos do PORTB[4:0] como entrada digitais
#pragma configLVP = OFF// Desabilita grava��o em baixa tens�o
#pragma configXINST = OFF//Desabilita modo de instru��o estendido
//Defini��o das vari�veis e seus nomes usados no c�digo
#define _XTAL_FREQ 20000000 //Frequ�ncia de oscila��o igual a 20MHz
#define TAM_MAX 60//Tamanho m�ximo de bytes para a recep��o USART
#define RS LATEbits.LE0 //Bit de sele��o de dados e comando para o LCD ? conectado ao portE,0
#define RW LATEbits.LE2 //Bit de leitura/escrita do LCD - conectado ao portE,2
#define EN LATEbits.LE1//Bit de habilita��o (enable) do LCD ? conectado ao porte,0
//Inicializa��o das vari�veis globais usadas no programa

unsigned char Rxdata[TAM_MAX];//Vari�vel que armazena dados recebidos pela USART;
int PONTOS =44;//N�mero de pontos nos dados da medi��o experimental
int ValorPORCatua,Volt,Valor_convAD, Valor_convAD11;//Valores do conversor AD
unsigned int ValorPORCatual; //Valor da porcentagem atual
char contador = 0;//contador para �ndice de Rxdata
int PERIODO = 245;//defini��o do valor do per�odo para o PWM
unsigned long querolux;//valor da luminosidade desejada
unsigned int Duty,Duty_ac,PorceDesejada;//variav�s relacionadas ao duty cycle e � porcentagem desejada
int LUX_A=650; //m�xima luminosidade (valor variado durante os testes)
int DUTY_A=1000;//m�ximo duty considerado
char confB2,confB1,confB0, Dut;//vari�veis para de retorno das buscas em Rx
char Desejado; //valor da porcentagem desejado
int confCONNECT,confCLOSED, confpagina, confGET,confGETBotao,confIPD, confGEThttp;
//vari�veis para de retorno das buscas em Rx
//Dados da medi��o experimental usados para interpola��o
long int lux[] = {31,55,144,293,783,1124,1534,2003,3128,3801,4480,5250,6980,7930,8940,10000,12280,13550,14840,16190,19010,20460,22260,23900,27330,29100,30940,32830,36710,38500,40500,42600,46900,48400,50200,53800,58600,61000,63600,64800,67400,68600,69200,69200}; //dados da medi��o da lumin�ncia multiplicados por 100
long int vtens[]={4,6,7,1235,3540,3920,4158,4311,4492,4559,4602,4639,4696,4717,4735,4751,4777,4788,4798,4807,4822,4828,4834,4839,4848,4852,4856,4860,4867,4869,4872,4874,4808,4881,4883,4886,4889,4892,4893,4894,4895,4896,4896,4896}; //dados da medi��o da tens�o multiplicados por 1000
long int dutymed[]={0,200,400,600,1000,1200,1400,1600,2000,2200,2400,2600,3000,3200,3400,3600,4000,4200,4400,4600,5000,5200,5400,5600,6000,6200,6400,6600,7000,7200,7400,7600,8000,8200,8400,8600,9000,9200,9400,9500,9700,9800,9900,10000}; //dados do duty cycle multiplicados por 100
//Lista de comandos AT para configura��o do EPS
unsigned char MsgFromPIC1[] = "AT\r\n";
unsigned char MsgFromPIC2[] = "AT+RST\r\n";
unsigned char MsgFromPIC3[] = "ATE0\r\n";
unsigned char MsgFromPIC4[] = "AT+CWMODE=2\r\n";
unsigned char MsgFromPIC5[] = "AT+CIPMUX=1\r\n";
unsigned char MsgFromPIC6[] = "AT+CWDHCP=0,0\r\n";
unsigned char MsgFromPIC7[] = "AT+CWSAP=\"ESPAdriane\",\"23456789\",7,2\r\n";
unsigned char MsgFromPIC8[] = "AT+CIPSERVER=1,80\r\n";
unsigned char MsgFromPIC9[] = "AT+CIPAP=\"192.168.5.1\"\r\n";
unsigned char MsgFromPIC10[] = "AT+CIPSTO=1000\r\n";
//Comandos para enviar Web Page
//comando AT
unsigned char MsgFromPIC11[]= "AT+CIPSEND=0,720\r\n";
//comandos html
unsigned char paginaHTML1[]="<html><head><meta http-equiv=\"refresh\" content=\"30\"><title>TCC Adriane</title></head><h1>PROGRAMA ESP8266</h1><body align=\"center\"><BODY BGCOLOR=\"#B0C4DE\"><p><form method=\"GET\"></p><TABLE><TR><input type=\"submit\" name=\"BOTAO\" value=\"Ligar\"><input type=\"submit\" name=\"BOTAO\" value=\"Desligar\" ><TR></TABLE>";
unsigned char paginaHTML2[]="<h3>Controle de intensidade luminosa: 0% a 100%</h3>";
unsigned char paginaHTML3[]="% de sua luminosidade m&aacutexima.</p><h4>By: Adriane Rodange</h4></body></html>";
unsigned char stra[310];// armazenar string a ser enviada para p�gina
//Mensagens a serem exibidas no display LCD
unsigned char Linha1[]="CEFET-MG";
unsigned char Linha2[]="TCC Adriane";
unsigned char Erro[]="ERRO: Reinicie ";
unsigned char OKConf[]="ESP Configurado";
unsigned char Aguard[]="Aguardando...";
unsigned char Connect[]="Conectado ";
unsigned char Tnova[]="Pausado ";
unsigned char Pag[]="Pagina Enviada ";
unsigned char Lamp0[]="Duty em: ";
unsigned char Lamp1[]="Desejado: ";
unsigned char Lamp[]="Lampada em: ";
//Buffers para armazenar o valor da porcentagem
unsigned char buffer[3],buffera[5];
//Vari�veis associadas � configura��o do m�dulo
unsigned char modconf = 'A';//inicio
unsigned char FIM = 'z';//fim



//INICIO DAS FUN��ES
/*Nome: CharToInt
Fun��o: obtem o inteiro de um char que seja de 0 a 9
 * 
Par�metro de entrada: char a ser obtido i inteiro.
Par�metro de sa�da: retorna o inteiro obtido ou 10 */
int CharToInt(unsigned char conv_char) {
 switch(conv_char)
 
{
 case '0':
 return(0);
 case '1':
 return(1);
 case '2':
 return(2);
 case '3':
 return(3);
 case '4':
 return(4);
 case '5':
 return(5);
 case '6':
 return(6);
 case '7':
 return(7);
 case '8':
 return(8);
 case '9':
 return(9);
 default:
 return(10);
 
}
}
/*Nome: IntToChar
Fun��o: Armazena um inteiro em formato de char em buffera.
Par�metro de entrada: inteiro a ser transformado
Par�metro de sa�da: n�o tem.*/
void IntToChar (int aduty) {
 int div1,div2,div3,div4;
 int rest1,rest2,rest3,rest4; //4.873->0,0
 //aduty vai de 0 a 100, int de 0 a 65mil 94
 div1=aduty/10000; //0.0094
 rest1=aduty%10000; //94
 div2=rest1/1000; //0.094
 rest2=rest1%1000; //94
 div3=rest2/100; //0.94
 rest3=rest2%100; //94
 div4=rest3/10; //9.4
 rest4=rest3%10; //4
 buffera[0]=div1+0x30;
 buffera[1]=div2+0x30;
 buffera[2]=div3+0x30;
 buffera[3]=div4+0x30; //quero
 buffera[4]=rest4+0x30; //quero
}
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
/*Nome: limpa_rx
Fun��o: Limpa a vari�vel de recep��o USART colcando todos os seus valores em zero.
Par�metro de entrada: n�o tem.
Par�metro de sa�da: n�o tem.*/
void limpa_rx()
{
 int i = 0;
 while(i<(TAM_MAX-1))
 {
 Rxdata[i]=0x00;
 i++;
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
//Fun��es de busca
/*Nome: buscaOK
Fun��o: Busca OK em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaOK (unsigned char *data)
{
 int i =0;
 while(i<TAM_MAX)
 {
 if(( data[i] == 'O')&&( data[i+1] == 'K'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaready
Fun��o: Busca ready em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaready (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if( (data[i] == 'r')&&( data[i+1] == 'e')&&(data[i+2] == 'a')&&(data[i+3] == 'd')&&(data[i+4] == 'y'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaCLOSED
Fun��o: Busca CLOSED em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */

int buscaCLOSED (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if( (data[i] == 'C')&&( data[i+1] == 'L')&&(data[i+2] == 'O')&&(data[i+3] == 'S')&&(data[i+4] == 'E')&&(data[i+5] == 'D'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscabuilded
Fun��o: Busca builded em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscabuilded (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if( (data[i] == 'b')&&( data[i+1] == 'u')&&(data[i+2] == 'i')&&(data[i+3] == 'l')&&(data[i+4] == 'd')&&(data[i+5] == 'e')&& (data[i+6] 
== 'd'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaCONNECT
Fun��o: Busca CONNECT em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaCONNECT(unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if( (data[i] == 'C')&&( data[i+1] == 'O')&&(data[i+2] == 
'N')&&(data[i+3]=='N')&&(data[i+4]=='E')&&(data[i+5]=='C')&&(data[i+6]=='T'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaGET
Fun��o: Busca GET em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaGET (unsigned char *data) //ok

{
 int i =0;
 while(i<TAM_MAX)
 {
 if( (data[i] == 'G')&&( data[i+1] == 'E')&&(data[i+2] == 'T'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaIPD
Fun��o: Busca +IPD em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaIPD(unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if( (data[i] == '+')&&( data[i+1] == 'I')&&(data[i+2] == 'P')&&(data[i+3]=='D'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaGEThttp
Fun��o: Busca HTTP em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaGEThttp (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if( (data[i] == 'H')&&(data[i+1] == 'T')&&(data[i+2] == 'T')&&(data[i+3] == 'P'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaB1
Fun��o: Busca BOTAO=L em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaB1 (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)

 {
 if(( data[i] == 'B')&&( data[i+1]== 'O')&&( data[i+2]== 'T')&&( data[i+3]== 'A')&&( data[i+4]== 'O')&&( data[i+5]== '=')&&( 
data[i+6]== 'L'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaB0
Fun��o: Busca BOTAO=D em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaB0 (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if(( data[i] == 'B')&&( data[i+1]== 'O')&&( data[i+2]== 'T')&&( data[i+3]== 'A')&&( data[i+4]== 'O')&&(data[i+5]== '=')&&( 
data[i+6]== 'D'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaB2
Fun��o: Busca BOTAO=C em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaB2 (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)
 {
 if(( data[i] == 'B')&&( data[i+1]== 'O')&&( data[i+2]== 'T')&&( data[i+3]== 'A')&&( data[i+4]== 'O')&&( data[i+5]== '=')&&( 
data[i+6]== 'C'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaBotao
Fun��o: Busca BOTAO em um texto utilizando verifica��o caractere por caractere do texto.
Par�metro de entrada: texto
Par�metro de sa�da: 0 se n�o encontrar, 1 se encontrar. */
int buscaBotao (unsigned char *data) //ok
{
 int i =0;
 while(i<TAM_MAX)

 {
 if( (data[i] == 'B')&&(data[i+1] == 'O')&&(data[i+2] == 'T')&&(data[i+3] == 'A')&&(data[i+4] == 'O'))
 {
 return (1);
 }
 i++;
 }
return (0);
}
/*Nome: buscaDuty
Fun��o: Busca o valor inteiro de Duty em um texto utilizando verifica��o caractere por caractere do texto. Este valor que pode ser da 
ordem de unidade, dezena ou centena encontra-se como Duty=(valor)&.
Par�metro de entrada: texto
Par�metro de sa�da: 101 se n�o encontrar um valor ou retorna o valor que encontrar. */
int buscaDuty (unsigned char *data) //ok sem delay
{
 unsigned int j =0;
 int unidade, centena, dezena, Dut;
 while(j<TAM_MAX)
 {
 if( (data[j] == 'D')&&(data[j+1] == 'u')&&(data[j+2] == 't')&&(data[j+3] == 'y')&&(data[j+4] == '='))
 {
 if(data[j+6]== '&') //so tem um digito
 {
 unidade=CharToInt(data[j+5]);
 Dut=unidade;
 return (Dut);
 }
 if(data[j+7]=='&') //tem unidade e dezena
 {
 dezena=CharToInt(data[j+5]);
 unidade=CharToInt(data[j+6]);
 Dut=10*dezena+unidade;
 return (Dut);
 }
 if(data[j+8]=='&') //centena
 {
 centena=CharToInt(data[j+5]);
 dezena=CharToInt(data[j+6]);
 unidade=CharToInt(data[j+7]);
 Dut=100*centena+10*dezena+unidade;
 return (Dut);
 }
 }
 j++;
 }
return (101);
}
/*Nome: buscaDutychar
Fun��o: Busca o valor de Duty em formato de char em um texto utilizando verifica��o caractere por caractere do texto. Este valor que 
pode ser da ordem de unidade, dezena ou centena encontra-se como Duty=(valor)&. O char � armazenado na vari�vel buffer caso seja 
encontrado.
Par�metro de entrada: texto
Par�metro de sa�da: n�o tem. */
void buscaDutychar (unsigned char *data) //ok sem delay

{
 unsigned int j =0;
 while(j<TAM_MAX)
 {
 if( (data[j] == 'D')&&(data[j+1] == 'u')&&(data[j+2] == 't')&&(data[j+3] == 'y')&&(data[j+4] == '='))
 {
 if(data[j+6]== '&') //so tem um digito
 {
 buffer[1] = (data[j+5]);
 buffer[0] = '0';
 }
 if(data[j+7]=='&') //tem unidade e dezena
 {
 buffer[0] = data[j+5];
 buffer[1] = data[j+6];
 }
 if(data[j+8]=='&')
 {
 buffer[2] = '0';
 buffer[1] = '0';
 buffer[0] = '1';
 }
 if(data[j+5]== '0') //so tem um digito
 {
 buffer[0] = '0';
 buffer[1] = '0';
 }
 }
 j++;
 }
}
//FUN��ES DE INICIALIZA��O
/*Nome: inicioUSART
Fun��o: Configura a USART e seus pinos.
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
void inicioUSART()
{
 TRISCbits.RC6 = 0; //pino TX � saida
 TRISCbits.RC7 = 1; //pino RX � entrada
 OpenUSART( USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH,10 ); //defini��o do bauld rate como high (/16) 10 � o spbrg para baud rate de 115200 em 20MHz
}
/*Nome: inicioADC
Fun��o: Configura o conversor ADC.
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
void inicioADC()
{
 OpenADC(ADC_FOSC_32 & ADC_RIGHT_JUST & ADC_0_TAD, ADC_CH0 & ADC_INT_ON & ADC_REF_VDD_VSS, ADC_1ANA);//seleciona as portas 0 e 1 como anal�gicas
}
/*Nome: inicioInterrupcoes
Fun��o: Configura as interrup��es do timer0, do ADC e da USART
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
void inicioInterrupcoes()
{
 IPEN = 1; //por prioridade
 ADIE = 1; // habilita��o do fim de convers�o A/D (interrup��o anal�gica habilitada)
 RCIE = 1; // controle de habilita��o da recep��o usart
 TMR0IE = 1; // controle de habilita��o do OVERFLOW do TMR0
 GIEH = 1; //habilita interrup��es globais de alta prioridade, bit do registrado INTCON.
 GIEL = 1; //habilita interrup��es globais de baixa prioridade, bit do registrador INTCON.
 ADIP = 0; // sele��o da prioridade da interrup��o de fim de convers�o A/D (BAIXA prioridade)
 TMR0IP = 0; // sele��o da prioridade da interrup��o do timer0 (BAIXA prioridade)
 RCIP = 1; //sele��o da prioridade da interrup��o do recebimento da USART (ALTA prioridade)
}
/*Nome: inicioTIMER
Fun��o: Configura o timer0 e o timer2.
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
void inicioTIMER()
{
 //TIMER0 para a convers�o AD
 TRISAbits.TRISA4 = 1; //define como pino do timer0 input
 TMR0IF = 0; //zera o flag de interrup��o (deve ser feito antes de habilitar)
 TMR0ON=1; // habilita timer0
 T08BIT = 1; // configura como 8bits
 T0CS = 0; // configurado como timer (a partir do clock)
 PSA = 1; // sem pre escala do clock
 T0CONbits.T0PS2=1; //prescale de 1:128
 T0CONbits.T0PS1=1;
 T0CONbits.T0PS0=0;
 TMR0L = 0; //define valor inicial como 0
 //TIMER2 para o PWM
 OpenTimer2(TIMER_INT_OFF & // sem interrup��o
 T2_PS_1_16 & // pr�-escala de 1:16
 T2_POST_1_1);// p�s-escala de 1:1
 WriteTimer2(0);// define valor inicial como 0
}
/*Nome: inicioPWM
Fun��o: Configura o PWM.
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
void inicioPWM()
{
 TRISCbits.RC2 = 0;// pino do PWM como sa�da
 OpenPWM2(PERIODO); //inicializa PWM na frequ�ncia definida pela vari�vel PERIODO
}
/*Nome: inicioLCD
Fun��o: Configura o LCD e exibe a primeira mensagem.
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
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
/*Nome: inicioInterp
Fun��o: Multiplica os valores de vtens medidos por 8. Isso � feito porque o ajuste do divisor de tens�o resulta em vtens_novo=4/5 * 
vtens, o que resulta em zero. Logo multiplica-se por 10 e divide por 5, resultando em 8. Os valores de vtens s�o agora miltiplicados por 
10mil.
Par�metro de entrada: n�o tem.
Par�metro de sa�da: n�o tem.*/
void inicioInterp()
{
 int dan=0;
 while (dan<PONTOS)
 {
 vtens[dan]=8*vtens[dan];
 dan=dan+1;
 }
}
//Tratamento das interrup��es
/*Nome: interrupcaoLOW tipo interrupt low_priority
Fun��o: Verifica qual interrup��o do tipo LOW ocorreu. Se for do conversor AD (ADIF=1): salva o valor da convers�o na vari�vel 
Valor_convAD e limpa o flag da interrup��o. Se for do TIMER0: Limpa o flag da interrup��o e ativa o conversor AD.
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
void interrupt low_priority interrupcaoLOW(void)
{
 if(ADIF == 1)
 {
 Valor_convAD=ADRES;
ADIF = 0;
 }
 if (TMR0IF == 1)
 {
 TMR0IF = 0;
 ADCON0bits.GO_DONE=1; //ativa a interrup��o so conversor para pegar o valor analogico
 }
}
/*Nome: interrupcaoHIGH tipo interrupt high_priority
Fun��o: Se a interrup��o HIGH que ocorreu for do repector da USART salva o dado recebido caractere por caractere at� o tamanho 
m�ximo definido e fica sobreescrevendo no ultimo endere�o.Em seguida limpa o flag da interrup��o.
Par�metro de entrada: n�o tem
Par�metro de sa�da: n�o tem.*/
void interrupt high_priority interrupcaoHIGH(void) //ok
{
 if (RCIF == 1)
 {
 Rxdata[contador]=RCREG;
 contador=contador++;
 if (contador == TAM_MAX)
 {
 contador = TAM_MAX-1;
 }
 RCIF=0;
 }
}
//Fun��es de calculos
/*Nome: VoltAcertado
Fun��o: Realiza a compara��o e se necess�rio a interpola��o dos dados medidos para buscar o valor de luminosidade que corresponde � 
tens�o passada como parametro a ele
Par�metro de entrada: valor de tens�o a ser interpolado
Par�metro de sa�da: valor de lux associado a esta tens�o.*/
long VoltAcertado (unsigned int Volt)
{
 char ia=0;
 unsigned int lux1, lux3, lux7;
 unsigned long lux2,lux5, lux6, luxTotal;
 while(ia<PONTOS) //4790
 {
 if ((Volt>vtens[ia])&&(vtens[ia+1]>Volt))
 { //lux_int= lux2(i) + (lux2(i+1)-lux2(i))*((valor-vcalc(i))/(vcalc(i+1)-vcalc(i)));
 //faz os calculos por etapas para n�o causar estouro nas variaveis
 lux1 =(vtens[ia+1]-vtens[ia]);
 lux2=(ValorPORCatual-vtens[ia])*1000;
 lux3=(lux[ia+1]-lux[ia]); 
 lux5=lux2/lux1;
 lux6=(lux3*lux5)/1000; 
 lux7=(lux[ia]);
 luxTotal=lux7+lux6;
 return(luxTotal);
 }
 if (Volt >= vtens[PONTOS-1])
 {
 luxTotal = LUX_A; 
 return(luxTotal);
 }
 if (Volt == vtens[ia])
 {
 luxTotal = (lux[ia]); 
 return(luxTotal);
 }
 if(Volt < vtens[0])
 {
 luxTotal = 0;
 return(luxTotal);
 }
 ia=ia+1;
 }
}
/*Nome: PorcentagemLux
Fun��o: Realiza o calculo da porcentagem de um determinado lux em relaz�o ao lux m�ximo definido no inicio do c�digo
Par�metro de entrada: luminosidade (lux)
Par�metro de sa�da: porcentagem de lux m�ximo associado � luminosidade de entrada.*/
int PorcentagemLux (long Luxaa)
{
 int lux_porc; 
 long Lux, lux_porc1, lux_porc2, lux_porc3;
 if(Luxaa==LUX_A)
 {
 lux_porc=10000;
 return(lux_porc);
 }
 Lux=(long)(Luxaa);
 //porcentagem=[(valor)/(max)]*100 => ([Luxaa/(650*100)]*100)*100(esse ultimo � pq
 //a porcentagem desejada � vezes 100, logo [Luxaa/650]*100
 lux_porc1=10*Lux;
 lux_porc2=(lux_porc1*10);
 lux_porc3=lux_porc2/LUX_A; 
 lux_porc=(int)(lux_porc3);
 return(lux_porc);
}
/*Nome: trataAD
Fun��o: Realiza o calculo da tens�o associada ao digital recebido.
Par�metro de entrada: valor digital da convers�o anal�gico digital.
Par�metro de sa�da: tens�o associada ao digital de entrada.*/
unsigned int trataAD (int ValorAD)
{
 //V_atual=((4,7V*1000)/1024)*Binario recebido =>V_atual=4.59*binario => arredonda para 46 [0-47104]
 int a;
 long Vatual;
 unsigned int Valoratual;
 a=ValorAD;
 Vatual =2*a; //2*23=46
 Vatual =23*Vatual;
 Valoratual=Vatual; 
 return(Valoratual);
}
/*Nome: DutyAcertado
Fun��o: Realiza a compara��o e se necess�rio a interpola��o dos dados medidos para buscar o valor de luminosidade desejada e a qual 
duty cycle ele corresponde
Par�metro de entrada: valor de luminosidade desejado
Par�metro de sa�da: valor de duty cycle associado a esta luminosidade.*/
long DutyAcertado (long int querolux)
{
 char abacate=0;
 unsigned long ajusteTotal,ajuste, ajuste1, ajuste2, ajuste3, ajuste4, ajuste5, ajuste6,ajusteextra, ajuste7, ajuste8;
 while(abacate<PONTOS)
 {
 if ((querolux>lux[abacate])&&(lux[abacate+1]>querolux))
 { //duty_int= duty(i) + (duty(i+1)-duty(i))*((querolux-lux(i))/(lux(i+1)-lux(i)));
 //exemplo para 99% => querolux=64350 esta entre 636.0*100 648*100 e Duty=94 e 95
 ajuste1 =(dutymed[abacate+1]-dutymed[abacate]);
 //ajuste1 = 94*100 - 95*100 = 100
 ajuste2=(querolux-lux[abacate]);
 //ajuste2 = 64350 - 636.0*100 = 750
 ajuste3=(lux[abacate+1]-lux[abacate]);
 //ajuste3 = 648*100 - 636.0*100 = 1200
 ajuste4=(1000*ajuste2);
 //ajuste4 = 1.000 * 750 = 750.000
 ajuste5=ajuste4/ajuste3;
 //ajuste5 = 750.000/1.200 = 625
 ajuste6=(ajuste1*ajuste5)/100;
 //ajuste6 = 100*625 = 62500/100=625
 ajusteextra=(dutymed[abacate]); //94
 ajuste7= 10*ajusteextra;
 //ajuste7 = 10* 94*100 = 94.000;
 ajuste8=ajuste7+ajuste6;
 //ajuste8 = 94.000 + 625 = 94.625
 ajusteTotal=(ajuste8)/100;
 //ajuste = 94625 /1.000 = 94.625 => 94
 return(ajusteTotal);
 }
 if (querolux == lux[abacate])
 {
 ajuste = (dutymed[abacate])/100;
 return(ajuste);
 }
 if (querolux > lux[PONTOS])
 {
 ajuste = (dutymed[PONTOS])/100;
 return(ajuste);
 }
 if(querolux < lux[0])
 {
 ajuste = 0;
 return(ajuste);
 }
 abacate=abacate+1;
 }
}
//Fun��es relacionadas ao Wf-Fi
/*Nome: conf_ESP8266
Fun��o: Realiza a configura��o do m�dulo ESP na ordem de comandos definidas usando as fun��es de busca para verificar as respostas 
anteriores e enviar o comando seguinte. Caso a resposta n�o seja encontrada volta ao comando inicial.
Par�metro de entrada: n�o tem.
Par�metro de sa�da: n�o tem.*/
void conf_ESP8266() 
{
 switch (modconf)
 {
 case 'A':
 {
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC1); // ativa interrup��o
 Delay1Second(0.3);
 if(buscaOK(Rxdata) == 0)
 {
 contador=0;
 modconf= 'A';
 }
 else
 {
 contador=0;
 modconf= 'B';//vai para proximo passo
 }
 }break;
 case 'B':
 {
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC2);
 Delay1Second(3); //espera 5 segundos para ter certeza de que resetou por seguran�a
 if ((buscaOK(Rxdata) == 0)&&(buscaready(Rxdata) == 0))
 {
 modconf= 'A';
 contador=0;
 }
 else
 {
 contador=0;
 modconf= 'C';//vai para proximo passo
 }
 }break;
 case 'C':
 {
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC3);
 Delay1Second(0.3);
 if (buscaOK(Rxdata) == 0)
 {
 modconf= 'A';
 contador=0;
 }
 else
 {
 contador=0;
 modconf= 'D';//vai para proximo passo
 
}
 }break;
 
case 'D':
 
{
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC4);
 Delay1Second(0.3);
 if (buscaOK(Rxdata) == 0)
 
{
 modconf= 'A';
 contador=0;
 
}
 else
 
{
 contador=0;
 modconf= 'E';//vai para proximo passo
 
}
 }break;
 case 'E':
 
{
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC5);
 Delay1Second(0.3);
 if ((buscaOK(Rxdata) == 0)&&(buscabuilded(Rxdata) == 0))
 
{
 modconf= 'A';
 contador=0;
 
}
 else
 
{
 contador=0;
 modconf= 'F';//vai para proximo passo
 
}
 }break;
 case 'F':
 
{
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC6);
 Delay1Second(0.3);
 if (buscaOK(Rxdata) == 0)
 
{
 modconf= 'A';
 contador=0;
 
}
 else
 
{
 contador=0;
 modconf= 'G';//vai para proximo passo
 
}
 }break;
 case 'G':
 
{
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC7);
 Delay1Second(0.3);
 if (buscaOK(Rxdata) == 0)
 
{
 modconf= 'A';
 contador=0;
 
}
 else
 
{
 contador=0;
 modconf= 'H';//vai para proximo passo
 
}
 }break;
 case 'H':
 
{
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC8);
 Delay1Second(0.3);
 if (buscaOK(Rxdata) == 0)
 
{
 Delay1Second(1);
 modconf= 'A';
 contador=0;
 
}
 else
 
{
 contador=0;
 modconf= 'I';//vai para proximo passo
 
}
 }break;
 case 'I':
 
{
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC9);
 Delay1Second(0.3);
 if (buscaOK(Rxdata) == 0)
 
{
 modconf= 'A';
 contador=0;
 
}
 else
 
{
 contador=0;
 modconf= 'J';//vai para proximo passo
 
}
 }break;
 case 'J':
 
{
 limpa_rx();
 while(BusyUSART());
 putsUSART(MsgFromPIC10);
 Delay1Second(0.3);
 if (buscaOK(Rxdata) == 0)
 
{
 modconf= 'A';
 contador=0;
 }
 else
 {
 Delay1Second(0.3);
 contador=0;
 modconf= FIM;//termina
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 texto(&OKConf,sizeof(OKConf)); //envia a mensagem ao usu�rio de que a configura��o foi feita
 Delay1Second(2);
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 comando (0b11000000); //0xC0 onde come�a os caracteres da segunda linha
 texto(&Aguard,sizeof(Aguard)); //envia a mensagem 'Aguardando...'
 }
 }break;
 default:
 {
 modconf= 'A';
 }
 }
}
/*Nome: paginahtml
Fun��o: Envia o comando AT que sinaliza o pedido por HTTP e envia a p�gina em seguida avisando o usu�rio atrav�s do display LCD.
Par�metro de entrada: n�o tem.
Par�metro de sa�da: n�o tem.*/
void paginahtml() 
{
 putsUSART(MsgFromPIC11);
 Delay1Second(0.25);
 putsUSART(paginaHTML1);
 Delay1Second(0.25);
 putsUSART(paginaHTML2);
 sprintf(stra, "<input type=\"range\" name=\"Duty\" min=\"0\" max=\"100\" value=\"%d\" oninput=\"this.form.ADuty.value=this.value\" /><input type=\"number\" name=\"ADuty\" min=\"0\" max=\"100\" value=\"%d\" oninput=\"this.form.Duty.value=this.value\" /><input type=\"submit\" name=\"BOTAO\" value=\"Controle\" >", Duty_ac/10,Duty_ac/10);
 putsUSART(stra);
 sprintf(stra, "<p>L&acircmpada em %d",Duty_ac);
 putsUSART(stra);
 putsUSART(paginaHTML3);
 putcUSART(0X0D); // \r (enter)
 putcUSART(0X0A); // \n (nova linha)
 contador=0; 
 limpa_rx(); // sen�o limpar fica mandando a pagina varias vezes
}

int main() // ok sem ports
{
 int d=0;
 TRISA=0xff;
 TRISAbits.TRISA0 = 1; //entrada analogica
 TRISAbits.TRISA3 = 0; //led vermelha � saida
 TRISAbits.TRISA5 = 0; //led verde � saida
 TRISCbits.TRISC1 = 0; // lampada DC � saida
 PORTAbits.RA5=0; // INICIA CONFIGURA��O EM nada (estar� OK em VERDE)
 PORTCbits.RC1=0; //INICIA LAMPADA APAGADA
 PORTAbits.RA3=0; //led vermelho apagado
 inicioADC(); //Fun��es de inicializa��o
 inicioUSART();
 inicioInterrupcoes();
 inicioTIMER();
 inicioPWM();
 inicioInterp();
 inicioLCD();
 CCP2CONbits.CCP2M3 = 0; // desabilita PWM Mode
 CCP2CONbits.CCP2M2 = 0;
 CCP2CONbits.CCP2M1 = 0;
 CCP2CONbits.CCP2M0 = 0;
 
 ADRES = 0; //Inicializa variaveis em 0
 Duty_ac=0;
 Valor_convAD11=0;
 Duty=0;
 
 for(;;) //inicio do loop infinito
 {
 //tenta configurar o modulo por no m�ximo 25 vezes.Caso n�o seja possivel avisa o usu�rio do erro atrav�s do display
 while (modconf != FIM)
 {
 conf_ESP8266();
 d++;
 if (d>25)
 {
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor
 comando (0b10000000); //0x80 onde come�a os caracteres da primeira linha
 texto(&Erro,sizeof(Erro));
 Delay1Second(1);
 d=0;
 }
 }
 Valor_convAD11=Valor_convAD; //recebe o valor da convers�o AD
 PORTAbits.RA5=1; //ACIONA VERDE QUANDO TERMINAR DE CONFIGURAR
 
 confCONNECT = buscaCONNECT(Rxdata); //Busca para saber qual dado foi recebino pela USART
 confCLOSED = buscaCLOSED(Rxdata);
 confIPD=buscaIPD(Rxdata);
 confGET = buscaGET(Rxdata);
 confGEThttp = buscaGEThttp(Rxdata);
 confGETBotao = buscaBotao(Rxdata);
 confB2=buscaB2(Rxdata);
 confB1=buscaB1(Rxdata);
 confB0=buscaB0(Rxdata);
 Dut=buscaDuty(Rxdata);
 buscaDutychar(Rxdata); // salva o valor de duty no buffer
 if (Dut != 101)//se a busca por duty retornar um valor, salva esse valor em Desejado
 {
 Desejado=Dut;
 }
 if (buscaCONNECT(Rxdata) == 1) //Se encontrar CONNECT, avisa pelo display ao usu�rio
 {
 comando (0b10000000); //0x80 onde come�a os caracteres da primeira linha
 texto(&Connect,sizeof(Connect)); //exibe mensagem de que foi encontrado pedido de conexao
 Delay1Second(2);
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 }
 if ((buscaCLOSED(Rxdata) == 1)&& (buscaCONNECT(Rxdata) == 0)) //Se encontrar CONNECT e CLOSED, avisa pelo display ao usu�rio
 {
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 comando (0b10000000); //0x80 onde come�a os caracteres da primeira linha
 texto(&Tnova,sizeof(Tnova)); //Exibe mensagem de erro
 }
 if(confIPD == 1) //Se encontrar IPD significa que algum dado foi mandado, deve decobrir qual
 {
 if(confGET == 1) //se encontrar GET deve continuar e descobrir o que o m�dulo esta pedindo
 {
 if (confGEThttp == 1) //se achar HTTP, esta pedindo pela pagina
 {
 comando (0b00000001);
 paginahtml(); //envia a pagina e aguarda
 comando (0b10000000); //0x80 onde come�a os caracteres da primeira linha
 texto(&Pag,sizeof(Pag)); //envia mensagem de que a p�gina foi enviada
 Delay1Second(2);
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 comando (0b11000000); //0xC0 onde come�a os caracteres da segunda linha
 texto(&Aguard,sizeof(Aguard)); //exibe 'Aguardando'
 }
 if (confGETBotao == 1) //se achar botao, esta pedindo o botao, deve verificar qual botao
 {
 if ((confB2 == 1)&&(Dut != 101)) //significa que esta pedindo uma intensidade v�lida
 {
 Desejado=Dut; //salva o valor de porcentagem desejada
 PorceDesejada=Desejado*100; //porcentagem neste c�digo s�o multiplicadas por 100
 IntToChar(PorceDesejada); //obtem o char da porcentagem desejada
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 comando (0b10000000); //0x80 onde come�a os caracteres da primeira linha
 texto(&Lamp1,sizeof(Lamp1)); //;exibe 'Lampada em: ' seguido do valor em decimal
 enviadados(buffera[1]);
 enviadados(buffera[2]);
 querolux =LUX_A*10; //o calculo de quantos lux correspondem a � porcentagem desejada � feito por partes
 querolux=querolux*Desejado;
 querolux=querolux/10;
 Duty_ac=DutyAcertado(querolux); //obtem o duty cycle correspondente � limunisodade desejada
 IntToChar(Duty_ac); //salva o duty obtido em buffera
 CCP2CONbits.CCP2M3 = 1; // Ativa modo PWM
 CCP2CONbits.CCP2M2 = 1;
 CCP2CONbits.CCP2M1 = 1;
 CCP2CONbits.CCP2M0 = 1;
 SetDCPWM2(Duty_ac); //envia o pwm para a lampada
 comando (0b11000000); //Endere�o da segunda linha
 texto(&Lamp0,sizeof(Lamp0)); //exibe 'Duty em: ' seguido do valor do duty
 enviadados(buffera[1]);
 enviadados(buffera[2]);
 enviadados(buffera[3]);
 enviadados(buffera[4]);
 //Prepara��o para o controle
 //defini��o das variav�is usadas
 unsigned long lux___1;
 int a=0;
 int digital;
 long saidaI1, saidaI2, saidaP1,saidaP2;
 long saida=0 ;
 unsigned int atual, erro,eabs, erro_anterior=501, eabs=501;
 int a=0;
 long B1=-99180; //valor calculado a partir dos ganho de kp e ki *10000
 int B0=100; //valor calculado a partir dos ganho de kp e ki *100
 while (eabs>500)
 {
 digital=Valor_convAD; //Salva o valor da convers�o AD
 ValorPORCatual=trataAD(digital);//obtem a tens�o associada ao digital
 lux___1=VoltAcertado(ValorPORCatual); //obtem a luminosidade associada � tens�o
 atual= PorcentagemLux(lux___1);// obtem a porcentagem atual
 erro=PorceDesejada-atual; //calcula o erro como a porcentagem desejada menos o estado atual
 if (erro>=0) //verifia se o m�dulo do erro � positivo ou negativo
 {
 eabs=erro;
 }
 if (erro<0) 
 {
 eabs=-erro;
 }
 //saida=saida+B0erro+B1erro -> � feito por partes
 saidaI1=B1*erro_anterior;
 saidaI2=saidaI1/100000; //B1 � multiplicado por 10mil
 saidaP1=B0;
 saidaP1=saidaP1*erro;
 saidaP2=saidaP1/10; //B0 � multiplicado por 100
 saidaP2=saidaP2/10;
 saida=saida+saidaP2+saidaI2; //a sa�da � o valor de porcentagem atual
 erro_anterior=erro; //erro_anterior recebe o erro atual
 if((saida>10000)||(saida<0))
 {
 saida=500; //se a sa�da foi superior a 10000 (superior a duty=1000*10) para n�o chegar no m�ximo de 1023*10, reduz pra porcentagem=50*10
 }
 a=a+1; //LOOP colocado para aquisi��o dos 30 primeiros pontos
 if (a>30)
 {
 erro_anterior=499;
 }
 Desejado=saida/100; //calcula a porcentagem da s�ida
 querolux =LUX_A*10; //o calculo de quantos lux correspondem a � porcentagem da sa�da � feito por partes
 querolux=querolux*Desejado;
 querolux=querolux/10;
 Duty_ac=DutyAcertado(querolux); //obtem o duty cycle correspondente � liminosidade na sa�da
 IntToChar(Duty_ac); //salva o duty obtido em buffera
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 comando (0b10000000); //seleciona a primeira posi��o da primeira linha
 enviadados(buffera[1]); //envia o valor do duty a ser utilizado
 enviadados(buffera[2]);
 enviadados(buffera[3]);
 enviadados(buffera[4]);
 CCP2CONbits.CCP2M3 = 1; // Ativa modo PWM
 CCP2CONbits.CCP2M2 = 1;
 CCP2CONbits.CCP2M1 = 1;
 CCP2CONbits.CCP2M0 = 1;
 SetDCPWM2(Duty_ac); //envia o pwm para a lampada
 }
 Delay1Second(1); //aguarda
 paginahtml(); //envia a pagina ao final do controle
 }
 if ((confB1 == 1)||((confB2 == 1)&&(Dut == 100))) //ligar a l�mpada em seu m�ximo pode ser devido a:
 //botao ligar ou controle em 100
 {
 PORTCbits.RC1 = 1; //acende a l�mpada DC
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 comando (0b10000000); //0x80 seleciona onde come�a os caracteres da primeira linha
 texto(&Lamp,sizeof(Lamp)); //exibe 'Lampada em '
 enviadados(0b00110001); //seguido de 100%
 enviadados(0b00110000);
 enviadados(0b00110000);
 enviadados(0b00100101);
 Delay1Second(1); //aguarda
 paginahtml(); //envia a pagina e aguarda
 }
 if ((confB0 == 1) ||((confB2 == 1)&&(Dut == 0)))//Desligar a l�mpada pode ser devido a:
 //botao desligar ou controle em 0
 {
 CCP2CONbits.CCP2M3 = 0; // Desativa PWM Mode
 CCP2CONbits.CCP2M2 = 0;
 CCP2CONbits.CCP2M1 = 0;
 CCP2CONbits.CCP2M0 = 0;
 PORTCbits.RC1 = 0; // apaga a l�mpada
 comando (0b00000001); // 0x01 Limpeza do Display com retorno do cursor a primeira linha
 comando (0b10000000); //0x80 seleciona onde come�a os caracteres da primeira linha
 texto(&Lamp,sizeof(Lamp)); //exibe 'Lampada em '
 enviadados(0b00110000);// seguido de 0%
 enviadados(0b00100101);
 Delay1Second(1); //aguarda
 paginahtml(); //envia a pagina e aguarda
 }
 }
 limpa_rx(); //limpa o registrador dos dados Rx
 contador=0; //limpa o contador para que o proximo dado recebido seja escrito na primeira posi��o de Rx
 Duty_ac=0; //limpa as vari�veis
 Duty=0; //limpa as vari�veis
 Dut=0; //limpa as vari�veis
 }
 }
 else
 {
 contador=0; //limpa o contador para que o proximo dado recebido seja escrito na primeira posi��o de Rx
 Duty_ac=0; //limpa as vari�veis
 Duty=0; //limpa as vari�veis
 Dut=0; //limpa as vari�veis
 }
 contador=0; //limpa o contador para que o proximo dado recebido seja escrito na primeira posi��o de Rx
 Duty_ac=0; //limpa as vari�veis
 Duty=0; //limpa as vari�veis
 Dut=0; //limpa as vari�veis
 } //FIM FOR INFINITO
}//FIM MAI