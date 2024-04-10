// Include Arduino FreeRTOS library

#include "task.hpp"
#include <Arduino.h>
#include "StepperMotor.hpp"

//替换引脚编号
StepperMotor motor(223, 332);

void setup()
{
  Semaphore_Init();
  task_Create();
  Serial.begin(115200);
  motor.setGearRatio(2.0);
}

void loop() {
 motor.rotateWithEarth();
}
