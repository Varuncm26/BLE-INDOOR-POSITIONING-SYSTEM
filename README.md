
# 📍 Low-Cost BLE Indoor Localization System using ESP32

![Platform](https://img.shields.io/badge/Platform-ESP32-red)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Arduino-blue)
![Connectivity](https://img.shields.io/badge/Connectivity-BLE%20%26%20ESP--NOW-green)
![Status](https://img.shields.io/badge/Status-Completed-success)

## 📖 Project Overview

This project presents a **Bluetooth Low Energy (BLE) device monitoring and indoor localization system** implemented using ESP32 microcontrollers. It is designed to detect, track, and visualize active BLE devices in restricted or sensitive indoor environments such as examination halls, laboratories, and secure zones.

The system passively monitors BLE advertising packets, processes unstable RSSI measurements using **Kalman filtering**, estimates distance via a **Log-Distance Path Loss model**, and localizes devices using **Trilateration** and **Min-Max algorithms**. A web-based interface provides real-time visualization of detected devices and their estimated positions.

## 🎯 Objectives

- [x] **Detect** unauthorized BLE devices in restricted indoor areas.
- [x] **Reduce** RSSI fluctuations using lightweight Kalman filtering techniques.
- [x] **Achieve** accurate indoor localization using minimal hardware costs.
- [x] **Compare** localization performance of Trilateration vs. Min-Max methods.
- [x] **Visualize** real-time monitoring through a web-based dashboard.

---

## 🛠 Hardware Architecture

The system utilizes **four ESP32 microcontrollers** in a distributed network:

### 1. Anchor Nodes (3x ESP32)
* Placed at fixed, known coordinates.
* Continuously scan for BLE advertisements.
* Measure RSSI from target devices.
* Apply **Kalman Filtering** for signal smoothing.
* Estimate distance using calibrated path-loss parameters.

### 2. Server Node (1x ESP32)
* Acts as the central processing unit.
* Aggregates data from all anchors via **ESP-NOW** (low latency).
* Executes the localization algorithms (Trilateration / Min-Max).
* Hosts the **Async Web Server** for the user dashboard.

---

## 📡 Signal Processing & Algorithms

### 1. RSSI Filtering (Kalman Filter)
Raw RSSI values are highly unstable due to multipath propagation, human movement, and signal shadowing. We implemented a **1D Kalman Filter** to:
* Smooth noisy RSSI samples.
* Outperform standard moving average methods.
* Provide stable distance estimates even in dynamic environments.

### 2. Distance Estimation
Distances are calculated using the **Log-Distance Path Loss Model**:
> **RSSI = -10n log10(d) + A**

* **A:** Reference RSSI at 1 meter.
* **n:** Path loss exponent (environmental factor).
* *Note: Both parameters were calibrated experimentally for each specific anchor.*

### 3. Localization Algorithms

| Algorithm | Method | Strengths | Weaknesses |
| :--- | :--- | :--- | :--- |
| **Trilateration** | Intersection of 3 circles | Good in Line-of-Sight (LOS) | High sensitivity to RSSI errors; Fails in noisy environments |
| **Min-Max** | Bounding box intersection | Robust against outliers; Computationally efficient | Slightly less precise in perfect conditions |

---

## 📊 Experimental Setup & Results

**Environment:** 5m × 5m indoor room
**Anchor Coordinates:** `(0,0)`, `(5,0)`, `(0,5)`
**Anchor Height:** 1.25 meters

### Key Findings
1.  **Kalman Filtering** significantly stabilizes RSSI signals compared to raw data.
2.  **Distance Accuracy** improves noticeably when using calibrated *A* and *n* values.
3.  **Algorithm Performance:** The **Min-Max algorithm** consistently outperformed Trilateration in real-world, noisy indoor environments, providing more reliable coordinate estimation.

---

## 🌐 Web Interface

The Server Node hosts a web page accessible via browser:
* **Scan:** Automatically lists detected BLE devices (Name & MAC).
* **Track:** User selects a specific target to track.
* **Visualize:** Real-time 2D plot of the room showing the target's estimated position.
* **Debug:** Displays real-time RSSI values and calculated distances from all 3 anchors.

---

## 📂 Folder Structure








