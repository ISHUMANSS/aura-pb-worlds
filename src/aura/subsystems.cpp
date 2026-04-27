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
        lever_pid(10.0, 0.5, 9.0, 0.0, "Lever PID")
    {    
    }

        void lever::setLeverState(double voltage, bool angle_state, bool hood_state){
            lever_1.move_voltage(floor(voltage));
            lever_2.move_voltage(floor(voltage));
            lever_angle.set_value(angle_state);
            hood.set_value(hood_state);
        }

        

        bool lever::isUnderStrain() {
            //returns milliamps
            int current_1 = lever_1.get_current_draw();
            int current_2 = lever_2.get_current_draw(); 
            return (current_1 + current_2)/2 > STRAIN_THRESHOLD;
        }


        double lever::getLeverPosition() {
            //average both motors position
            return (lever_1.get_position());
        }

        void lever::setLeverTarget(double position, int max_speed) {
            pid_max_speed  = max_speed;
            lever_pid.target_set(position);
            usingPIDTarget = true;
        }

        //used to stop the intake from spinning when the lever its moving to target
        //will give a small amout of time where the intake spins
        bool lever::isGoingUp() {
            return usingPIDTarget;
        }

        //stops the intake from spinning at all when the lever is going down
        bool lever::isGoingDown() {
            return homing;
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
            lever_angle.set_value(angle_state);


            //special put down
            if(currentMode != LEVER_IDLE){
                if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)){
                    currentMode = LEVER_IDLE;
                }
            }

            //speed toggles
            if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1)) {
                currentMode = (currentMode == LEVER_FAST) ? LEVER_IDLE : LEVER_FAST;
            }
            else if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)) {
                currentMode = (currentMode == LEVER_SLOW) ? LEVER_IDLE : LEVER_SLOW;
            }
            
            //hold buttons checked independently casue like it just didn't work
            //can override toggles release goes IDLE
            if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
                currentMode = LEVER_MANUAL;
            }
            else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT)) {
                currentMode = LEVER_EMERGENCY;
            }
            //neither hold button is pressed return to IDLE
            else if (currentMode == LEVER_MANUAL || currentMode == LEVER_EMERGENCY) {
                currentMode = LEVER_IDLE;
            }

            //open and close the hood
            bool hoodOpen = (
                currentMode == LEVER_FAST  ||
                currentMode == LEVER_SLOW   ||
                currentMode == LEVER_MANUAL
            );
            hood.set_value(hoodOpen);


            //set the state for the lever
            switch (currentMode) {

                case LEVER_FAST: {
                    homed = false;
                    double target = (leverAngle == LEVER_UP) ? TARGET_FAST_UP : TARGET_FAST_DOWN;
                    setLeverTarget(target, SPEED_FAST);
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
                    lever_1.move_voltage(3000);
                    lever_2.move_voltage(3000);
                    break;
                }

                case LEVER_EMERGENCY: {
                    usingPIDTarget = false;
                    lever_1.move_voltage(HOMING_VOLTAGE);
                    lever_2.move_voltage(HOMING_VOLTAGE);
                    break;
                }

                case LEVER_IDLE:
                default: {
                    usingPIDTarget = false;

                    if (homed) {
                        lever_1.move_voltage(0);
                        lever_2.move_voltage(0);
                    } else {
                        //drive toward hard stop
                        homing = true;
                        lever_1.move_voltage(HOMING_VOLTAGE);
                        lever_2.move_voltage(HOMING_VOLTAGE);

                        if (isUnderStrain()) {
                            strain_counter++;
                            if (strain_counter >= STRAIN_CONFIRM_TICKS) {
                                lever_1.move_voltage(0);
                                lever_2.move_voltage(0);
                                
                                homing = false;
                                homed  = true;
                                strain_counter = 0;
                                leverTare();
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

                // pros::lcd::print(6, "Lever pos: %.1f, ", getLeverPosition());
                // pros::lcd::print(7, "Lever 1 mA:  %d, 2 mA: %d",   lever_1.get_current_draw(), lever_2.get_current_draw());


                if (usingPIDTarget) {
                    double output = lever_pid.compute(getLeverPosition());
                    
                    output = ez::util::clamp(output, pid_max_speed);
                    lever_1.move(output);
                    lever_2.move(output); 
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


        ////auton
        void lever::autoAngleShift(bool angle){
            lever_angle.set_value(angle);
        
        }

        //helper function to wait until the lever PID reaches its target
        void lever::waitUntilSettled(double target_position, int timeout_ms) {
            int elapsed = 0;
            const int POLL_INTERVAL = 10;

            while (std::abs(target_position - getLeverPosition()) > 10.0) {
                if (elapsed >= timeout_ms) {
                    // Timed out — stop motors and bail
                    usingPIDTarget = false;
                    lever_1.move_voltage(0);
                    lever_2.move_voltage(0);
                    break;
                }
                elapsed += POLL_INTERVAL;
                pros::delay(POLL_INTERVAL);
            }

            pros::delay(150); // settle buffer
        }

        //homing loop for auto
        void lever::autoHome() {
            usingPIDTarget = false; // Turn off PID
            homing = true;
            homed = false;

            //drive motors down
            lever_1.move_voltage(HOMING_VOLTAGE);
            lever_2.move_voltage(HOMING_VOLTAGE);

            int auto_strain_counter = 0;
            
            //wait until it hits the hard stop
            while (true) {
                if (isUnderStrain()) {
                    auto_strain_counter++;
                    if (auto_strain_counter >= STRAIN_CONFIRM_TICKS) {
                        break; // Stop detected!
                    }
                } else {
                    auto_strain_counter = 0;
                }
                pros::delay(10);
            }

            //stop motors and reset position
            lever_1.move_voltage(0);
            lever_2.move_voltage(0);
            homing = false;
            homed = true;
            leverTare();
        }

        //move the lever to target position at set speed and then waint at the top
        void lever::autoScore(double target_position, int speed, int wait_time_ms, int timeout_ms) {
            //open hood
            hood.set_value(true);

            //set the PID target and spin speed
            setLeverTarget(target_position, speed);

            //wait for the lever to physically reach the top
            waitUntilSettled(target_position,timeout_ms);

            //wait the specified amount of time at the top
            pros::delay(wait_time_ms);

            //close the hood
            hood.set_value(false);

            //bring the lever back down safely to the hard stop
            autoHome();
        }
        

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    



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
        void intake::driverFunctions(lever& lev) {

            //lever going down stop intake
            if (lev.isGoingDown()) {
                setIntakeState(0);
                indexingEnabled = false;
                boost_start_time = 0;
                return;
            }

            //lever just started going up begin boost timer
            if (lev.isGoingUp()) {
                if (boost_start_time == 0) {
                    boost_start_time = pros::millis();
                }

                if (pros::millis() - boost_start_time < BOOST_DURATION_MS) {
                    //this also kinda picks up the next block which is not great
                    setIntakeState(12000); //boost!!!!!!!!!!!!!!!!!
                    return;
                } else {
                    setIntakeState(0); //no more boost :(
                    return;
                }
            }

            //lever not active reset boost timer and run normal driver logic
            boost_start_time = 0;

            //TOGGLES
            //start indexing
            if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)) {
                indexingEnabled = !indexingEnabled;
            }

            //speed overide to help the intake go faster and slower for scoreing low goals
            bool speedOverride = master.get_digital(pros::E_CONTROLLER_DIGITAL_UP);


            //----------------------------------------------------
            //DETERMINE CURRENT MODE
            //----------------------------------------------------
            
            
            if (master.get_digital(pros::E_CONTROLLER_DIGITAL_Y)){
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
                    voltage = 12000;

                    break;
                }
                case OUTTAKE_LOW:{
                    bool lowIsFast = lowFast || speedOverride;
                    voltage = lowIsFast ?  -9000 : -12000;

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
            -9000 //goes the other direction
           ); 
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
            matchload_press_count += master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN);

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
            descore_solanoid.set_value(state);
        }

        void descore::driverFunctions()
        {
            //hold L1 to extend
            //release to retract
            bool buttonHeld = master.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
            setState(buttonHeld);
        }
       

    
}