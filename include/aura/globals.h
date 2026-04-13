#pragma once

/**
 * This is the File where all of the globals are set up
 * created to help simplify changing commonly used numbers and information
 * use negative to flip the port
 * Contains:
 *   - Drive Train
 *       - Wheel Sizes
 *       - Any sensor ports
 *       - Ports used
 *   - Any other subsystems Motor ports
 *   - Prenumatics ports
 */

// Drive Train
#define DRIVE_WHEEL_DIAMETER 2.75
#define DRIVE_GEAR_RATIO 0.75

// Track width
// distance between left and right wheel centers, meters (calibrate)
#define TRACKWIDTH 11.5

#define TRACKING_WHEEL_DIAMETER 3.25

#define HORIZONTAL_OFFSET 0.0
#define VERTICAL_OFFSET 0.0

// Motor ports
// left
#define LEFT_MOTOR_1 -6
#define LEFT_MOTOR_2 -7
#define LEFT_MOTOR_3 -5
#define LEFT_MOTOR_4 -4

// right
#define RIGHT_MOTOR_1 15
#define RIGHT_MOTOR_2 14
#define RIGHT_MOTOR_3 2
#define RIGHT_MOTOR_4 3

// encoding ports (currently not used)
//  #define X_ENCODER_TOP 'A'
//  #define X_ENCODER_BOTTOM 'B'
//  #define Y_ENCODER_TOP 'C'
//  #define Y_ENCODER_BOTTOM 'D'

// imu ports
#define IMU1 11
#define IMU2 12121

// Subsystems Motor ports

// set up so that they are all going up by default
// Intake
#define LEVER_1 -12
#define LEVER_2 13


// Lever
#define INTAKE_1 -1 // left
#define INTAKE_2 9  // right


// Prenumatics ports
// Matchload
#define MATCHLOAD 'A' 

// Intake
#define DESCORE 'C' 

// Hood 
#define HOOD 'B' 

//Lever angle shifter
#define LEVER_ANGLE_SHIFTER 'H'



