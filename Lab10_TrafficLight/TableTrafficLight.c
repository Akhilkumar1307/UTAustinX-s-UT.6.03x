// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
//#include "TExaSscope.h"
// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000032;     // 1) activate clock for Port F&E&B
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x0F;           // allow changes to PF3-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R &=~(0x0A);        // 3) disable analog on PF
	GPIO_PORTE_AMSEL_R &=~(0x07);        // 3) disable analog on PE
	GPIO_PORTB_AMSEL_R &=~(0x3F);        // 3) disable analog on PB
  GPIO_PORTF_PCTL_R =0X00;   // 4) PCTL GPIO on PF4-0
	 GPIO_PORTE_PCTL_R =0X00;   // 4) PCTL GPIO on PE
	 GPIO_PORTB_PCTL_R =0X00;   // 4) PCTL GPIO on PB
  GPIO_PORTF_DIR_R |=(0x0A);          // 5)  PF3-1 out
	 GPIO_PORTE_DIR_R &=~(0x07);
	 GPIO_PORTB_DIR_R |=(0x3F);
  GPIO_PORTF_AFSEL_R &=~(0x0A);        // 6) disable alt funct on PF7-0
	GPIO_PORTE_AFSEL_R &=~(0x07);        // 6) disable alt funct on PE7-0
	GPIO_PORTB_AFSEL_R &=~(0x3F);        // 6) disable alt funct on PB7-0
	 GPIO_PORTF_PUR_R &=~(0x0A);
	GPIO_PORTE_PUR_R &=~(0x07);
	GPIO_PORTB_PUR_R &=~(0x3F);
	 GPIO_PORTF_PDR_R &=~(0x0A);
	GPIO_PORTE_PDR_R &=~(0x07);
	GPIO_PORTB_PDR_R &=~(0x3F);
  GPIO_PORTF_DEN_R |=(0x0A);          // 7) enable digital I/O on PF3-0
	GPIO_PORTE_DEN_R |=(0x07);          // 7) enable digital I/O on PE3-0
	GPIO_PORTB_DEN_R |=(0x3F);          // 7) enable digital I/O on PB5-0
	GPIO_PORTF_DATA_R=0x00 ;
	GPIO_PORTB_DATA_R =0x08 ;
	
}
struct state{
unsigned char out;
unsigned char out1;
unsigned short wait;
unsigned char next[8];
};
typedef const struct state statetype;
statetype fsm[9]={
{0x4c,0x02,40,{0,0,1,1,1,1,0,1}},
{0x54,0x02,5,{2,2,2,2,4,2,2,2}},
{0x61,0x02,40,{2,3,2,3,3,2,3,3}},
{0x62,0x02,5,{0,0,0,0,4,0,0,4}},
{0xa4,0x08,40,{4,5,5,5,4,5,5,5}},
{0x64,0x02,5,{6,6,6,6,6,6,6,6}},
{0x24,0x00,5,{7,7,7,7,7,7,7,7}},
{0x64,0x02,5,{8,8,8,8,8,8,8,8}},
{0x24,0x00,5,{0,0,2,0,4,2,0,0}},
};
unsigned char cstate;
unsigned char input;
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x00FFFFFF;        // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it             
  NVIC_ST_CTRL_R = 0x00000005;          // enable SysTick with core clock
}
void systick_wait10ms(unsigned long delay){
NVIC_ST_RELOAD_R=(delay-1)*800000;
NVIC_ST_CURRENT_R=0;
while((NVIC_ST_CTRL_R&0X00010000)==0){
}	
}
int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
// TExaS_Scope();
  PortF_Init();   // initialize PF1 to output
  SysTick_Init(); // initialize SysTick, runs at 16 MHz
  EnableInterrupts();
	cstate=0;
  while(1){
     GPIO_PORTB_DATA_R=fsm[cstate].out;
		 GPIO_PORTF_DATA_R=fsm[cstate].out1;
		 systick_wait10ms(fsm[cstate].wait);
		input=(GPIO_PORTE_DATA_R&0x07);
		cstate=fsm[cstate].next[input];
  }
}
