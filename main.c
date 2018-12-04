#include "msp.h"
#include "global.h"
#include "timing.h"

//global variables
global var;

//timing interrupts
//timer3 reset
void TA3_0_IRQHandler(void){
    //reset interrupt flag
    TIMER_A3->CCTL[0] &= ~BIT0;

    //perform actions
    timing0(&var);
}

//timer3 Compare
void TA3_N_IRQHandler(void){
    //reset interrupt flag + copy the TA3IV register
    short iv = TIMER_A3->IV;

    //decide what timing actions to take
    switch(iv){
        case 0x02:
            //these functions are defined in timing.c and imported from timing.h
            //to change behavior, you should modify these instead of the ISRs
            timing1(&var);
            break;
        case 0x04:
            //timing2();
            break;
        case 0x06:
            //timing3();
            break;
        case 0x08:
            //timing4();
            break;
        }
}

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	//index for loop
	int i;

	//initialize global variables
	var.ctlstate = LINE_FOLLOW;
	var.cycles = 0;
	for(i = 0; i < 8; i++){
	    var.calib[i] = 0;
	    var.reflect[i] = 0;
	}

<<<<<<< HEAD
	timingSetup();
=======
    //configure Timer A0 (modify!)
    TIMER_A0->CCTL[0] = 0x0080;    // CCI0 toggle
    TIMER_A0->CCR[0] =  255;    // Period or about 10 kHz
    //TIMER_A0->EX0 =     0x0005;    // Divide by 3
    TIMER_A0->CCTL[1] = 0x00E0;    // CCR1 toggle/reset
    TIMER_A0->CCTL[2] = 0x00E0;
    TIMER_A0->CCTL[3] = 0x00E0;
    TIMER_A0->CCTL[4] = 0x00E0;
    TIMER_A0->CCR[1] =  0;    // CCR1 duty cycle is 50%
    TIMER_A0->CCR[2] =  0;
    TIMER_A0->CCR[3] =  128;
    TIMER_A0->CCR[4] =  128;
    TIMER_A0->CTL =     0x0250;    // up mode, divide by 1

    //configure Timer A1 (capture 0-3)
    TIMER_A1->CCTL[0] = 0x0080;        //compare mode
    TIMER_A1->CCR[0] = 0xFFFF;    //maximum count range
    TIMER_A1->CCTL[1] = 0x8900;//capture mode (falling edge), CCI1A, SCS=1,
    TIMER_A1->CCTL[2] = 0x8900;//outmode: out bit, no interrupts
    TIMER_A1->CCTL[3] = 0x8900;
    TIMER_A1->CCTL[4] = 0x8900;
    TIMER_A1->CTL = 0x0210;//SMCLK/1(3MHz), up mode, no interrupts

    //configure Timer A2 (capture 4-7)
    TIMER_A2->CCTL[0] = 0x0080;
    TIMER_A2->CCR[0] = 0xFFFF;
    TIMER_A2->CCTL[1] = 0x8900;//capture mode (falling edge), CCI1A, SCS=1,
    TIMER_A2->CCTL[2] = 0x8900;//outmode: out bit, no interrupts
    TIMER_A2->CCTL[3] = 0x8900;
    TIMER_A2->CCTL[4] = 0x8900;
    TIMER_A2->CTL = 0x0210;

    //configure Timer A3 (general-purpose timing) [change to continuous mode]
    TIMER_A3->CCTL[0] = 0x0090;    // CCI0 toggle
    TIMER_A3->CCR[0] =  10000;    // Period of about 100 Hz
    TIMER_A3->EX0 =     0x0002;    // Divide by 3
    TIMER_A3->CCTL[1] = 0x0050;    // CCR1 toggle/reset
    TIMER_A3->CCR[1] =  20;         //after 20 counts, GPIO->input
    //TIMER_A3->CCTL[2] = 0x0040;
    //TIMER_A3->CCR[2] =  500;
    TIMER_A3->CTL =     0x0210;    // up mode, divide by 1

>>>>>>> parent of 6b4591d... integration prep
}
