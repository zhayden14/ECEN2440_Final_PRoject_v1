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

void timing0(global * vars){
    int peak = 0, diff = 0;
    int sum_l, sum_r;
    int sensor_ref[6][2];

    // values below which sensor is over a line
    sensor_ref[0][0] = 6250;
    sensor_ref[1][0] = 6000;
    sensor_ref[2][0] = 6000;
    sensor_ref[3][0] = 5000;
    sensor_ref[4][0] = 5000;
    sensor_ref[5][0] = 5500;

    // values above which sensor is over a line
    sensor_ref[0][1] = 11000;
    sensor_ref[1][1] = 11000;
    sensor_ref[2][1] = 11000;
    sensor_ref[3][1] = 8500;
    sensor_ref[4][1] = 76000;
    sensor_ref[5][1] = 9500;

    vars->cycles++;
    //copy previous reflectance data
    char i;
    for(i = 0; i < 4; i++){
        vars->reflect[i] = TIMER_A1->CCR[i+1];
    }
    for(i = 4; i < 8; i++){
        vars->reflect[i] = TIMER_A2->CCR[i-3];
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
        //start motors
    }
    else{
        if(vars->ctlstate == LINE_FOLLOW){
            // determine which sensors are over a line
                    sum_l = 0;
                    sum_r = 0;
                    for(i=1; i<7; ++i)
                    {
                        if((vars->reflect[i]<sensor_ref[i][0]) || (vars->reflect[i]>sensor_ref[i][1])){
                            if(i<4) sum_l += 1;
                            else sum_r += 1;
                        }
                    }

                    // turn
                    if(sum_l>sum_r) {
                        powerDiff(50, 400);
                    } else if (sum_r>sum_l) {
                        powerDiff(400, 50);
                    } else {
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
     TIMER_A3->CCR[0] =  10000;    // Period of about 100 Hz
     TIMER_A3->EX0 =     0x0002;    // Divide by 3
     TIMER_A3->CCTL[1] = 0x0050;    // CCR1 toggle/reset
     TIMER_A3->CCR[1] =  20;         //after 20 counts, GPIO->input
     //TIMER_A3->CCTL[2] = 0x0040;
     //TIMER_A3->CCR[2] =  500;
     TIMER_A3->CTL =     0x0210;    // up mode, divide by 1

     //enable interrupts
     NVIC_EnableIRQ(TA3_0_IRQn);
     NVIC_EnableIRQ(TA3_N_IRQn);
}
