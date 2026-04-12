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

        void intake::setIntakeState(double voltage){
            //set the intakes motors to go the ways they need to
            intake_1.move_voltage(floor(voltage));
            intake_2.move_voltage(floor(voltage));
        }

        

        // intake driver functions:

        /**
            * All driver control functions for intake:
            *
            * Indexing
            * Speed override
            * Score low
            *
            * Controls:
        
        */
        void intake::driverFunctions() {
            //TOGGLES
            //start indexing
            if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
                indexingEnabled = !indexingEnabled;
            }

            //speed overide to help the intake go faster and slower for scoreing low goals
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
                char lever_angle_port,
                char hood_port
            )
        : lever_1(pros::Motor(lever_1_port,
                            pros::v5::MotorGearset::red,
                            pros::v5::MotorEncoderUnits::degrees)),
        lever_2(pros::Motor(lever_2_port,
                            pros::v5::MotorGearset::red,
                            pros::v5::MotorEncoderUnits::degrees)),
        lever_angle(pros::adi::Pneumatics(lever_angle_port, false)),
        hood(pros::adi::Pneumatics(hood_port, false)),
        // kP, kI, kD, start_i
        lever_pid(5.0, 0.0, 20.0, 0.0, "Lever PID")
    {    
    }

            void lever::setLeverState(double voltage, bool angle_state){
                lever_1.move_voltage(floor(voltage));
                lever_2.move_voltage(floor(voltage));
                lever_angle.set_value(angle_state);
            }

            

            bool lever::isUnderStrain() {
                // get_current_draw() returns milliamps
                int current_1 = lever_1.get_current_draw();
                // int current_2 = lever_2.get_current_draw(); // add when second motor in use
                return current_1 > STRAIN_THRESHOLD;
            }


            double lever::getLeverPosition() {
                // Average both motors once lever_2 is active
                // For now just use lever_1 since it's the active one
                return lever_1.get_position();
            }

            void lever::setLeverTarget(double position, int max_speed) {
                pid_max_speed  = max_speed;
                lever_pid.target_set(position);
                usingPIDTarget = true;
            }

            //one button to toggle if the lever is able to go up or down and switches between the 2
            //2 buttons controlling speed
            //the speed that the lever moves at depends on if the lever is up or down 
            //for example if the lever is down the fast speed is slower then when the lever is up and the fast button is clicked
            void lever::driverFunctions() {
                
                // angle toggle
                if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)) {
                    angle_press_count++;
                    leverAngle = (angle_press_count % 2 != 0) ? LEVER_UP : LEVER_DOWN;         
                }

                bool angle_state = (leverAngle == LEVER_UP);
                lever_angle.set_value(angle_state); // set pneumatic independently

                // speed toggles
                if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1)) {
                    currentMode = (currentMode == LEVER_FAST) ? LEVER_IDLE : LEVER_FAST;
                }
                else if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)) {
                    currentMode = (currentMode == LEVER_SLOW) ? LEVER_IDLE : LEVER_SLOW;
                }
                else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
                    currentMode = LEVER_MANUAL;
                }
                else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN)) {
                    currentMode = LEVER_EMERGENCY;
                }

                switch (currentMode) {

                    case LEVER_FAST: {
                        homed = false;
                        double target = (leverAngle == LEVER_UP) ? TARGET_FAST_UP : TARGET_FAST_DOWN;
                        setLeverTarget(target, SPEED_FAST);
                        // leverTask() drives the motor — nothing more needed here
                        break;
                    }

                    case LEVER_SLOW: {
                        homed = false;
                        double target = (leverAngle == LEVER_UP) ? TARGET_SLOW_UP : TARGET_SLOW_DOWN;
                        setLeverTarget(target, SPEED_SLOW);
                        break;
                    }

                    case LEVER_MANUAL: {
                        homed = false;
                        usingPIDTarget = false;
                        lever_1.move_voltage(2000);
                        // lever_2.move_voltage(2000);
                        break;
                    }

                    case LEVER_EMERGENCY: {
                        usingPIDTarget = false;
                        lever_1.move_voltage(HOMING_VOLTAGE);
                        // lever_2.move_voltage(HOMING_VOLTAGE);
                        break;
                    }

                    case LEVER_IDLE:
                    default: {
                        usingPIDTarget = false;

                        if (homed) {
                            lever_1.move_voltage(0);
                            // lever_2.move_voltage(0);
                        } else {
                            // drive toward hard stop
                            homing = true;
                            lever_1.move_voltage(HOMING_VOLTAGE);
                            // lever_2.move_voltage(HOMING_VOLTAGE);

                            if (isUnderStrain()) {
                                strain_counter++;
                                if (strain_counter >= STRAIN_CONFIRM_TICKS) {
                                    lever_1.move_voltage(0);
                                    leverTare();
                                    homing = false;
                                    homed  = true;
                                    strain_counter = 0;
                                }
                            } else {
                                strain_counter = 0;
                            }
                        }
                        break;
                    }
                }
            }

            void lever::leverTask() {
                while (true) {

                    pros::lcd::print(6, "Lever pos: %.1f", getLeverPosition());
                    pros::lcd::print(7, "Lever mA:  %d",   lever_1.get_current_draw());


                    if (usingPIDTarget) {
                        double output = lever_pid.compute(getLeverPosition());
                        
                        output = ez::util::clamp(output, pid_max_speed);
                        lever_1.move(output);
                        // lever_2.move(output); 
                    }
                    pros::delay(10);
                }
            }

            //reset the lever postion to 0
            //happens at the start of a match before auton and happebns each time the motor knows it at the bottom
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