#pragma once

#include "pros/adi.hpp"
#include "pros/motors.hpp"



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

    class lever{
        pros::Motor lever_1;
        pros::Motor lever_2;

        public:
        lever(int lever_1_port, 
                int lever_2_port);

        void setIntakeState(double voltage);

        void scoreHightFast();
        void scoreHighSlow();

        void scoreMidFast();
        void scoreMidSlow();



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