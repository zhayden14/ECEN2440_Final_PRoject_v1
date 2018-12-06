/*
 * global.h
 * static variables for final project.  To be shared with all code via global.h
 *
 *  Created on: Nov 29, 2018
 *      Author: Zack
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

//macros that define control state names
#define LINE_FOLLOW 0
#define BUMP_AVOID 1
#define IR_AVOID 2
#define IR_FOLLOW 3

//defines which sensor algorithm has control
//0 -
//1 -
//2 -
//3 -
typedef struct{
char ctlstate;
//records the number of this cycle of timer A3
unsigned long cycles;
//calibration data for line following code
int calib[8];
//reflectance data from line following sensors
int reflect[8];
//IR sensor motor drive
unsigned int irDriveL, irDriveR;
} global;

#endif /* GLOBAL_H_ */
