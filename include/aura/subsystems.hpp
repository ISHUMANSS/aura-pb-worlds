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

    enum LeverMode {
        LEVER_IDLE,
        UP_FAST,
        UP_SLOW,
        DOWN_FAST,
        DOWN_SLOW
    };

    class lever{
        pros::Motor lever_1;
        pros::Motor lever_2;
        pros::adi::Pneumatics lever_angle;


        ez::PID lever_pid;//EZ Template PID controller
        LeverMode currentMode = LEVER_IDLE;


        //postions (in degrees)
        static constexpr double POS_UP   = 100.0;
        static constexpr double POS_DOWN =   0.0;

        //voltage
        static constexpr double VOLT_FAST = 12000.0;
        static constexpr double VOLT_SLOW =  4000.0;

        public:
        lever(int lever_1_port, 
                int lever_2_port,
            char lever_angle_port);

        void setLeverState(double voltage, bool angle_state);


        //keeps the lever in the correct positons and put the lever to idle when done
        void update();

        void driverFunctions();

        //mode setters 
        //also usable from auton
        void goUpFast();
        void goUpSlow();
        void goDownFast();
        void goDownSlow();
        void stop();


        bool isSettled();

        void leverTare();



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