#include "aura/subsystems.hpp"
#include "EZ-Template/util.hpp"
#include "pros/misc.h"
#include "pros/motors.hpp"




/**
*   Contains all the non DriveTrain functions 
*   for any other systems on the robot
*
*   Contains:
*       - Intake 
*       - Lever
*       - Match Load
*       - Descore
*       - 
*
*/

namespace subsystems {
    //intake class
        //constructor
        intake::intake(
                    int intake_1_port, 
                    int intake_2_port
                )
            :   intake_1(pros::Motor(intake_1_port, pros::v5::MotorGearset::blue, pros::v5::MotorEncoderUnits::degrees)),
                intake_2(pros::Motor(intake_2_port, pros::v5::MotorGearset::blue, pros::v5::MotorEncoderUnits::degrees))
            {


            }
        //rest of the intakes functions

        void intake::setIntakeState(double voltage){
            //set the intakes motors to go the ways they need to
            intake_1.move_velocity(floor(voltage));
            intake_2.move_velocity(floor(voltage));

        }

        

        // intake driver functions:
        //allow for hood to open and close along with diffrent scoreing modes
        //allows for easier switching between modes of what needs to spin and what doesn't
        //intake is able to keep spinning on a toggle (also closes hood)
        //intake score up button opens the hood and spins so it scores up
        //intake mid reverses the mid roller while the rest still go up
        //intake bottom button outakes out the bottom and then also stops the toggle

        /**
            * All driver control functions for intake:
            *
            * Indexing
            * Speed override
            * Score low
            *
            * Controls:
            * A  -> Toggle indexing
            * UP  -> Speed override
            * IDK -> Score Low
        
        */
        void intake::driverFunctions() {
            //toggles
            //start indexing
            if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
                indexingEnabled = !indexingEnabled;
            }

            

            //make speed for mid goals changed when down is being held
            bool speedOverride = master.get_digital(pros::E_CONTROLLER_DIGITAL_UP);


            //----------------------------------------------------
            //DETERMINE CURRENT MODE
            //----------------------------------------------------
            
            
            if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
                currentMode = OUTTAKE_LOW;
            }
            else if (indexingEnabled)
                currentMode = INTAKE_INDEX;
            else
                currentMode = IDLE;
        

            double voltage = 0;

            //----------------------------------------------------
            //APPLY MODE LOGIC FROM BUTTON PRESS
            //----------------------------------------------------

            switch(currentMode)
            {
                
                case INTAKE_INDEX:{   // Toggle B
                    voltage = 8000;

                    

                    break;
                }

                case OUTTAKE_LOW:{   // L2
                    
                    bool lowIsFast = lowFast || speedOverride;
                    voltage = lowIsFast ?  -250 : -600;

                    indexingEnabled = false; 
                    break;
                }

               
                

                case IDLE:
                default:
                    //everything off
                    voltage = 0;
                    break;
            }


            setIntakeState(voltage);
        }

    //auto functions
    /**
        @brief keep the intake spinning and the hood closed
    */
    void intake::autoIndex(){
        setIntakeState(
            12000
        );            
    }

    /**
        @brief score out of the lower goal
        the lower intake needs to like spin slower?
    */
    void intake::autoScoreLow(){
        setIntakeState(
            -12000 //goes the other direction
           ); //lift up intake
    }

    /**
        @brief stop from spinning 
    */
    void intake::stopAuto(){
        setIntakeState(
            0
        );
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    




        
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    

    //matchload class
        //constructor
        matchload::matchload(char matchload_solanoid_port) 
        :   matchload_solanoid(pros::adi::Pneumatics (matchload_solanoid_port, false))
        {}

        void matchload::setState(bool matchloadState)
        {
            matchload_solanoid.set_value(matchloadState);
        }

        void matchload::driverFunctions()
        {
            matchload_press_count += master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A);

            //pressed odd amount of times
            if(matchload_press_count % 2 != 0)
            {
                setState(true);
            }
            else
            {
                setState(false);
            }  
        }

        // void matchload::autonFucntions(){


        // }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    

    //descorer class
    //Constructor
        descore::descore(char descore_solanoid_port) 
        :   descore_solanoid(pros::adi::Pneumatics (descore_solanoid_port, false))
        {}

        void descore::setState(bool state)
        {
            descore_solanoid.set_value(!state);
        }

        void descore::driverFunctions()
        {
            //hold L1 to extend
            //release to retract
            bool buttonHeld = master.get_digital(pros::E_CONTROLLER_DIGITAL_L1);
            setState(buttonHeld);
        }
       

    
}