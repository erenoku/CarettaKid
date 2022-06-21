#include "colorsensor.hpp"

#include "constants.hpp"
#include "debug.hpp"
ColorSensor::ColorSensor(int S3, int Out, int RedMin, int RedMax, int BlueMin,
                         int BlueMax, float BlueThresh, float RedThresh)
    : pin_s3(S3),
      pin_out(Out),
      red_last_millis(0),
      blue_last_millis(0),
      counting_red(0),
      counting_blue(0),
      r_min(RedMin),
      r_max(RedMax),
      b_min(BlueMin),
      b_max(BlueMax),
      b_thresh(BlueThresh),
      r_thresh(RedThresh) {}

void ColorSensor::setup() {
    pinMode(this->pin_s3, OUTPUT);
    pinMode(this->pin_out, INPUT);
}

ColorSensor::Colors ColorSensor::read_color() {
    digitalWrite(this->pin_s3, LOW);
    float RedPW =
        COLOR_CALCULATION_PRECISION_SCALING / pulseIn(this->pin_out, LOW);
    digitalWrite(this->pin_s3, HIGH);
    float BluePW =
        COLOR_CALCULATION_PRECISION_SCALING / pulseIn(this->pin_out, LOW);

    // Hmm
    float BlueValue = map(BluePW, this->b_min, this->b_max, 0, 255);
    float RedValue = map(RedPW, this->r_min, this->r_max, 0, 255);

    float TotalValue = BlueValue + RedValue;
    BlueValue /= TotalValue;
    RedValue /= TotalValue;

    DEBUG_PRINT("[Colors] BlueValue:");
    DEBUG_PRINT(BlueValue);
    DEBUG_PRINT("\n[Colors] RedValue: ");
    DEBUG_PRINT(RedValue);
    DEBUG_PRINT("\n");

    // Hmm
    if (BlueValue >
        b_thresh) {  // Rules can change depending upon calibration values
        return Colors::Blue;
    } else if (RedValue > r_thresh) {  // Rules can change depending upon
                                       // calibration values
        return Colors::Red;
    } else {
        return Colors::None;
    }
}

ColorSensor::Colors ColorSensor::sync_color() {
    // Public wrapper for threshholding
    Colors sensed = this->read_color();

    return sensed;

    switch (sensed) {
        case Colors::Red:
            counting_blue = false;
            if (counting_red) {
                if (millis() - red_last_millis >=
                    COLOR_SENSOR_CONSISTENCY_THRESHHOLD) {
                    return Colors::Red;
                } else {
                    return Colors::None;
                }
            } else {
                counting_red = true;
                red_last_millis = millis();
                return Colors::None;
            }
            break;
        case Colors::Blue:
            counting_red = false;
            if (counting_blue) {
                if (millis() - blue_last_millis >=
                    COLOR_SENSOR_CONSISTENCY_THRESHHOLD) {
                    return Colors::Blue;
                } else {
                    return Colors::None;
                }
            } else {
                counting_blue = true;
                blue_last_millis = millis();
                return Colors::None;
            }
            break;
        case Colors::None:
            counting_blue = false;
            counting_red = false;
            break;
    }
}
