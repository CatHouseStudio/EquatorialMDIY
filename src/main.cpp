// Include Arduino FreeRTOS library

#include "task.hpp"
#include <Arduino.h>
#include "StepperMotor.hpp"
#include "EquatorialMount.hpp"

//替换引脚编号
StepperMotor motor(STEP_PIN, DIR_PIN);

void setup()
{
  Semaphore_Init();
  task_Create();
  Serial.begin(115200);
  // motor.setGearRatio(2.0);
  setupMotorControl();
  setMotorSpeed(94);
  
}

void loop() {
//  motor.rotateWithEarth();

  stepMotor(1, HIGH);
  //考虑减速比之后设置的步数
  delay(3600000 / 94);
}
