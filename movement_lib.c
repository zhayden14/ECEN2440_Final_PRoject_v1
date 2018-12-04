#include <stdlib.h>
#include "msp.h"
#include "movement_lib.h"

#define LEFT_MOTOR 1
#define RIGHT_MOTOR 2
#define FORWARDS 0
#define BACKWARDS 1

static size_t turns_rgt = 0;
static size_t turns_lft = 0;

void PORT4_IRQHandler()
{
    if(P4->IFG & BIT0) {
        ++turns_lft;
        P4->IFG &= ~BIT0;
    }
    if(P4->IFG & BIT1) {
        ++turns_rgt;
        P4->IFG &= ~BIT1;
    }
    P4->IFG = 0;
}

static inline void setDir(unsigned char, unsigned char);

void setupMovement()
{
    // set up motor PWM input
    const unsigned char mtr_mask = 0x30;
    P2->SEL0 |= mtr_mask;
    P2->SEL1 &= ~mtr_mask;
    P2->DS |= mtr_mask;
    P2->DIR |= mtr_mask;
    // set up timer A0
    TIMER_A0->CCTL[0] = 0x0080;
    TIMER_A0->CCR[0] =  1000;
    TIMER_A0->EX0 =     0x0000;
    TIMER_A0->CCTL[1] = 0x00E0;
    TIMER_A0->CCTL[2] = 0x00E0;
    TIMER_A0->CCR[1] =  0;
    TIMER_A0->CCR[2] =  0;
    TIMER_A0->CTL = 0x0250;
    // set up encoder input pins
    const unsigned char enc_mask = 0x63;
    P4->SEL0 &= ~enc_mask;
    P4->SEL1 &= ~enc_mask;
    P4->DIR &= ~enc_mask;
    P4->REN |= enc_mask;
    P4->OUT &= ~enc_mask;
    P4IV |= enc_mask;
    P4->IE |= enc_mask;
    // set up dir, sleep
    const unsigned char ctl_mask = 0x0F;
    P10->SEL0 &= ~ctl_mask;
    P10->SEL1 &= ~ctl_mask;
    P10->DIR |= ctl_mask;
    P10->OUT |= 0x0C;
    NVIC_EnableIRQ(PORT4_IRQn);
    setDir(LEFT_MOTOR,FORWARDS);
    setDir(RIGHT_MOTOR,FORWARDS);
}

static inline void setPWMPct(unsigned char motor, unsigned int percent)
{
    TIMER_A0->CCR[motor] = percent;
}

static inline void setDir(unsigned char motor, unsigned char direction)
{
    const unsigned char lft_mask = 0x01;
    const unsigned char rgt_mask = 0x02;
    unsigned char mask = 0;
    if(motor==LEFT_MOTOR) mask |= lft_mask;
    if(motor==RIGHT_MOTOR) mask |= rgt_mask;
    if(direction==FORWARDS) P10->OUT &= ~mask;
    else P10->OUT |= mask;
}

inline void revDir(unsigned char motor)
{
    if(motor==LEFT_MOTOR) P10->OUT ^= BIT0;
    else if(motor==RIGHT_MOTOR) P10->OUT ^= BIT1;
}

inline void powerDiff(unsigned int speed_l, unsigned int speed_r)
{
    setPWMPct(LEFT_MOTOR,speed_l);
    setPWMPct(RIGHT_MOTOR,speed_r);
}

inline int getTurnsL()
{
    return turns_lft;
}

inline int getTurnsR()
{
    return turns_rgt;
}

inline void clearTurns()
{
    turns_rgt = 0;
    turns_lft = 0;
}

inline void moveStraightDist(size_t ticks)
{
    setDir(LEFT_MOTOR,FORWARDS);
    setDir(RIGHT_MOTOR,FORWARDS);
    size_t dist_r = ticks; // correction since right wheel is crooked => smaller effective radius, not correctable with encoders.
    size_t dist_l = (998*ticks)/1000; // ...
    unsigned int speed_l_inc = 500; // modifiable speeds to allow speed matching between motors. *_min is minimum motor power, *_inc+*_min is max power.
    unsigned int speed_l_min = 150; // Wheels accelerate/decelerate to prevent skidding/bouncing.
    unsigned int speed_r_inc = 500; // ...
    unsigned int speed_r_min = 150; // ...
    const unsigned int slow_thresh = 500; // number of encoder ticks before stop at which deceleration begins
    const unsigned int fast_thresh = 200; // number encoder ticks after start at which acceleration finishes
    size_t i,j; // storage for timer between corrections
    unsigned int togo_r, togo_l; // storage for ticks left for right/left motor
    clearTurns();
    while(turns_rgt<dist_r || turns_lft<dist_l)
    {
        togo_r = (1000*(dist_r-turns_rgt))/dist_r; // scaled to base on proportion of travel left, since absolute values can vary
        togo_l = (1000*(dist_l-turns_lft))/dist_l; // ...
        if(togo_r>togo_l) {
            speed_r_inc += 1; // speed up right wheel, slow down left wheel if left has completed greater proportion of distance
            speed_r_min += 1; // Necessary since motor speed is highly variable and unpredictable, and if wheels travel same distance
            speed_l_inc -= 1; //   the final angle will be correct, but will be offset from start. Need same speed and distance traveled.
            speed_l_min -= 1; // ...
        }
        if(togo_l>togo_r) {
            speed_l_inc += 1; // speed up left wheel, slow down right wheel if right has completed greater proportion of distance
            speed_l_min += 1; // ...
            speed_r_inc -= 1; // ...
            speed_r_min -= 1; // ...
        }
        i = turns_lft, j = turns_rgt;
        if(turns_rgt>=dist_r) setPWMPct(RIGHT_MOTOR,0); // disable right if traveled far enough
        else if(turns_rgt<=fast_thresh) // accelerate right to max speed while below specified distance threshold
            setPWMPct(RIGHT_MOTOR,speed_r_min + (speed_r_inc*turns_rgt)/fast_thresh);
        else if(turns_rgt>=slow_thresh) // decelerate right to min speed while above specified distance threshold
            setPWMPct(RIGHT_MOTOR,speed_r_min + (speed_r_inc*(dist_r-turns_rgt))/slow_thresh);
        if(turns_lft>=dist_l) setPWMPct(LEFT_MOTOR,0); // disable left if traveled far enough
        else if(turns_lft<=fast_thresh) // accelerate left to max speed while below specified distance threshold
            setPWMPct(LEFT_MOTOR,speed_l_min + (speed_l_inc*turns_lft)/fast_thresh);
        else if(turns_lft>=slow_thresh) // decelerate left to min speed while above specified distance threshold
            setPWMPct(LEFT_MOTOR,speed_l_min + (speed_l_inc*(dist_l-turns_lft))/slow_thresh);
        while(turns_lft-i<30 && turns_rgt-j<30); // delay until robot has traveled ~30 encoder ticks to prevent overcorrection of speed difference
    }
    powerDiff(0,0);
}

inline void rotDeg(int deg)
{
    if(deg>0) {
        setDir(LEFT_MOTOR,FORWARDS);
        setDir(RIGHT_MOTOR,BACKWARDS);
    } else {
        setDir(LEFT_MOTOR,BACKWARDS);
        setDir(RIGHT_MOTOR,FORWARDS);
    }
    deg = abs(deg); // at this point deg no longer represents actual degrees to rotate, rather number of encoder ticks for each wheel
    ++deg; // correction as else robot tends not to travel far enough
    deg *= 2; // simple adjustment since 360 encoder ticks per rotation
    clearTurns();

    while(turns_rgt<deg || turns_lft<deg-4) { // correction since right wheel is crooked
        if(turns_rgt>deg) setPWMPct(RIGHT_MOTOR,0); // turn off right when traveled far enough
        else setPWMPct(RIGHT_MOTOR, 500 - (400*turns_rgt)/deg); // decelerate right as approaches correct distance
        if(turns_lft>deg) setPWMPct(LEFT_MOTOR,0); // turn off left when traveled far enough
        else setPWMPct(LEFT_MOTOR, 500 - (400*turns_lft)/deg); // decelerate left as approaches correct distance
    }
    setPWMPct(LEFT_MOTOR,0);
    setPWMPct(RIGHT_MOTOR,0);
}





