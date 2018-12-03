/*
 * timing.c
 *
 *  Created on: Nov 29, 2018
 *      Author: Zack
 */

#include "msp.h"
#include "global.h"
#include "timing.h"

void timing0(global * vars){
    int norm = 0, peak = 0;
    int linedata[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int reflect[8] = {0, 0, 0, 0, 0, 0, 0, 0};
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
    else if((vars->cycles > 16) && (vars->cycles < 25)){
        for(i = 0; i < 8; i++){
            vars->reflect[i] -= vars->calib[i];
            if(vars->reflect[i] < 0) vars->reflect[i] *= -1;
            //if(reflect[i] > devmax[i]) devmax[i] = reflect[i];
        }
    }
    if(vars->cycles == 25){
        //enable IR sensors and bump switches
        //start motors
    }
    else{

        //motor speed control
        //center around calibration value + take absolute value
        for(i = 0; i < 8; i++){
            vars->reflect[i] -= vars->calib[i];
            if(reflect[i] < 0) vars->reflect[i] *= -1;
            linedata[i] = vars->reflect[i];
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
            peak += i * linedata[i];
        }
        //peak - 4096 should give posn. of center of the line on scale from -4096 to +4096

        //characterize "waveform" to find splits, lost lines, etc.


        if(vars->ctlstate == LINE_FOLLOW){
            //setLeftPWM(PWMleft);
            //setRightPWM(PWMright);
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
