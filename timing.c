/*
 * timing.c
 *
 *  Created on: Nov 29, 2018
 *      Author: Zack
 */

#include "msp.h"
#include "global.h"
#include "timing.h"

void timing0(void){
    int norm = 0, peak = 0;
    int linedata[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    cycles++;
    //copy previous reflectance data
    char i;
    for(i = 0; i < 4; i++){
        reflect[i] = TIMER_A1->CCR[i+1];
    }
    for(i = 4; i < 8; i++){
        reflect[i] = TIMER_A2->CCR[i-3];
    }

    //set ports 5, 6, 7 to GPIO
    P5SEL0 &= 0x3F;
    P6SEL0 &= 0x3F;
    P7SEL0 &= 0x0F;

    //set ports 5, 6, 7 to outputs
    //P7OUT = 0xFF;
    P5DIR |= 0xC0;
    P6DIR |= 0xC0;
    P7DIR |= 0xF0;

    //calibration
    if((cycles > 8) && (cycles < 17)){
        for(i = 0; i < 8; i++){
            calibration[i] += reflect[i];
        }
    }
    else if(cycles == 17){
        for(i = 0; i < 8; i++) calibration[i] /= 8;
    }
    else if((cycles > 16) && (cycles < 25)){
        for(i = 0; i < 8; i++){
            reflect[i] -= calibration[i];
            if(reflect[i] < 0) reflect[i] *= -1;
            if(reflect[i] > devmax[i]) devmax[i] = reflect[i];
        }
    }
    if(cycles == 25){
        //enable IR sensors and bump switches
        //start motors
    }
    else{

        //motor speed control
        //center around calibration value + take absolute value
        for(i = 0; i < 8; i++){
            reflect[i] -= calibration[i];
            if(reflect[i] < 0) reflect[i] *= -1;
            linedata[i] = refelct[i];
        }
        /*//for controlling RGB LED (only if PMAP control set up)
        TIMER_A0->CCR[3] = reflect[5]/512;
        TIMER_A0->CCR[4] = reflect[2]/512;*/

        //find "expectation value"
        //normalize area to 1024
        for(i = 0; i < 8; i++){
            norm += linedata[i];
        }
        for(i = 0; i < 8; i++){
            linedata[i] *= 1024;
            linedata[i] /= norm;
            peak += i * linedata[i]
        }
        //peak - 4096 should give posn. of center of the line on scale from -4096 to +4096

        //characterize "waveform" to find splits, lost lines, etc.


        if(ctlstate == LINE_FOLLOW){
            setLeftPWM(PWMleft);
            setRightPWM(PWMright);
        }
    }
}

void timing1(void){
    //set ports 5,6,7 to inputs
    P5DIR &= 0x3F;
    P6DIR &= 0x3F;
    P7DIR &= 0x0F;

    //set pin function to capture
    P5SEL0 |= 0xC0;
    P6SEL0 |= 0xC0;
    P7SEL0 |= 0xF0;

    //start Timers 1 and 2
    TIMER_A1->CTL |= BIT2;
    TIMER_A2->CTL |= BIT2;

}

void timing2(void){}

void timing3(void){}

void timing4(global * vars){}

void timingSetup(void){
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

}
