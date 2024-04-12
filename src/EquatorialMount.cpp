#include "EquatorialMount.hpp"

const uint32_t STEP_PIN = 27;
const uint32_t DIR_PIN = 26;
const uint32_t ENABLE_PIN = 14;

void setupMotorControl() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);
}

void setMotorSpeed(long speed) {

}

void stepMotor(uint32_t steps, bool direction) {
    digitalWrite(DIR_PIN, direction);
    for (uint32_t i = 0; i < steps; i++)
    {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(500);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(500);
    }
    
}