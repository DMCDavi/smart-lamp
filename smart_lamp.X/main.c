#include<pic18f4550.h>
#include <stdio.h>

#define delay for (i = 0; i <= 1000; i++)
#define rs RC0
#define rw RC1
#define e RC2
#define LED RB0

float analog_reading = 0;

void Init_AD() {
  ADCON0 = 0x01; //select channel AN0, enable A/D module
  ADCON1 = 0x0E; //use VDD, VSS as reference and configure AN0 for analog
  ADCON2 = 0xA6; //result right justified, acquisition time = 8 TAD, FOSC/64
  ADRESH = 0; /* Flush ADC output Register */
  ADRESL = 0;
}

unsigned int adc();

void lcd_int();
void cmd(unsigned char a);
void dat(unsigned char b);
void show(unsigned char * s);

int i;

void main() {
  Init_AD();
  unsigned int val;
  TRISD = TRISC = 0; //Port B and Port C is Output (LCD)
  TRISB = 0; //Port D is output LED
  TRISA0 = 1; //RA0 is input (ADC)
  lcd_int();
  while (1) {

    cmd(0x80);
    ADCON0 |= ((1 << ADON) | (1 << GO)); /*Enable ADC and start conversion*/
    while (ADCON0bits.GO_nDONE == 1); /*wait for End of conversion i.e. Go/done'=0 conversion completed*/
    analog_reading = (ADRESH * 256) + (ADRESL); /*Combine 8-bit LSB and 2-bit MSB*/
    analog_reading = (analog_reading * 5)/(10000 + analog_reading);
    char mystr[10];
    sprintf(mystr, "%f", analog_reading);
    show(mystr);
    if (analog_reading > 0 && analog_reading < 10) {
      LATB = 0x80;
    } else if (analog_reading >= 10 && analog_reading < 20) {
      LATB = 0xC0;
    } else if (analog_reading >= 20 && analog_reading < 30) {
      LATB = 0xE0;
    } else if (analog_reading >= 30 && analog_reading < 40) {
      LATB = 0xF0;
    } else if (analog_reading >= 40 && analog_reading < 50) {
      LATB = 0xF8;
    } else if (analog_reading >= 50 && analog_reading < 60) {
      LATB = 0xFC;
    } else if (analog_reading >= 60 && analog_reading < 70) {
      LATB = 0xFE;
    } else if (analog_reading >= 70) {
      LATB = 0xFF;
    } else {
      LATB = 0x00;
    }
  }
}

void lcd_int() {
  cmd(0x38);
  cmd(0x0c);
  cmd(0x06);
  cmd(0x80);
}

void cmd(unsigned char a) {
  PORTD = a;
  rs = 0;
  rw = 0;
  e = 1;
  delay;
  e = 0;
}

void dat(unsigned char b) {
  PORTD = b;
  rs = 1;
  rw = 0;
  e = 1;
  delay;
  e = 0;
}

void show(unsigned char * s) {
  while ( * s) {
    dat( * s++);
  }
}

unsigned int adc() {
  unsigned int adcval;

  ADCON1 = 0xc0; //right justified
  ADCON0 = 0x85; //adc on, fosc/64
  while (GO_nDONE); //wait until conversion is finished
  adcval = ((ADRESH << 8) | (ADRESL)); //store the result
  adcval = (adcval / 3) - 1;

  return adcval;
}