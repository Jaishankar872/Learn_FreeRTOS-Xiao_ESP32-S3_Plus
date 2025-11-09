#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Use only 1 core for FreeRTOS demo tasks
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Pin
static const int ledPin = BUILTIN_LED; // On-board LED pin

// Task 1 - Blink LED
void toggleLED(void *parameter)
{
  while (true)
  {
    digitalWrite(ledPin, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(ledPin, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  pinMode(ledPin, OUTPUT);

  // Create Blink LED Task
  xTaskCreatePinnedToCore( // Use xTaskCreate() in vanilla FreeRTOS
      toggleLED,           // Name of the task
      "Toggle LED Task",   // Name of the task (for debugging)
      1024,                // Stack size (in words)
      NULL,                // Task input parameter
      1,                   // Priority of the task
      NULL,                // Task handle
      app_cpu              // Run on one core for demo purposes (ESP32 only)
  );
  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() in
  // main after setting up your tasks.
}

void loop()
{
  // Main loop does nothing, all work is done in the FreeRTOS task
}