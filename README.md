# BLE-INDOOR-POSITIONING-SYSTEM
Hardware implementation of a low-cost BLE device monitoring and indoor localization system using ESP32. The project uses RSSI sensing with Kalman filtering, path-loss modeling, and Min-Max/Trilateration algorithms, plus a web interface for real-time device detection in restricted areas.

📌 Project Overview

This project presents a low-cost Bluetooth Low Energy (BLE) device monitoring and indoor localization system implemented using ESP32 microcontrollers. It is designed to detect, track, and visualize active BLE devices in restricted or sensitive indoor environments such as examination halls, laboratories, and secure zones.

The system passively monitors BLE advertising packets, processes unstable RSSI measurements using Kalman filtering, estimates distance via a log-distance path loss model, and localizes devices using Trilateration and Min-Max algorithms. A web-based interface provides real-time visualization of detected devices and their estimated positions.


🎯 Objectives

Detect unauthorized BLE devices in restricted indoor areas
Reduce RSSI fluctuations using lightweight filtering techniques
Achieve accurate indoor localization using minimal hardware
Compare localization performance of Trilateration and Min-Max methods
Provide real-time monitoring through a web-based dashboard


🛠 Hardware Architecture

The system is built using four ESP32 microcontrollers:
✅ Anchor Nodes (3 × ESP32)
Continuously scan for BLE advertisements
Measure RSSI from nearby BLE devices
Apply Kalman filtering for signal smoothing
Estimate distance using calibrated path-loss parameters

✅ Server Node (1 × ESP32)
Aggregates data from all anchors via ESP-NOW
Executes localization algorithms
Hosts a web server for visualization and user interaction
All anchors are placed at known coordinates in a 5 m × 5 m indoor environment.


📡 Signal Processing & RSSI Filtering
Raw RSSI values are highly unstable due to:
Multipath propagation
Human movement
Signal shadowing and interference
To overcome this, the project implements a Kalman Filter, which:
Smooths noisy RSSI samples
Outperforms standard moving average methods
Provides stable distance estimates even in dynamic environments
Kalman filtering significantly reduces distance estimation error, especially beyond 7 meters.

📐 Distance Estimation – Path Loss Model

The system converts filtered RSSI values into distance using the Log-Distance Path Loss Model:
Environmental parameters (A and n) are calibrated experimentally
RSSI samples are collected at 1–5 meter intervals
Calibration is performed individually for each anchor
Provides environment-specific accuracy
This calibration ensures reliable distance estimation in indoor conditions.

📍 Localization Algorithms
🔹 Trilateration

Uses intersection of three distance-based circles
Highly sensitive to RSSI and distance errors
Performs well under Line-of-Sight (LOS) conditions
Fails under noisy or NLOS environments

🔹 Min-Max Algorithm

Uses bounding boxes instead of circle intersections
Robust against distance estimation outliers
Computationally efficient
Provides stable localization in real-world conditions

📊 Result: Min-Max consistently outperformed Trilateration in noisy indoor environments.

🌐 Web-Based Visualization

A web server hosted on the ESP32 server node provides:
Automatic BLE device scanning
Display of device names and MAC addresses
User-selectable device tracking
Real-time plotting of estimated position
Distance and localization algorithm comparison
Users can access the interface via a browser using the ESP32’s IP address.

🧪 Experimental Setup

Environment: 5 m × 5 m indoor room
Anchor placement: (0,0), (5,0), (0,5)
Anchor height: 1.25 meters
Tested under LOS and simulated noisy conditions
Multiple trials conducted for validation

📊 Results & Key Findings

Kalman filtering significantly stabilizes RSSI signals
Distance estimation accuracy improves over standard averaging
Trilateration suffers from large coordinate errors under noise
Min-Max provides reliable localization even in harsh conditions
System performs well using only four low-cost ESP32 devices








