// MeasurementOfDistance.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal distance, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.  The display is optional.
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// Slide pot pin 3 connected to +3.3V
// Slide pot pin 2 connected to PE2(Ain1) and PD3
// Slide pot pin 1 connected to ground


#include "ADC.h"
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "UART.h"
#include "TExaS.h"

void EnableInterrupts(void);  // Enable interrupts

//unsigned char String[10]; // null-terminated ASCII string
unsigned long Distance;   // units 0.001 cm
unsigned long ADCdata;    // 12-bit 0 to 4095 sample
unsigned long Flag;       // 1 means valid Distance, 0 means Distance is empty

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
  return ((2000*sample)/4095);  // 2cm is total distance
}

// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = period;    // reload value
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
}
// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 
ADCdata = ADC0_In();//call ADC module to get adc value
	Flag = 1;//acknowledgement
}



int main(void){ 
  volatile unsigned long delay;
  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
  UART_Init();
	Nokia5110_Init();
	SysTick_Init(1999999);//(80M/40-1)
	
  EnableInterrupts();
  while(1){ 
		if(Flag==1){
    
    //Nokia5110_SetCursor(0, 0);
    Distance = Convert(ADCdata);
    UART_ConvertDistance(Distance); // from Lab 11
    //Nokia5110_OutString(String);    // output to Nokia5110 LCD (optional)
			Flag = 0;
  }
}

}
