/*
 * timing.c
 *
 *  Created on: Nov 29, 2018
 *      Author: Zack
 */

#include "msp.h"
#include "global.h"
#include "movement_lib.h"
#include "timing.h"

#define SPEED_DIVIDE 2

void timing0(global * vars){
    int norm = 0;
    vars->peak = 0;
    int linedata[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int diff[7] = {0, 0, 0, 0, 0, 0, 0};
    vars->cycles++;
    //copy previous reflectance data
    char i;//, on = 0, mask = 1;
    for(i = 0; i < 4; i++){
        vars->reflect[i] = TIMER_A1->CCR[i+1];
    }
    for(i = 4; i < 8; i++){
        vars->reflect[i] = TIMER_A2->CCR[i-3];
    }
    /*for(i = 0; i < 4; i++){
        TIMER_A1->CCR[i] = 0;
        TIMER_A2->CCR[i] = 0;
    }*/

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
    if((vars->cycles > 8) && (vars->cycles < 17)){
        for(i = 0; i < 8; i++){
            vars->calib[i] += vars->reflect[i];
        }
    }
    else if(vars->cycles == 17){
        for(i = 0; i < 8; i++) vars->calib[i] /= 8;
    }

    if(vars->cycles == 25){
        vars->ctlstate = LINE_FOLLOW;
        //enable IR sensors and bump switches
        NVIC_EnableIRQ(PORT6_IRQn);
        //start motors
    }
    else{

        //motor speed control
        //center around calibration value + take absolute value
        /*for(i = 0; i < 8; i++){
            vars->reflect[i] -= vars->calib[i];
            if(vars->reflect[i] < 0) vars->reflect[i] *= -1;
            linedata[i] = vars->reflect[i];
        }*/
        /*//for controlling RGB LED (only if PMAP control set up)
        TIMER_A0->CCR[3] = reflect[5]/512;
        TIMER_A0->CCR[4] = reflect[2]/512;*/

        //find "expectation value"
        //normalize area to 1024
        /*for(i = 0; i < 8; i++){
            norm += linedata[i];
        }
        for(i = 0; i < 8; i++){
            linedata[i] *= 1024;
            linedata[i] /= norm;
            vars->peak += i * linedata[i];
        }*/
        //peak - 4096 should give posn. of center of the line on scale from -4096 to +4096

        //characterize "waveform" to find splits, lost lines, etc.


        for(i = 0; i < 7; i++){
            diff[i] = vars->reflect[i]-vars->reflect[i+1];
        }
        //vars->irDriveL += (vars->peak-4096)/512;
        //vars->irDriveR -= (vars->peak-4096)/512;

        if(vars->ctlstate == LINE_FOLLOW){
            /*if(vars->reflect[0] > 2048){
                powerDiff(63/SPEED_DIVIDE, 500/SPEED_DIVIDE);
            }
            else if(vars->reflect[7] > 2048){
                powerDiff(500/SPEED_DIVIDE, 63/SPEED_DIVIDE);
            }
            else if(vars->reflect[1] > 2048){
                powerDiff(125/SPEED_DIVIDE, 500/SPEED_DIVIDE);
            }
            else if(vars->reflect[6] > 2048){
                powerDiff(500/SPEED_DIVIDE, 125/SPEED_DIVIDE);
            }
            else if(vars->reflect[2] > 2048){
                powerDiff(250/SPEED_DIVIDE, 500/SPEED_DIVIDE);
            }
            else if(vars->reflect[5] > 2048){
                powerDiff(500/SPEED_DIVIDE, 250/SPEED_DIVIDE);
            }
            else{
                powerDiff(500/SPEED_DIVIDE, 500/SPEED_DIVIDE);
            }*/
            if(vars->reflect[0] > 2048){
                powerDiff(50, 400);
            }
            else if(vars->reflect[7] > 2048){
                powerDiff(400, 50);
            }
            else if(vars->reflect[6] > 2048){
                powerDiff(100, 400);
            }
            else if(vars->reflect[1] > 2048){
                powerDiff(400, 100);
            }
            else if(vars->reflect[2] > 2048){
                powerDiff(400, 200);
            }
            else if(vars->reflect[5] > 2048){
                powerDiff(200, 400);
            }
            else if(vars->reflect[4] > 2048){
                powerDiff(400, 300);
            }
            else if(vars->reflect[3] > 2048){
                powerDiff(300, 400);
            }
            else{
                powerDiff(400, 400);
            }
        }
    }
}

void timing1(global * vars){
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

void timing2(global * vars){}

void timing3(global * vars){}

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
     TIMER_A3->CCR[0] =  5000;    // Period of about 100 Hz
     TIMER_A3->EX0 =     0x0002;    // Divide by 3
     TIMER_A3->CCTL[1] = 0x0050;    // CCR1 toggle/reset
     TIMER_A3->CCR[1] =  20;         //after 20 counts, GPIO->input
     //TIMER_A3->CCTL[2] = 0x0040;
     //TIMER_A3->CCR[2] =  500;
     TIMER_A3->CTL =     0x0210;    // up mode, divide by 1

     //configure port 5 (capture input - TA2)
     P5DIR &= 0x3F;
     P5DS |= 0xC0;
     P5SEL0 |= 0xC0;
     P5SEL1 &= 0x3F;
     P5OUT |= 0xC0;

     //configure port 6 (capture input - TA2)
     P6DIR &= 0x3F;
     P6DS |= 0xC0;
     P6SEL0 |= 0xC0;
     P6SEL1 &= 0x3F;
     P6OUT |= 0xC0;

     //configure port 7 (capture input - TA1)
     P7DIR &= 0x0F;
     P7DS |= 0xF0;
     P7SEL0 |= 0xF0;
     P7SEL1 &= 0x0F;
     P7OUT |= 0xF0;

     //enable interrupts
     NVIC_EnableIRQ(TA3_0_IRQn);
     NVIC_EnableIRQ(TA3_N_IRQn);
}
