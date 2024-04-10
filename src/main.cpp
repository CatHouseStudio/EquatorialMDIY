// Include Arduino FreeRTOS library

#include "task.hpp"

void setup()
{
  Semaphore_Init();
  task_Create();
}

void loop() {}
