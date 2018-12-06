#include "msp.h"
#include "global.h"
#include "movement_lib.h"
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
	var.ctlstate = NONE;
	var.cycles = 0;
	var.irDriveL = 500;
	var.irDriveR = 500;
	for(i = 0; i < 8; i++){
	    var.calib[i] = 0;
	    var.reflect[i] = 0;
	}

	setupMovement();
	timingSetup();

	while(1){
	    for(i = 0; i < 65536; i++){}
	}
}
