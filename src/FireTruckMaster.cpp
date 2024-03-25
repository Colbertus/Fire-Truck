#include <master/firetruck.h>

// Setup function that only runs once when the Arduino is powered on - ctm
void setup()
{
    Wire.begin();       // join i2c bus
    Serial.begin(9600); // start serial for output
    // block while waiting for character over serial
    while (!Serial)
    {
    }

    // Uncomment for Debug message - ctm
    // Serial.println("Starting Fire Truck Master Test");

#if !defined(__MIPSEL__)
    while (!Serial)
        ; // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
    if (Usb.Init() == -1)
    {
        Serial.print(F("\r\nOSC did not start"));
        while (1)
            ; // Halt
    }

    Serial.print(F("\r\nGameController Bluetooth Library Started"));

    // Initialize the old state of the firetruck - ctm
    firetruck.oldState = fireTruckStates::still;

    // Initialize the old state of the left stick - ctm
    truckMotorEscControlStick.oldState = leftStickStates::leftStickNeutral;

    // Initialize the old state of the right stick - ctm
    truckSteeringServoControlStick.oldState = servoControlStickStates::rightStickNeutral;

    // set the angles for the motor and servo
    truckMovementAngles.motor = 90;
    truckMovementAngles.servo = 90;

    SetUpPWMModule();
}

/********************************************************************************************************************/

// Loop function that runs over and over again - ctm
void loop()
{

    Usb.Task();
    if (GameController.connected())
    {

        // If the X button has been hit, play the second sound - ctm
        if (GameController.getButtonClick(CROSS))
        {
            sendData(TruckControlData.SoundTwo, dummyData);
        }

        // If the triangle button has been hit, play the first sound - ctm
        if (GameController.getButtonClick(TRIANGLE))
        {
            sendData(TruckControlData.SoundOne, dummyData);
        }

        // If the circle button has been hit, stop the current sound from playing - ctm
        if (GameController.getButtonClick(CIRCLE))
        {
            sendData(TruckControlData.SoundStop, dummyData);
        }

        // If the square button has been hit, toggle the water pump - ctm
        if (GameController.getButtonClick(SQUARE))
        {
            sendData(TruckControlData.ToggleWaterPump, dummyData);
        }
        if (GameController.getButtonClick(R3))
        {
            truckControlTimes.servoEngaged = 0;
            truckMovementAngles.servo = 90;
            // Replace with PWM module code
        }

        // call these functions to get and set the current state of the control sticks
        truckControlTimes.current = micros();
        if (truckControlTimes.current + truckControlTimes.motorsEngaged > LONG_MAX + (unsigned long)1)
        {
            truckControlTimes.motorsEngaged = 0;
            truckControlTimes.current = micros();
        }
        setSteeringServoState();
        truckControlTimes.current = micros();

        if (truckControlTimes.current + truckControlTimes.servoEngaged > LONG_MAX + (unsigned long)1)
        {
            truckControlTimes.servoEngaged = 0;
            truckControlTimes.current = micros();
        }
        setMotorState();
    }
}

/********************************************************************************************************************/

// send data over I2C interface to slave
void sendData(char data, char secondMovementChar)
{
    Serial.print("Sending: ");
    Wire.beginTransmission(I2CAddress); // Transmit to device
    Serial.println(data);
    Wire.write(data);       // Send serial data
    Wire.endTransmission(); // Stop transmitting
}

/********************************************************************************************************************/

// This function updates the new state of the left stick and returns the new state depending on if the old state is the same or not - ctm
void setMotorState()
{
    // Set these bools equal to the macros defined at the beginning - ctm
    // check motor stick position and set flags aproproately
    // Might have to add debouncing later

    bool isUp = upConditional;

    bool isDown = downConditional;

    // If the left stick is up - ctm
    if (isUp && !isDown)
    {
        motorsMoving = true;
        if (truckControlTimes.current - truckControlTimes.motorsEngaged >= motorPeriod)
        {
            if (MotorForwardAngleCheck)
            {
                truckControlTimes.motorsEngaged = millis();
                Serial.println("Forward");
                Serial.println(truckMovementAngles.motor);
                // Replace with PWM module code
                truckMovementAngles.motor += motorAngleChange;
            }
        }
    }
    else if (!isUp && isDown)
    {
        motorsMoving = true;
        if (MotorBackwardAngleCheck)
        {
            truckControlTimes.motorsEngaged = millis();
            Serial.println("Forward");
            Serial.println(truckMovementAngles.motor);
            // Replace with PWM module code
            truckMovementAngles.motor -= motorAngleChange;
        }
    } // If the left stick is neutral - ctm
    else
    {
        if (motorsMoving && truckControlTimes.current - truckControlTimes.motorsEngaged >= motorPeriod)
        {
            if (truckMovementAngles.motor <= 90)
            {
                Serial.println(truckMovementAngles.motor);
                // Replace with PWM module code
                truckMovementAngles.motor += motorAngleChange;
                truckControlTimes.motorsEngaged = truckControlTimes.current;
            }
            else if (truckMovementAngles.motor >= 90)
            {
                Serial.print("angle = ");
                Serial.println(truckMovementAngles.motor);
                // Replace with PWM module code
                truckMovementAngles.motor -= motorAngleChange;
                truckControlTimes.motorsEngaged = truckControlTimes.current;
            }
        }
    }
}

/********************************************************************************************************************

This function updates the new state of the right stick and returns the new state depending on if the old state is the same or not - ctm

  Outline:

    - check and set the current states of the servoControl stick
      - check if the servo control stick is right or left
      - set the truckControlTimes.motorsEngaged to the current time
      - we might want to have additional bools for the state of the servo

********************************************************************************************************************/
void setSteeringServoState()
{

    // Set these bools equal to the macros defined at the beginning - ctm
    // delay(500);
    // delayMicroseconds(2000);
    bool isLeft = leftConditional;
    // delayMicroseconds(2000);
    // delay(500);
    bool isRight = rightConditional;

    // truckMovementAngles.servo = SteeringServo.read();
    // If the right stick is to the left - ctm
    if (isLeft && !isRight)
    {
        if (truckControlTimes.current - truckControlTimes.servoEngaged >= servoPeriod)
        {
            if (truckMovementAngles.servo >= 60)
            {
                truckControlTimes.servoEngaged = truckControlTimes.current;
                truckMovementAngles.servo -= steeringAngleChange;
                Serial.println("Left");
                Serial.println(truckMovementAngles.servo);
                // Replace with PWM module code
            }
        }
    }
    else if (!isLeft && isRight)
    {

        if (truckControlTimes.current - truckControlTimes.servoEngaged >= servoPeriod)
        {
            if (truckMovementAngles.servo <= 120)
            {
                truckControlTimes.servoEngaged = truckControlTimes.current;
                truckMovementAngles.servo += steeringAngleChange;
                Serial.println("Right");
                Serial.println(truckMovementAngles.servo);
                // Replace with PWM module code
            }
        }
    }
}

void SetUpPWMModule() {
     pwm.begin();
  /*
   * In theory the internal oscillator (clock) is 25MHz but it really isn't
   * that precise. You can 'calibrate' this by tweaking this number until
   * you get the PWM update frequency you're expecting!
   * The int.osc. for the PCA9685 chip is a range between about 23-27MHz and
   * is used for calculating things like writeMicroseconds()
   * Analog servos run at ~50 Hz updates, It is importaint to use an
   * oscilloscope in setting the int.osc frequency for the I2C PCA9685 chip.
   * 1) Attach the oscilloscope to one of the PWM signal pins and ground on
   *    the I2C PCA9685 chip you are setting the value for.
   * 2) Adjust setOscillatorFrequency() until the PWM update frequency is the
   *    expected value (50Hz for most ESCs)
   * Setting the value here is specific to each individual I2C PCA9685 chip and
   * affects the calculations for the PWM update frequency. 
   * Failure to correctly set the int.osc value will cause unexpected PWM results
   */
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  delay(10);
}