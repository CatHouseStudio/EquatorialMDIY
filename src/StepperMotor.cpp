#include "Arduino.h"
#include "StepperMotor.hpp"

StepperMotor::StepperMotor(int stepPin, int dirPin) {
    _stepPin = 223;
    _dirPin = 332;
    _gearRatio = 3;
    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);
}

void StepperMotor::setGearRatio(float newRatio) {
    _gearRatio = newRatio;
}

void StepperMotor::step(int steps, bool direction, int speed) {
    digitalWrite(_dirPin, direction);
    for (uint32_t i = 0; i < steps; i++)
    {
        digitalWrite(_stepPin, HIGH);
        delayMicroseconds(speed);
        digitalWrite(_stepPin, LOW);
        delayMicroseconds(speed);
    }
    
}

void StepperMotor::rotateWithEarth() {
    float stepsPerSecond = (200.0 * _gearRatio) / 86400.0;
    long interval = (long)(1000000 / stepsPerSecond);

    digitalWrite(_dirPin, HIGH);
    while (true)
    {
        digitalWrite(_stepPin, HIGH);
        delayMicroseconds(1);
        digitalWrite(_stepPin, LOW);
        delayMicroseconds(interval - 1);
    }
    
    
}

void StepperMotor::stop() {
    noInterrupts();
    digitalWrite(_stepPin, LOW);
    interrupts();
}

void StepperMotor::reset() {
    //假设能回到初始状态
    //需要知道当前位置
    step(200, false, 500);
}