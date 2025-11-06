# Synchronization Quest - Application 4 

## Real Time Systems - Summer 2025
**Instructor:** Dr. Mike Borowczak

## Author
**Name:** Lakshmi Katravulapalli
**Theme:** Hardware Security

---

## Project Overview

This project explores task coordination and shared resource protection using FreeRTOS on the ESP32. The system simulates real-time sensor monitoring and alert handling using three key synchronization mechanisms:

- **Binary Semaphore** - signals from a user pushbutton
- **Counting Semaphore** - queues threshold events from an analog sensor
- **Mutex** - protects shared access to the serial console output

**Hardware Setup (Wokwi simulation):**

- 1x Potentiometer → GPIO34 (Analog Sensor)
- 1x Pushbutton → GPIO18
- 1x Red LED → GPIO4 (Alert)
- 1x Green LED → GPIO2 (Heartbeat)

---

## Engineering Analysis

Answer the following based on your project implementation and observations:

### 1. Signal Discipline
Signal Discipline: How does the binary semaphore synchronize the button task with the system? What happens if the button is pressed multiple times quickly? How might this differ if you’d used a counting semaphore instead?

**Answer:**  
The button task will signal the binary semaphore to run the event handler task if the button is pressed. If the user tries to press the button multiple times, the event handler task might not register it because every time it is triggered it resets to 0, and if they are pressed quickly at once the semaphore will not signal in time resulting in lost or delayed outputs. If we're using a counting semaphore, it will allows each button press to register as a count and the event handler task would process however many counts are given. This means that no presses would be lost or miscounted, making it the better option in this case. 

---

### 2. Event Flood Handling 
When you adjust the potentiometer rapidly above/below threshold, what behavior do you observe with the red LED and console prints? How does the counting semaphore handle multiple signals? What would break if you swapped it for a binary semaphore?

**Answer:**  
When I adjusted the potentiometer rapidly above and below threshold in my program, I noticed that the print statements were printing on time as expected, but I noticed the red LED skipped or delayed a few times when I was switching threshold's really quickly. The counting semaphore will queue events every time an event is triggered, and it can handle up to 10 semaphores for a 30-second period. If multiple signals are triggered, the counting semaphore can handle them by calling the event handler each time, and decrease the count by 1 after processing it. If we used a binary semaphore, the event handler might not trigger the immediate events and respond accordingly, so this might break the semaphore event function because it might not be able to process the multiple rapid tasks. This would cause the LED's to only blink once or the print statements to print once even if we changed the potentiometer multiple times rapidly.

Example Code Output (During Testing):

Sensor Value: 3655
Intrusion Sensor event: Threshold has been exceeded!
Sensor Value: 4095
Sensor Value: 4095
....
Sensor Value: 2722
Sensor Value: 4095
Intrusion Sensor event: Threshold has been exceeded!
Sensor Value: 4095
Sensor Value: 4095
...
Sensor Value: 1369
Sensor Value: 3655
Intrusion Sensor event: Threshold has been exceeded!
Sensor Value: 4095

---

### 3. Protecting Shared Output (Mutex)
Describe a moment where removing the mutex caused incorrect console behavior or other issues. Why do mutexes matter for protecting output? What real-world failure could occur if logs were interleaved or shared state was modified concurrently?

**Answer:**  
A potential moment where removing the mutex could cause incorrect behavior is when trying to print the values of the sensor, while also printing the push button response, this could lead to a misprinted statement or unexpected issues. Mutexes are important because they protect shared resources and make sure that they are not used at the same time. It ensures that only one task can access the shared resource at a time. This can help avoid real world failures like race conditions, deadlocks, data corruption, and missed output alerts. 

Example Code Snippet (I removed the print mutexes from the code and observed the behavior)

void intrusionAlertHandler_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(sem_sensor, 0)) {
            SEMCNT--;  // DO NOT MODIFY THIS LINE

            //xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("Intrusion Sensor event: Threshold has been exceeded!\n");
            //xSemaphoreGive(print_mutex);
           .......
        }

        if (xSemaphoreTake(sem_button, 0)) {
            //xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("Button event: Intrusion has been detected!\n");
            //xSemaphoreGive(print_mutex);
            .....
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Idle delay to yield CPU
    }
}

---

### 4. Scheduling and Preemption
Did task priorities influence system responsiveness as expected? Give one example where a high-priority task preempted a lower one. What happened to the heartbeat during busy periods?

**Answer:**  
The task priorities did not noticeably influence the responsiveness of the system. I played around with different priorities but did not see any changes in the behavior. Although, one example of a high-priority task pre-empting a lower one could happen when the other critical tasks like button, sensor, and event_handler have higher priorities of 3 and the securitySystem_Alive_task (heartbeat task) has the lowest priority of 1, this could cause this task to be delayed or stalled until a higher priority task finishes. 

I tested with different task priorities to see if it affected system behavior. 
Example Code Snippet from application:

// Create tasks
    xTaskCreate(securitySystem_Alive_task, "system_alive", 2048, NULL, 1, NULL);  //Hearbeat Task 
    xTaskCreate(intrusionSensor_task, "sensor", 2048, NULL, 3, NULL); //Sensor Task
    xTaskCreate(alertButton_task, "button", 2048, NULL, 3, NULL); //Button Task
    xTaskCreate(intrusionAlertHandler_task, "intrusion_event_handler", 2048, NULL, 3, NULL); //Event Handler Task


---

### 5. Timing and Responsiveness
The code provided uses `vTaskDelay` rather than `vTaskDelayUntil`. How did delays impact system responsiveness and behavior? Does the your polling rate affect event detection? Would you consider changing any of the `vTaskDelay` rather than `vTaskDelayUntil` - why or why not? Adjust your code accordingly.

**Answer:**  
Using vTaskDelay impacted system response by creating a relative delay period for a certain amount of time, but it can also lead to slow drifts over time which cause tasks to take longer than expected to run. The polling time for the sensor task is 100 ms, and the event handler and button task is 10 ms. Using a longer polling rate might slow down the responsiveness and reliability of tasks over time. I would consider using vTaskDelayUntil for the sensor task to make sure the sensor is reading and logging its values precisely on time and running systematically. 

Example Code Snippet:

void intrusionSensor_task(void *pvParameters) {
  static bool rising_edge = false;
  const TickType_t periodTicks = pdMS_TO_TICKS(10); // e.g. 10 ms period
  TickType_t lastWakeTime = xTaskGetTickCount(); // initialize last wake time
  
  while (1) {
        int val = adc1_get_raw(POT_ADC_CHANNEL);
	.....
    
    	if(val <= SENSOR_THRESHOLD)  //Checks if value is below threshold and set to false
          rising_edge = false; 

        vTaskDelayUntil(&lastWakeTime, periodTicks); //Used VTaskDelayUntil
    }
}



---

### 6. Theme Integration
Connect your implementation to your chosen theme. What does each task/LED/semaphore represent in that real-world system? How might synchronization be life-critical in your domain?

**Answer:**  
My chosen theme was hardware security and I chose to go with the same description that was provided in the assignment which could represent a security alarm that is constantly checking a sensor that informs the user of an intrusion and immediately alert them. The green LED from the heartbeat task (securitySystem_Alive_task) informs the user that the alarm system is alive and reading the sensor values. The red LED would inform the user that the threshold has been crossed and an intrusion has been detected, it can also be triggered by the push button. The sensor monitor task will be used by the security alarm to check the threshold by reading the sensor values. If the threshold is crossed it will immediately inform the semaphore in the event handler task to inform the user of an intrusion and security alert. The button task represents a button on an alarm that is triggered if an intrusion is detected an immediately alerts the owner in the event handler task. Synchronization is important in this theme because the alarm needs to immediately detect any intrusion and inform the user, if the sensor task skips a reading it could mean missing a intrusion/alert leading to a security breach or attack. 

Example Code snippet to show theme implementation:

void alertButton_task(void *pvParameters) {
.....
if (xSemaphoreTake(print_mutex, portMAX_DELAY)) {
                printf("Button Pressed: Security alert triggered.\n");
                xSemaphoreGive(print_mutex); 
}


---

### 7. [Bonus] Induced Failure - Starvation or Loss of Responsiveness
Can you create a situation where the system starves one or more tasks? (E.g., block the heartbeat for more than 3 seconds, or drop button events.) What caused it? Leave your code commented in the project with an explanation.

**Answer:**  
[Your response here]

---

## Presentation Slides

Link to your 4-slide summary here (google slides, onedrive powerpoint):

1. Introduction and Theme
2. Most Important Technical Lesson
3. Favorite Part of the Project
4. Something That Challenged You or You'd Explore More

(Bonus, Optional) If you included a voiceover, describe how to access it or link to a video.
### Access: I attached the Powerpoint Presentation and Recording to the Assignment Submission on Webcourses.
---

## Summary
You’re not just coding — you’re building a real-time system. Each semaphore is a signal. Each mutex is a lock guarding safety. Each LED pulse is a message from your system’s heartbeat.**
Can you keep your events ordered, your resources safe, and your system timely? This is your synchronization quest.
Good luck.

**Final Wokwi Project Link:** https://wokwi.com/projects/445119424199105537

Download the project Zip.
Head over to webcourses