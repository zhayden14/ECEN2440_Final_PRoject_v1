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

//bump switch interrupt handler
void PORT6_IRQHandler(void){
    //disable interrupt
    NVIC_DisableIRQ(PORT6_IRQn);
    var.ctlstate = BUMP_AVOID;
    //will clear flags and reenable interrupt later

    //back up
    unsigned long int w;
    powerDiff(0,0);
    setDir(LEFT_MOTOR, BACKWARDS);
    setDir(RIGHT_MOTOR, BACKWARDS);
    powerDiff(125, 125);
    for(w = 0; w < 262144; w++){}

    //reenable interrupts
    P6IFG = 0x00;
    NVIC_EnableIRQ(PORT6_IRQn);

    //move to parallel position
    rotDeg(-90);
    moveStraightDist(2880);
    rotDeg(90);

    //move forwards and to line
    moveStraightDist(2880);
    rotDeg(45);
    moveStraightDist(3600);
    var.ctlstate = LINE_FOLLOW;
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

	//configure port 6 (bump switch input)
	P6DIR &= 0xC0;    //All pins are inputs
	P6SEL0 &= 0xC0;   //all pins are configured for GPIO
	P6SEL1 &= 0xC0;   //
	P6REN |= 0x3F;    //enable pullup resistors
	P6OUT  |= 0x3F;
	P6IES |= 0x3F;   //interrupt on low to high transition
	P6IE  |= 0x3F;   //interrupts enabled on bump switches
	P6IFG &= 0xC0;

	setupMovement();
	timingSetup();

	while(1){
	    for(i = 0; i < 65536; i++){}
	}
}
