#include "main.h"
#include "aura/globals.h"
#include "aura/subsystems.hpp"


#include "aura/lvgl_selector.hpp"
#include "autons.hpp"
#include "pros/misc.h"


pros::Task* auton_task_handle = nullptr;
bool auton_running = false;

/////
// For installation, upgrading, documentations, and tutorials, check out our website!
// https://ez-robotics.github.io/EZ-Template/
/////

//set up lever
subsystems::lever lever = subsystems::lever(LEVER_1,LEVER_2, LEVER_ANGLE_SHIFTER, HOOD);



//set up intake
subsystems::intake intake = subsystems::intake(
                                               INTAKE_1,
                                               INTAKE_2
                                              );

//set up match load
subsystems::matchload matchload = subsystems::matchload(MATCHLOAD);


//set up lever
subsystems::descore descore = subsystems::descore(DESCORE);




// Chassis constructor
ez::Drive chassis(
    // These are your drive motors, the first motor is used for sensing!
    {LEFT_MOTOR_1, LEFT_MOTOR_2, LEFT_MOTOR_3,LEFT_MOTOR_4},     // Left Chassis Ports (negative port will reverse it!)
    {RIGHT_MOTOR_1, RIGHT_MOTOR_2, RIGHT_MOTOR_3,RIGHT_MOTOR_4},  // Right Chassis Ports (negative port will reverse it!)

    IMU1,      // IMU Port
    DRIVE_WHEEL_DIAMETER,  // Wheel Diameter (Remember, 4" wheels without screw holes are actually 4.125!)
    600);   // Wheel RPM = cartridge * (motor gear / wheel gear)


    //our ticks
    //600 * (motorGear / wheel gear)


// Uncomment the trackers you're using here!
// - `8` and `9` are smart ports (making these negative will reverse the sensor)
//  - you should get positive values on the encoders going FORWARD and RIGHT
// - `2.75` is the wheel diameter
// - `4.0` is the distance from the center of the wheel to the center of the robot
// ez::tracking_wheel horiz_tracker(8, 2.75, 4.0);  // This tracking wheel is perpendicular to the drive wheels
// ez::tracking_wheel vert_tracker(9, 2.75, 4.0);   // This tracking wheel is parallel to the drive wheels



//auto escape task
// Escape task just kills the auton movement and sets the flag — does NOT call opcontrol()
void auton_escape_task() {
  while (auton_running) {
    if (
        master.get_digital(DIGITAL_B) &&
        master.get_digital(DIGITAL_Y)
      ) {

      master.rumble("- -");
      auton_running = false;

      //stop all movement immediately
      chassis.drive_brake_set(MOTOR_BRAKE_COAST);
      chassis.pid_targets_reset();

      return;  //task exits, autonomous() returns, opcontrol() loop continues
    }
    pros::delay(20);
  }
}



/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
  // Print our branding over your terminal :D
  ez::ez_template_print();

  pros::delay(500);  // Stop the user from doing anything while legacy ports configure

  // Look at your horizontal tracking wheel and decide if it's in front of the midline of your robot or behind it
  //  - change `back` to `front` if the tracking wheel is in front of the midline
  //  - ignore this if you aren't using a horizontal tracker
  // chassis.odom_tracker_back_set(&horiz_tracker);
  // Look at your vertical tracking wheel and decide if it's to the left or right of the center of the robot
  //  - change `left` to `right` if the tracking wheel is to the right of the centerline
  //  - ignore this if you aren't using a vertical tracker
  // chassis.odom_tracker_left_set(&vert_tracker);

  // Configure your chassis controls
  // chassis.opcontrol_curve_buttons_toggle(true);   // Enables modifying the controller curve with buttons on the joysticks
  chassis.opcontrol_drive_activebrake_set(0.0);   // Sets the active brake kP. We recommend ~2.  0 will disable.
  chassis.opcontrol_curve_default_set(0.0, 0.0);  // Defaults for curve. If using tank, only the first parameter is used. (Comment this line out if you have an SD card!)

  // Set the drive to your own constants from autons.cpp!
  default_constants();

  
  // Autonomous Selector using LLEMU
  //ALSO COMMENT OUT THIS
  ////
  ///
  ///
  // ez::as::auton_selector.autons_add({
  //     {"Drive\n\nDrive forward and come back", drive_example},
  //     {"Turn\n\nTurn 3 times.", turn_example},
  //     {"Drive and Turn\n\nDrive forward, turn, come back", drive_and_turn},
  //     {"Drive and Turn\n\nSlow down during drive", wait_until_change_speed},
  //     {"Swing Turn\n\nSwing in an 'S' curve", swing_example},
  //     {"Motion Chaining\n\nDrive forward, turn, and come back, but blend everything together :D", motion_chaining},
  //     {"Combine all 3 movements", combining_movements},
  //     {"Interference\n\nAfter driving forward, robot performs differently if interfered or not", interfered_example},
  //     {"Simple Odom\n\nThis is the same as the drive example, but it uses odom instead!", odom_drive_example},
  //     {"Pure Pursuit\n\nGo to (0, 30) and pass through (6, 10) on the way.  Come back to (0, 0)", odom_pure_pursuit_example},
  //     {"Pure Pursuit Wait Until\n\nGo to (24, 24) but start running an intake once the robot passes (12, 24)", odom_pure_pursuit_wait_until_example},
  //     {"Boomerang\n\nGo to (0, 24, 45) then come back to (0, 0, 0)", odom_boomerang_example},
  //     {"Boomerang Pure Pursuit\n\nGo to (0, 24, 45) on the way to (24, 24) then come back to (0, 0, 0)", odom_boomerang_injected_pure_pursuit_example},
  //     {"Measure Offsets\n\nThis will turn the robot a bunch of times and calculate your offsets for your tracking wheels.", measure_offsets},
  //     {"run right side auton",rightSideAuton},
  // });

  LVGLTheme my_theme = {
    lv_color_hex(0x000000),   // background
    lv_color_hex(0x4a4949),   // panel
    lv_color_hex(0xFFFFFF),   // button
    lv_color_hex(0x800080),   // button_selected
    lv_color_hex(0x38663a),   // accent and path points
    lv_color_hex(0x238727),   // reversed path colour
    lv_color_hex(0x000000),   // text
    lv_color_hex(0x000000),   // text_muted
  };
  lvgl_selector_set_theme(my_theme);

  

  //my auton selector
  lvgl_selector_set_autons({
      {"right side auton", 
        "pick up blocks, match load, score",
        rightSideAuton,
        {   //waypoints: x/y in inches from field centre
            {-47, -5, false},
            { -47, -47, false},
            {-61,-47, false},
            { -28, -47,  true},
            {-61,-47, false},
            {-11,-11, true}
        }
      },
      {
        "Right Rush",
        "match load score wing",
        rightSideRush,
        {
          {-47,-6, true},
          {-47,-46, true},
          {-66,-46, true},
          {-28,-47, true},
          {-28,-62, true},
          {-7,-63, true}

        }
      },
      {
        "Right Mid",
        "Score mid first"
        ,
        rightMidScore,
        {
          {-47, -5, false},
          {-18, -15, false},
          {-62, -47, false},
          {-27, -47, false},
          {-62, -47, false},
          {-9, -41, false},
          {-12, -10, false},

        }
      }
   
  });


  // Initialize chassis and auton selector
  chassis.initialize();



  //CHANGE HERE TO BE ABLE TO USE THE PID SELECTOR
  //EZ DEFAULT  
  // ez::as::initialize();

  //CUSTOM ONE
  lvgl_selector_init();

  //reset the lever positions and start the task allowing for PID
  lever.leverTare();
  pros::Task lever_task([&]() { lever.leverTask(); });
  
  
  master.rumble(chassis.drive_imu_calibrated() ? "." : "---");
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
  // . . .
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
  // . . .
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
  auton_running = true;
  pros::Task escape_task(auton_escape_task);

  chassis.pid_targets_reset();                // Resets PID targets to 0
  chassis.drive_imu_reset();                  // Reset gyro position to 0
  chassis.drive_sensor_reset();               // Reset drive sensors to 0
  chassis.odom_xyt_set(0_in, 0_in, 0_deg);    // Set the current position, you can start at a specific position with this
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD);  // Set motors to hold.  This helps autonomous consistency

  /*
  Odometry and Pure Pursuit are not magic

  It is possible to get perfectly consistent results without tracking wheels,
  but it is also possible to have extremely inconsistent results without tracking wheels.
  When you don't use tracking wheels, you need to:
   - avoid wheel slip
   - avoid wheelies
   - avoid throwing momentum around (super harsh turns, like in the example below)
  You can do cool curved motions, but you have to give your robot the best chance
  to be consistent
  */

  //ALSO CHANGE HERE

  // ez::as::auton_selector.selected_auton_call();  // Calls selected auton from autonomous selector
  
  lvgl_selector_run_selected();
  

  auton_running = false;
}




/**
 * Simplifies printing tracker values to the brain screen
 */
void screen_print_tracker(ez::tracking_wheel *tracker, std::string name, int line) {
  std::string tracker_value = "", tracker_width = "";
  // Check if the tracker exists
  if (tracker != nullptr) {
    tracker_value = name + " tracker: " + util::to_string_with_precision(tracker->get());             // Make text for the tracker value
    tracker_width = "  width: " + util::to_string_with_precision(tracker->distance_to_center_get());  // Make text for the distance to center
  }
  ez::screen_print(tracker_value + tracker_width, line);  // Print final tracker text
}

/**
 * Ez screen task
 * Adding new pages here will let you view them during user control or autonomous
 * and will help you debug problems you're having
 */
void ez_screen_task() {
  while (true) {
    // Only run this when not connected to a competition switch
    if (!pros::competition::is_connected()) {
      // Blank page for odom debugging
      if (chassis.odom_enabled() && !chassis.pid_tuner_enabled()) {
        // If we're on the first blank page...
        if (ez::as::page_blank_is_on(0)) {
          // Display X, Y, and Theta
          ez::screen_print("x: " + util::to_string_with_precision(chassis.odom_x_get()) +
                               "\ny: " + util::to_string_with_precision(chassis.odom_y_get()) +
                               "\na: " + util::to_string_with_precision(chassis.odom_theta_get()),
                           1);  // Don't override the top Page line

          // Display all trackers that are being used
          screen_print_tracker(chassis.odom_tracker_left, "l", 4);
          screen_print_tracker(chassis.odom_tracker_right, "r", 5);
          screen_print_tracker(chassis.odom_tracker_back, "b", 6);
          screen_print_tracker(chassis.odom_tracker_front, "f", 7);
        }
      }
    }

    // Remove all blank pages when connected to a comp switch
    else {
      if (ez::as::page_blank_amount() > 0)
        ez::as::page_blank_remove_all();
    }

    pros::delay(ez::util::DELAY_TIME);
  }
}

//runs the printing in the background
pros::Task ezScreenTask(ez_screen_task);

/**
 * Gives you some extras to run in your opcontrol:
 * - run your autonomous routine in opcontrol by pressing DOWN and B
 *   - to prevent this from accidentally happening at a competition, this
 *     is only enabled when you're not connected to competition control.
 * - gives you a GUI to change your PID values live by pressing X
 */
void ez_template_extras() {
  // Only run this when not connected to a competition switch
  if (!pros::competition::is_connected()) {
    // PID Tuner
    // - after you find values that you're happy with, you'll have to set them in auton.cpp

    // Enable / Disable PID Tuner
    //  When enabled:
    //  * use A and Y to increment / decrement the constants
    //  * use the arrow keys to navigate the constants
    // if (master.get_digital_new_press(DIGITAL_X)&& master.get_digital(DIGITAL_UP))
    //   chassis.pid_tuner_toggle(); //works when not connected to comp switch

    // Trigger the selected autonomous routine
    if (master.get_digital(DIGITAL_B) && master.get_digital(DIGITAL_X)) {
      pros::motor_brake_mode_e_t preference = chassis.drive_brake_get();
      autonomous();
      chassis.drive_brake_set(preference);
    }

    // // Allow PID Tuner to iterate
    // chassis.pid_tuner_iterate();
  }
  // Disable PID Tuner when connected to a comp switch
  else {
    if (chassis.pid_tuner_enabled())
      chassis.pid_tuner_disable();
  }
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
  chassis.drive_brake_set(MOTOR_BRAKE_COAST);

  // autonomous();

  
  while (true) {
    // Gives you some extras to make EZ-Template ezier
    //allows for pid and for the auton runner while not plugged into the field
    ez_template_extras();

    
    
    chassis.opcontrol_arcade_standard(ez::SPLIT);   // Standard split arcade
    chassis.opcontrol_curve_buttons_toggle(false);  // Disable modifying curves through the controller
    chassis.opcontrol_curve_default_set(3, 6); //scaling curve so it like is drivable :3
  
    lever.driverFunctions();

    intake.driverFunctions(lever);
    // matchloader
    matchload.driverFunctions();

    descore.driverFunctions();



    pros::delay(ez::util::DELAY_TIME);  // This is used for timer calculations!  Keep this ez::util::DELAY_TIME
  }
}
