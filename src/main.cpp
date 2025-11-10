#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Use only 1 core for FreeRTOS demo tasks
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Task Handles
TaskHandle_t task_1 = NULL; // LED Toggle Task
TaskHandle_t task_2 = NULL; // Serial Monitor Task

// Pin
static const int ledPin = BUILTIN_LED;   // On-board LED pin
int16_t delayTime = 500;                 // Delay time in milliseconds
int8_t kill_task1 = 0, enable_task1 = 0; // Flags to control task termination and recreation

// Task 1 - Blink LED
void toggleLED(void *parameter)
{
  while (true)
  {
    digitalWrite(ledPin, HIGH);
    vTaskDelay(delayTime / portTICK_PERIOD_MS);
    digitalWrite(ledPin, LOW);
    vTaskDelay(delayTime / portTICK_PERIOD_MS);
    // Memory Flow create and see how MCU handles it or reboots
    // Here Stacks size only 1024 words (4KB)
    int a = 1;
    int b[10];
    for (int i = 0; i < 10; i++)
    {
      b[i] = a * 2;
      a = b[i];
    }
    Serial.println(b[5]);

    // print out remaining stack for this task
    Serial.print("Remaining stack for LED Task(in words): ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
  }
}

// Task 2 - Read Serial Monitor
void readSerialMonitor(void *parameter)
{
  while (true)
  {
    if (Serial.available() > 0)
    {
      String input = Serial.readStringUntil('\n');
      int16_t inputDelay = input.toInt();
      if (inputDelay > 0)
      {
        delayTime = inputDelay; // Copy input to delayTime
        Serial.print("Delay time set to: ");
        Serial.println(delayTime);
        if (inputDelay == 256)
        {
          Serial.println("Terminating LED toggle task.");
          kill_task1 = 1;
        }
        else if (inputDelay == 512)
        {
          enable_task1 = 1;                     // Set flag to terminate task in loop()
          vTaskDelay(100 / portTICK_PERIOD_MS); // Small delay to ensure task
        }
      }
      else
      {
        Serial.println("Please enter a valid positive integer for delay time.");
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Small delay to avoid busy waiting
  }
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

  vTaskDelay(4000 / portTICK_PERIOD_MS); // Wait for Serial to initialize

  Serial.println(" Demo: FreeRTOS on ESP32 - Task Handles ");
  // Print Self Priority
  Serial.print("Setup and loop running on core: ");
  Serial.println(xPortGetCoreID());
  Serial.print("with priority: ");
  Serial.println(uxTaskPriorityGet(NULL));

  Serial.println(" Enter delay time in milliseconds for LED toggle.");
  Serial.println(" 1. 256 - terminate LED task");
  Serial.println(" 2. 512 - recreate LED task");

  // Create Blink LED Task
  xTaskCreatePinnedToCore( // Use xTaskCreate() in vanilla FreeRTOS
      toggleLED,           // Name of the task
      "Toggle LED Task",   // Name of the task (for debugging)
      1024,                // Stack size (in words)
      NULL,                // Task input parameter
      1,                   // Priority of the task
      &task_1,             // Task handle
      app_cpu              // Run on one core for demo purposes (ESP32 only)
  );
  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() in
  // main after setting up your tasks.
  xTaskCreatePinnedToCore(   // Use xTaskCreate() in vanilla FreeRTOS
      readSerialMonitor,     // Name of the task
      "Read Serial Monitor", // Name of the task (for debugging)
      1024,                  // Stack size (in words)
      NULL,                  // Task input parameter
      2,                     // Highest Priority
      &task_2,               // Task handle
      app_cpu                // Run on one core for demo purposes (ESP32 only)
  );
}

void loop()
{
  if (kill_task1 == 1)
  {
    if (task_1 != NULL)
    {
      vTaskDelete(task_1);
      task_1 = NULL;
      digitalWrite(ledPin, HIGH); // Ensure LED is turned off
      Serial.println("LED toggle task terminated.");
      kill_task1 = 0; // Reset kill
    }
  }

  if (enable_task1 == 1)
  {
    if (task_1 == NULL)
    {
      // Recreate Blink LED Task
      xTaskCreatePinnedToCore( // Use xTaskCreate() in vanilla FreeRTOS
          toggleLED,           // Name of the task
          "Toggle LED Task",   // Name of the task (for debugging)
          1024,                // Stack size (in words)
          NULL,                // Task input parameter
          1,                   // Priority of the task
          &task_1,             // Task handle
          app_cpu              // Run on one core for demo purposes (ESP32 only)
      );
      Serial.println("LED toggle task recreated.");
      enable_task1 = 0; // Reset enable flag
    }
  }
}