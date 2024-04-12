#ifndef EQUATORIAL_MOUNT_HPP
#define EQUATORIAL_MOUNT_HPP

#include <Arduino.h>

extern const uint32_t STEP_PIN;
extern const uint32_t DIR_PIN;
extern const uint32_t ENABLE_PIN;

void setupMotorControl();

void setMotorSpeed(long speed);

void stepMotor(uint32_t steps, bool direction);



#endif