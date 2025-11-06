/* --------------------------------------------------------------
   Application: 04 - Rev1
   Release Type: Use of Memory Based Task Communication
   Class: Real Time Systems - Su 2025
   Author: [M Borowczak] 
   Email: [mike.borowczak@ucf.edu]
   Company: [University of Central Florida]
   Website: theDRACOlab.com
   AI Use: Please commented inline where you use(d) AI
---------------------------------------------------------------*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"

//TODO 8 - Update the code variables and comments to match your selected thematic area!

//TODO 0a connect the components in the diagram to the GPIO pins listed below.
//Note even if Wokwi won't blow your LEDs, we'll assume them to be off if they're not connected to resistors!
//Next, goto TODO 0b
#define LED_GREEN GPIO_NUM_2 // Green LED -> GPIO 2
#define LED_RED   GPIO_NUM_4 // Red LED -> GPIO 4
#define BUTTON_PIN GPIO_NUM_18 //Push Button -> GPIO 18
#define POT_ADC_CHANNEL ADC1_CHANNEL_6 // slide-potentiometer -> GPIO 34

#define MAX_COUNT_SEM 10 
// TODO 7: Based on the speed of events; 
//can you adjust this MAX Semaphore Counter to not miss a high frequency threshold events
//over a 30 second time period, e.g., assuming that the sensor exceeds the threshold of 30 seconds, 
//can you capture every event in your counting semaphore? what size do you need?


// Threshold for analog sensor
//TODO 1: Adjust threshold based on your scenario or input testing
// You should modify SENSOR_THRESHOLD to better match your Wokwi input behavior;
// note the min/max of the adc raw reading
#define SENSOR_THRESHOLD 3000

// Handles for semaphores and mutex - you'll initialize these in the main program
SemaphoreHandle_t sem_button;
SemaphoreHandle_t sem_sensor;
SemaphoreHandle_t print_mutex;

volatile int SEMCNT = 0; //You may not use this value in your logic -- but you can print it if you wish

//TODO 0b: Set heartbeat to cycle once per second (on for one second, off for one second)
//Find TODO 0c
void securitySystem_Alive_task(void *pvParameters) {
    while (1) {
        //Turn LED on for one second
        gpio_set_level(LED_GREEN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
      //Turn the LED off for one second
        gpio_set_level(LED_GREEN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void intrusionSensor_task(void *pvParameters) {
  static bool rising_edge = false;
  
  while (1) {
        int val = adc1_get_raw(POT_ADC_CHANNEL);

        //TODO 2: Add serial print to log the raw sensor value (mutex protected)
        //Hint: use xSemaphoreTake( ... which semaphore ...) and printf
            if (xSemaphoreTake(print_mutex, portMAX_DELAY)) {
                printf("Sensor Value: %d\n", val);
                xSemaphoreGive(print_mutex); 
            }
 
        if (val > SENSOR_THRESHOLD && !rising_edge) {
            if(SEMCNT < MAX_COUNT_SEM+1) SEMCNT++; // DO NOT REMOVE THIS LINE
            //TODO 3: prevent spamming by only signaling on rising edge; See prior application #3 for help!
            xSemaphoreGive(sem_sensor);  // Signal sensor event
          	rising_edge = true; ///Rising Edge was detected
        }
    
    	if(val <= SENSOR_THRESHOLD)  //Checks if value is below threshold and set to false
          rising_edge = false; 

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}



void alertButton_task(void *pvParameters) {
    while (1) {
        int state = gpio_get_level(BUTTON_PIN);
        static uint32_t last_press_tick = 0;  //Save the last time the button was pressed
        uint32_t current_tick = xTaskGetTickCount(); 

        // TODO 4a: Add addtional logic to prevent bounce effect (ignore multiple events for 'single press')
        if(state == 0 && (current_tick - last_press_tick) > pdMS_TO_TICKS(300))
        {
	        last_press_tick = current_tick; //Save the time of the last button press
          xSemaphoreGive(sem_button);
          //TODO 4b: Add a console print indicating button was pressed (mutex protected); different message than in event handler
          if (xSemaphoreTake(print_mutex, portMAX_DELAY)) {
                printf("Button Pressed: Security alert triggered.\n");
                xSemaphoreGive(print_mutex); 
            }
        }
            
        vTaskDelay(pdMS_TO_TICKS(10)); // Do Not Modify This Delay!
    }
}


void intrusionAlertHandler_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(sem_sensor, 0)) {
            SEMCNT--;  // DO NOT MODIFY THIS LINE

            xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("Intrusion Sensor event: Threshold has been exceeded!\n");
            xSemaphoreGive(print_mutex);

            gpio_set_level(LED_RED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_RED, 0);
        }

        if (xSemaphoreTake(sem_button, 0)) {
            xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("Button event: Intrusion has been detected!\n");
            xSemaphoreGive(print_mutex);

            gpio_set_level(LED_RED, 1);
            vTaskDelay(pdMS_TO_TICKS(300));
            gpio_set_level(LED_RED, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Idle delay to yield CPU
    }
}

void app_main(void) {
    // Configure output LEDs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GREEN) | (1ULL << LED_RED),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // Configure input button
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&btn_conf);

    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POT_ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Create sync primitives
    // TODO 0c: Attach the three SemaphoreHandle_t defined earlier 
    // (sem_button, sem_sensor, print_mutex) to appropriate Semaphores.
    // binary, counting, mutex by using the appropriate xSemaphoreCreate APIs.
    // the counting semaphore should be set to (MAX_COUNT_SEM,0);
    // Move on to TODO 1; remaining TODOs are numbered 1,2,3, 4a 4b, 5, 6 ,7
    print_mutex = xSemaphoreCreateMutex(); // Create a Mutex for print statements
    sem_button = xSemaphoreCreateBinary(); //Create a binary semaphore for push button
    sem_sensor = xSemaphoreCreateCounting(MAX_COUNT_SEM,0); //Create a counting semaphore

    //TODO 5: Test removing the print_mutex around console output (expect interleaving)
    //Observe console when two events are triggered close together

    // Create tasks
    xTaskCreate(securitySystem_Alive_task, "system_alive", 2048, NULL, 1, NULL);  //Hearbeat Task 
    xTaskCreate(intrusionSensor_task, "sensor", 2048, NULL, 2, NULL); //Sensor Task
    xTaskCreate(alertButton_task, "button", 2048, NULL, 3, NULL); //Button Task
    xTaskCreate(intrusionAlertHandler_task, "intrusion_event_handler", 2048, NULL, 2, NULL); //Event Handler Task


    //TODO 6: Experiment with changing task priorities to induce or fix starvation
    //E.G> Try: xTaskCreate(sensor_task, ..., 4, ...) and observe heartbeat blinking
    //You should do more than just this example ...
}
