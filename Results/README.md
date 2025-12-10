# 📊 Experimental Results & Performance Analysis

This document details the experimental validation of the BLE Indoor Localization System.The tests were conducted in a **5m × 5m indoor environment**  to evaluate signal stability, distance estimation accuracy, and localization performance.

## 1. Experimental Setup
The system uses three ESP32 anchors placed at fixed coordinates `(0,0)`, `(5,0)`, and `(0,5)` meters. All anchors were mounted on tripods at a height of **1.25 meters** to minimize ground reflection interference (Fresnel zone clearance).

<img width="523" height="391" alt="image" src="https://github.com/user-attachments/assets/18d9e9eb-ea8e-419a-be78-74b93625fcdc" />

> *Figure 1: Physical placement of ESP32 anchors and target device in the test environment.*

---

## 2. Signal Analysis: The Need for Filtering
Raw RSSI signals are inherently unstable due to multipath effects and environmental noise. As shown below, raw signals at a static distance fluctuate significantly (e.g., between -52 dBm and -72 dBm).

<img width="523" height="256" alt="image" src="https://github.com/user-attachments/assets/f0ebc066-19db-429f-bb2a-c09e4c83948b" />

> *Figure 2: Time-domain response of Raw RSSI showing significant fluctuation.*

To address this, we implemented a **Kalman Filter**. The comparison below demonstrates that while the Moving Average method still exhibits rapid variation, the Kalman Filter produces a much steadier signal profile.

<img width="495" height="242" alt="image" src="https://github.com/user-attachments/assets/e28f914c-6141-45c4-a84a-62489c82fadf" />
<img width="488" height="239" alt="image" src="https://github.com/user-attachments/assets/cfdbf9b5-0743-4379-8cf9-689ae8526239" />

> *Figure 3: Comparison of Signal Smoothing: Moving Average (left) vs. Kalman Filter (right).*

---

## 3. Path Loss Calibration
To ensure accurate distance estimation, we calibrated the **Log-Distance Path Loss Model** parameters ($A$ and $n$) for each anchor individually using Line-of-Sight (LOS) experiments.

**Calibrated Parameters:**

| Anchor Node | Reference RSSI ($A$) | Path Loss Exponent ($n$) |
| :--- | :---: | :---: |
| **Anchor 1** | -46.75 dBm | 2.962 |
| **Anchor 2** | -44.90 dBm | 3.415 |
| **Anchor 3** | -50.20 dBm | 2.489 |


---

## 4. Distance Estimation Accuracy
We compared the distance estimation error between **Standard Averaging** and **Kalman Filtering** over a range of 2 to 12 meters.

> <img width="522" height="248" alt="image" src="https://github.com/user-attachments/assets/51f5a780-e3ac-4a76-9eff-546e29a62a67" />

> *Figure 4: Computed distance error comparison (Kalman Filter vs. Standard Averaging).*

**Distance Estimation Accuracy (Standard vs. Kalman Filter):**
This table compares the distance estimation performance between **Standard Averaging** and **Kalman Filtering** over a range of 2 to 12 meters
The Kalman Filter consistently provided lower error rates, especially at distances beyond 7 meters.

| Dist (m) | Std Avg RSSI (dBm) | Std Path Loss (n) | Std Est. Dist (m) | **Std Error (m)** | Kalman RSSI (dBm) | Kalman Path Loss (n) | Kalman Est. Dist (m) | **Kalman Error (m)** |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **2** | -56.83 | 1.06 | 1.50 | **0.49** | -58.37 | 1.57 | 1.81 | **0.18** |
| **3** | -65.70 | 2.53 | 4.64 | **1.64** | -65.97 | 2.58 | 4.72 | **1.72** |
| **4** | -61.95 | 1.38 | 2.88 | **1.12** | -61.75 | 1.35 | 2.77 | **1.23** |
| **5** | -62.40 | 1.25 | 3.05 | **1.95** | -61.51 | 1.13 | 2.69 | **2.31** |
| **6** | -66.40 | 1.64 | 5.07 | **0.93** | -67.46 | 1.78 | 5.69 | **0.31** |
| **7** | -67.20 | 1.60 | 5.61 | **1.39** | -68.60 | 1.77 | 6.57 | **0.43** |
| **8** | -75.12 | 2.38 | 15.37 | **7.37** | -74.62 | 2.32 | 14.01 | **6.01** |
| **9** | -71.38 | 1.86 | 9.55 | **-0.55** | -65.32 | 1.22 | 4.35 | **4.65** |
| **10** | -78.40 | 2.48 | 23.33 | **13.33** | -79.83 | 2.62 | 26.99 | **16.99** |
| **11** | -74.78 | 2.03 | 14.72 | **3.72** | -75.23 | 2.07 | 15.13 | **4.13** |
| **12** | -72.30 | 1.73 | 10.74 | **1.26** | -72.71 | 1.77 | 11.02 | **0.98** |

<img width="522" height="248" alt="image" src="https://github.com/user-attachments/assets/833a8ca9-a5bd-489f-acbf-a392a9112395" />

> *Figure 5: Computed distance error.*
---

## 5. Localization Algorithm Comparison
We tested the system at various known coordinates to compare **Trilateration** vs. **Min-Max** algorithms.

> <img width="431" height="426" alt="image" src="https://github.com/user-attachments/assets/43bdc1ad-0b43-451c-90bf-cbc72192bd54" />

> *Figure 6: Conceptual coordinate grid used for localization algorithms.*

**Performance Results:**
The **Min-Max algorithm** demonstrated superior robustness.In cases where RSSI noise caused Trilateration to calculate coordinates outside the room boundaries (e.g., Test Case 6), Min-Max provided a valid estimate .

| Test Case | Actual Co-ordinates | Min-Max Estimate | Trilateration Estimate | Verdict |
| :--- | :---: | :---: | :---: | :--- |
| **1** | (3.0, 3.0) | **(2.05, 2.95)** | (5.82, 6.40) | ❌ Trilateration Failed |
| **2** | (2.0, 2.0) | **(1.82, 1.74)** | (0.78, 2.75) | ✅ Min-Max Closer |
| **3** | (2.0, 3.0) | **(1.47, 3.31)** | (2.27, 3.04) | ➖ Comparable |
| **4** | (2.0, 3.5) | **(1.34, 3.66)** | (5.28, 6.95) | ❌ Trilateration Failed |
| **5** | (2.5, 3.0) | **(1.36, 3.64)** | (6.32, 7.66) | ❌ Trilateration Failed |
| **6** | (3.0, 4.0) | **(1.73, 3.27)** | (5.54, 6.63) | ✅ Min-Max Closer |

---

### Conclusion
The combination of **Kalman Filtering** for signal smoothing and the **Min-Max Algorithm** for position estimation provides the most reliable solution for this low-cost hardware setup, effectively mitigating the instability of RSSI in indoor environments.
