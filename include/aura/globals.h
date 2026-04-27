#pragma once

#include "subsystems.hpp"
//allows the use in the autons.cpp file
extern subsystems::lever lever;
extern subsystems::intake intake;
extern subsystems::descore descore;
extern subsystems::matchload matchload;

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
#define LEFT_MOTOR_1 -15
#define LEFT_MOTOR_2 -14
#define LEFT_MOTOR_3 -12
#define LEFT_MOTOR_4 -17

// right
#define RIGHT_MOTOR_1 2
#define RIGHT_MOTOR_2 5
#define RIGHT_MOTOR_3 6
#define RIGHT_MOTOR_4 4

// encoding ports (currently not used)
//  #define X_ENCODER_TOP 'A'
//  #define X_ENCODER_BOTTOM 'B'
//  #define Y_ENCODER_TOP 'C'
//  #define Y_ENCODER_BOTTOM 'D'

// imu ports
#define IMU1 20
//#define IMU2 

// Subsystems Motor ports

// set up so that they are all going up by default
// Lever
#define LEVER_1 18 //right
#define LEVER_2 -19 //left



// Intake
#define INTAKE_1 -3 //right
#define INTAKE_2 8 //left 


// #define EXPANDER_PORT 9

//Ports in the expander go from left to right A - H


// Prenumatics ports
// constexpr pros::adi::ext_adi_port_pair_t MATCHLOAD         = {EXPANDER_PORT, 2};
// constexpr pros::adi::ext_adi_port_pair_t DESCORE           = {EXPANDER_PORT, 4};
// constexpr pros::adi::ext_adi_port_pair_t HOOD              = {EXPANDER_PORT, 5};
// constexpr pros::adi::ext_adi_port_pair_t LEVER_ANGLE_SHIFTER = {EXPANDER_PORT, 1};
#define MATCHLOAD 'A'
#define DESCORE 'F'
#define HOOD 'H'
#define LEVER_ANGLE_SHIFTER 'E'