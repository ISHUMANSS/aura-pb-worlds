#pragma once

#include "pros/adi.hpp"
#include "pros/motors.hpp"

//allows the use of EZ pid
#include "EZ-Template/api.hpp"


/**
*   This is where all the systems controlling the robot live
*   Contains:
*       - Drive Train
*       - Intake
*       - Match load
*       - Descore
*       - Park
*/


namespace subsystems {
    
    //used to set the state of the intake
    enum IntakeMode {
        IDLE,
        INTAKE_INDEX,
        OUTTAKE_LOW,
        SCORE_TALL,
        SCORE_MID,
        UNJAM
    };
    
    class intake{
        //set up the motors
        pros::Motor intake_1;
        pros::Motor intake_2;

        

        //be able to ge between the modes when diffrent buttons pressed
        IntakeMode currentMode = IDLE;

        //allow the index to keep running
        bool indexingEnabled = false;
        
        //allow the low to be ran slowly
        bool lowFast = false;
        
        public:
        //constructor
        intake(
                int intake_1_port, 
                int intake_2_port);

        //sets the intake voltage and the states for all the pistons
        void setIntakeState(double voltage);

        
        void driverFunctions();

        //auton intake
        void autoIndex();

        void autoScoreLow();

        void stopAuto();


    };

    enum LeverSpeed {
        LEVER_IDLE, //go down automatically untill it reaches the hard stop
        LEVER_FAST, //spin as fast as possible to the top to push cubes fast
        LEVER_SLOW, //spin slower
        LEVER_MANUAL, //manual control allowing for stoping where ever it needs to
        LEVER_EMERGENCY //move down it manualy
    };

    enum LeverAngle{
        LEVER_DOWN,
        LEVER_UP
    };

    class lever{
        pros::Motor lever_1;
        pros::Motor lever_2;
        pros::adi::Pneumatics lever_angle;
        pros::adi::Pneumatics hood;

        //set the speed for the the lever arm
        //speed also depends on what angle the lever is currently at
        LeverSpeed currentMode = LEVER_IDLE;

        //toggle for changing the angle
        LeverAngle leverAngle = LEVER_DOWN;
        int angle_press_count = 0;


        ez::PID lever_pid;//EZ Template PID controller

        //hard stop detection for returning to idle
        bool homing = false;
        bool homed = false;
        int strain_counter = 0;

        static constexpr int HOMING_VOLTAGE = -2500;// voltage to drive toward hard stop
        static constexpr int STRAIN_THRESHOLD = 1000;// mA (TUNE THIS DEPENDING HOW HOW MUCH STRAIN IT CAN BE ON)
        static constexpr int STRAIN_CONFIRM_TICKS = 15;// how many loop ticks above threshold before stopping

        //PID control state
        bool usingPIDTarget = false;// true when PID should be driving motors
        int  pid_max_speed  = 100;

        //position targets (NEED TUNEING)
        //UP =lever is raised
        //DOWN = lever is lowered 
        static constexpr double TARGET_FAST_UP   = 20.0;
        static constexpr double TARGET_FAST_DOWN = 20.0;
        static constexpr double TARGET_SLOW_UP   = 23.0;
        static constexpr double TARGET_SLOW_DOWN = 23.0;

        static constexpr int SPEED_FAST = 90;//max PID output fast mode
        static constexpr int SPEED_SLOW = 60;//max PID output slow mode


        

        public:
        lever(  int lever_1_port, 
                int lever_2_port,
                char lever_angle_port,
                char hood_port);

        void setLeverState(double voltage, bool angle_state, bool hood_state);

        void setLeverTarget(double position, int max_speed); //set PID target
        
        double getLeverPosition();//read IME

        void driverFunctions();

        void leverTask();//runs in a task loop
    


        //reset the positions of the IMEs in the motors
        void leverTare();
        bool isUnderStrain();



    };

    class matchload{
        pros::adi::Pneumatics matchload_solanoid;

        int matchload_press_count = 0;

        public:
        //constructor
        matchload(char matchload_solanoid_port);

        //function to set output
        void setState(bool matchload_state);

        //function to run during driver control
        void driverFunctions();


        //activate match loader
        void autonFucntions(bool );
        
    };

    class descore
    {
        pros::adi::Pneumatics descore_solanoid;

        int pressCount = 0;

        public:
        //Constructor
        descore(char descore_solanoid_port);

        //Function to set output
        void setState(bool state);

        //Function to run during driver control
        void driverFunctions();
    };


    


    


}