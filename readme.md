# Synchronization Quest - Application 4 

## Real Time Systems - Fall 2025
**Instructor:** Dr. Mike Borowczak

## Author
**Name:** Lakshmi Katravulapalli
**Theme:** Hardware Security

System Description: The security system will read an “EM Signature” sensor and send an alert if thresholds are crossed, and the push button will trigger an intrusion alert response. 
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

**Final Wokwi Project Link:** https://wokwi.com/projects/445119424199105537


