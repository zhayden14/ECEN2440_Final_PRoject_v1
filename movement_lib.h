/*
 * movement_lib.h
 *
 *  Created on: Nov 29, 2018
 *      Author: jgamm
 */

#ifndef MOVEMENT_LIB_H_
#define MOVEMENT_LIB_H_

void setupMovement();
 // Run function to set up pins associated with motion. Will not affect other pins.

inline int getTurnsR();
 // Return number of ticks by encoder on right motor. 360 per wheel rotation. Reset to 0 whenever clearTurns is called.

inline int getTurnsL();
 // Return number of ticks by encoder on left motor. 360 per wheel rotation. Reset to 0 whenever clearTurns is called.

inline void clearTurns();
 // Reset left/right turn counts to 0.

inline void powerDiff(unsigned int speedL, unsigned int speedR);
 // Set speeds of left/right motors independently. No encoder-based error correction.
    // speedL: speed of left motor. Units of .1% PWM cycle. Positive value.
    // speedR: speed of right motor. Units of .1% PWM cycle. Positive value.

inline void rotDeg(int degrees);
 // Rotate robot by specified number of degrees. Should stay approximately stationary.
    // degrees: Number of degrees by which to rotate robot. Positive number to rotate clockwise, negative to rotate counterclockwise.

inline void moveStraightDist(size_t ticks);
 // Robot will move straight by specified number of ticks; 360 ticks per rotation. Encoder-based error correction to move approximately straight.
    // ticks: Number of encoder ticks by which to move robot. Positive number.

inline void setLEDColor(unsigned char color);

#endif /* MOVEMENT_LIB_H_ */
