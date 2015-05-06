// FabScan - http://hci.rwth-aachen.de/fabscan
//
//  Created by Francis Engelmann on 7/1/11.
//  Copyright 2011 Media Computing Group, RWTH Aachen University. All rights reserved.
//
//  Chngelog:
//  R. Bohne 29.01.2013: changed pin mapping to Watterott FabScan Arduino Shield
//  R. Bohne 30.12.2013: added pin definitions for stepper 4 --> this firmware supports the new FabScan Shield V1.1, minor syntax changes. Steppers are now disabled at startup.
//  R. Bohne 12.03.2014: renamed the pins 14..19 to A0..A5 (better abstraction for people who use Arduino MEGA, etc.)
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#define I2C_ADDR    0x27  // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

#define LIGHT_PIN A1
#define LASER_PIN A2
#define MS_PIN    5

//Stepper 1 as labeled on Shield, Turntable
#define ENABLE_PIN_0  2
#define STEP_PIN_0    3
#define DIR_PIN_0     4

#define TURN_LASER_OFF      200
#define TURN_LASER_ON       201
#define PERFORM_STEP        202
#define SET_DIRECTION_CW    203
#define SET_DIRECTION_CCW   204
#define TURN_STEPPER_ON     205
#define TURN_STEPPER_OFF    206
#define TURN_LIGHT_ON       207
#define TURN_LIGHT_OFF      208
#define ROTATE_LASER        209
#define FABSCAN_PING        210
#define FABSCAN_PONG        211
#define SELECT_STEPPER      212
#define LASER_STEPPER       11
#define TURNTABLE_STEPPER   10
//the protocol: we send one byte to define the action what to do.
//If the action is unary (like turnung off the light) we only need one byte so we are fine.
//If we want to tell the stepper to turn, a second byte is used to specify the number of steps.
//These second bytes are defined here below.

#define ACTION_BYTE         1    //normal byte, first of new action
#define LIGHT_INTENSITY     2
#define TURN_TABLE_STEPS    3
#define LASER1_STEPS        4
#define LASER2_STEPS        5
#define LASER_ROTATION      6
#define STEPPER_ID          7

int incomingByte = 0;
int byteType = 1;

LiquidCrystal_I2C	lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

//current motor: turn a single step
void step()
{
  digitalWrite(STEP_PIN_0, LOW);
  delay(3);
  digitalWrite(STEP_PIN_0, HIGH);
  delay(3);
}

//step the current motor for <count> times
void step(int count)
{
  for (int i = 0; i < count; i++)
  {
    step();
    lcd.setCursor (0, 1);
    lcd.print(i);
  }
}

void setup()
{

  lcd.begin (16, 2); //  <<----- My LCD was 16x2

  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home(); // go home

  lcd.print("3D Scan Ready!!!");

  // initialize the serial port
  Serial.begin(9600);
  pinMode(LASER_PIN, OUTPUT);

  pinMode(MS_PIN, OUTPUT);
  digitalWrite(MS_PIN, HIGH);  //HIGH for 16microstepping, LOW for no microstepping

  pinMode(ENABLE_PIN_0, OUTPUT);
  pinMode(DIR_PIN_0, OUTPUT);
  pinMode(STEP_PIN_0, OUTPUT);

  //disable all steppers at startup
  digitalWrite(ENABLE_PIN_0, HIGH);  //HIGH to turn off
  digitalWrite(LASER_PIN, LOW); //turn laser off
}

void loop()
{
  lcd.setCursor (0, 1);

  if (Serial.available() > 0)
  {
    lcd.print("                ");
    lcd.setCursor (0, 1);
    incomingByte = Serial.read();
    Serial.println(incomingByte);
    
    switch (byteType)
    {
      case ACTION_BYTE:
        switch (incomingByte)
        {  //this switch always handles the first byte
          //Laser
          case 49:	//1
          case TURN_LASER_OFF:
            digitalWrite(LASER_PIN, LOW);    // turn the LASER off
            lcd.print("TURN_LASER_OFF");
            break;
          case 50:	//2
          case TURN_LASER_ON:
            digitalWrite(LASER_PIN, HIGH);   // turn the LASER on
            lcd.print("TURN_LASER_ON");
            break;
          case ROTATE_LASER: //unused
            byteType = LASER_ROTATION;
            lcd.print("ROTATE_LASER");
            break;
          //TurnTable
          case 51:	//3
          case PERFORM_STEP:
            byteType = TURN_TABLE_STEPS;
            lcd.print("PERFORM_STEP");
            break;
          case 52:	//4
          case SET_DIRECTION_CW:
            digitalWrite(DIR_PIN_0, HIGH);
            lcd.print("SET_DIRECTION_CW");
            break;
          case 53:	//5
          case SET_DIRECTION_CCW:
            digitalWrite(DIR_PIN_0, LOW);
            lcd.print("SET_DIRECTION_CCW");
            break;
          case 54:	//6
          case TURN_STEPPER_ON:
            digitalWrite(ENABLE_PIN_0, LOW);
            lcd.print("TURN_STEPPER_ON");
            break;
          case 55:	//7
          case TURN_STEPPER_OFF:
            digitalWrite(ENABLE_PIN_0, HIGH);
            lcd.print("TURN_STEPPER_OFF");
            break;
          case 56:	//8
          case TURN_LIGHT_ON:
            byteType = LIGHT_INTENSITY;
            lcd.print("TURN_LIGHT_ON");
            break;
          case 57:	//9
          case TURN_LIGHT_OFF:
            digitalWrite(LIGHT_PIN, LOW);
            lcd.print("TURN_LIGHT_OFF");
            break;
          case 48:	//0
          case FABSCAN_PING:
            delay(1);
            Serial.write(FABSCAN_PONG);
            lcd.print("FABSCAN_PONG");
            break;
          case 113:	//q
          case SELECT_STEPPER:
            byteType = STEPPER_ID;
            lcd.print("SELECT_STEPPER");
            break;
        }

        break;
      case LIGHT_INTENSITY:       //after this point we take care of the second byte if one is sent
        analogWrite(LIGHT_PIN, incomingByte);
        byteType = ACTION_BYTE;  //reset byteType
        lcd.print("LIGHT_INTENSITY");
        break;
      case TURN_TABLE_STEPS:
        step(incomingByte);
        byteType = ACTION_BYTE;
        lcd.print("TURN_TABLE_STEPS");
        break;
    }
  }
}
