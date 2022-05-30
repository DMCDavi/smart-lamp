#include <xc.h>
#include <stdio.h>

#define MPX4115_ERROR 1.5
float analog_reading = 0;

float MXP4115_Read(float valor) {
  valor /= 204.8;
  valor += 0.475;
  valor /= 0.0459;
  return valor - MPX4115_ERROR;
}

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

  while (1) {

    ADCON0bits.GO = 1; //do A/D measurement
    while (ADCON0bits.GO = 1); /* Wait for End of conversion i.e. Go/done'=0 conversion completed */
    analog_reading = MXP4115_Read((ADRESH * 256) + (ADRESL)); /*Combine 8-bit LSB and 2-bit MSB*/

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