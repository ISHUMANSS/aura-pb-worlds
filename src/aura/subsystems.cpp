#include "aura/subsystems.hpp"
#include "EZ-Template/util.hpp"
#include "pros/misc.h"
#include "pros/motors.hpp"

#include "EZ-Template/api.hpp"




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
            intake_1.move_voltage(floor(voltage));
            intake_2.move_voltage(floor(voltage));

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
            
            
            if (master.get_digital(pros::E_CONTROLLER_DIGITAL_B)){
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
                
                case INTAKE_INDEX:{
                    voltage = 8000;

                    

                    break;
                }

                case OUTTAKE_LOW:{
                    
                    bool lowIsFast = lowFast || speedOverride;
                    voltage = lowIsFast ?  -2500 : -6000;

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

    lever::lever(int lever_1_port,
             int lever_2_port,
             char lever_angle_port)
        : lever_1(pros::Motor(lever_1_port,
                            pros::v5::MotorGearset::red,
                            pros::v5::MotorEncoderUnits::degrees)),
        lever_2(pros::Motor(lever_2_port,
                            pros::v5::MotorGearset::red,
                            pros::v5::MotorEncoderUnits::degrees)),
        lever_angle(pros::adi::Pneumatics(lever_angle_port, false)),
        // kP, kI, kD, start_i
        lever_pid(45.0, 0.0, 120.0, 0.0, "Lever PID")
    {
        
        // Define what it means to be "settled" for this PID
        // (small_error, small_error_time, large_error, large_error_time, max_time)
        lever_pid.exit_condition_set(5, 50, 15, 150, 2000); 
    
    }

            void lever::setLeverState(double voltage, bool angle_state){
                lever_1.move_velocity(floor(voltage));
                lever_2.move_velocity(floor(voltage));
                lever_angle.set_value(angle_state);
            }

            void lever::goUpFast()   { currentMode = UP_FAST;   lever_pid.target_set(POS_UP);   lever_angle.set_value(true); }
            void lever::goUpSlow()   { currentMode = UP_SLOW;   lever_pid.target_set(POS_UP);   lever_angle.set_value(true); }
            void lever::goDownFast() { currentMode = DOWN_FAST; lever_pid.target_set(POS_DOWN); lever_angle.set_value(false); }
            void lever::goDownSlow() { currentMode = DOWN_SLOW; lever_pid.target_set(POS_DOWN); lever_angle.set_value(false); }
            
            
            void lever::stop() {
                currentMode = LEVER_IDLE;
                lever_1.move_voltage(0);
                lever_2.move_voltage(0);
            }

            bool lever::isSettled() {
                return lever_pid.exit_condition({lever_1, lever_2});
            }


            void lever::update() {
                if (currentMode == LEVER_IDLE) return;

                //PID computes output from current motor position
                //also change here if gonna use rotaion sensor or if 2 motor isn't working
                double output = lever_pid.compute(lever_1.get_position());

                //average both of the motors to get the current position
                // double output = lever_pid.compute((lever_1.get_position() + lever_2.get_position()) / 2.0);


                //select the max speed
                double cap = (currentMode == UP_FAST || currentMode == DOWN_FAST)
                            ? VOLT_FAST
                            : VOLT_SLOW;

                output = std::clamp(output, -cap, cap);

                lever_1.move_voltage(static_cast<int>(output));
                lever_2.move_voltage(static_cast<int>(output));

                //idle once settled
                if (isSettled()) stop();
            }

            void lever::driverFunctions() {
                if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R1))
                    goUpFast();
                else if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R2))
                    goUpSlow();
                else if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1))
                    goDownFast();
                else if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2))
                    goDownSlow();
            }

            void lever::leverTare(){
                lever_1.tare_position();
                lever_2.tare_position();
            }


        
    
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