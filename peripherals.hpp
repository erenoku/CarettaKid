#pragma once

#include <Arduino.h>

// -- PINS --

//Color Sensor Pins

#define PIN_PUCK_COLOR_S3 A2
#define PIN_PUCK_COLOR_OUT A3
#define PIN_BASE_COLOR_S3 A0
#define PIN_BASE_COLOR_OUT A1


//DC Motor Pins

#define PIN_M1F 5
#define PIN_M1B 6
#define PIN_M2F 11
#define PIN_M2B 12

//TODO: Speed Pins

#define PIN_M1S 3
#define PIN_M2S 4

//Servo Pins

#define PIN_SERVO_PUCK 10
#define PIN_SERVO_BASE 9

//Sonic Sensor Pins

#define PIN_SONIC_LEFT_ECHO 7
#define PIN_SONIC_LEFT_TRIG 8
#define PIN_SONIC_RIGHT_ECHO A4
#define PIN_SONIC_RIGHT_TRIG A5

//Team Switch Pin

#define PIN_IS_TEAM_BLUE 2