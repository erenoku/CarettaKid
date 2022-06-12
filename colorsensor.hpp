#pragma once
#include <Arduino.h>

class ColorSensor {

  public:
    ColorSensor(int S3, int Out, int RedMin, int RedMax, int BlueMin, int BlueMax, float BlueThresh, float RedThresh);

    void setup();

    enum class Colors {
      Red,
      Blue,
      None
    };
    Colors sync_color();
  private:
    Colors read_color();

    const int r_min;
    const int r_max;
    const int b_min;
    const int b_max;

    const float r_thresh;
    const float b_thresh;

    const int pin_s3;
    const int pin_out;

    unsigned long red_last_millis;
    unsigned long blue_last_millis;

    bool counting_red;
    bool counting_blue;
};