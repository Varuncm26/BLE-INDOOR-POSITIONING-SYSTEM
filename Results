## 📊 Experimental Results

We conducted extensive testing in a **5m × 5m indoor environment** to validate the system's performance. The results highlight the effectiveness of Kalman Filtering and the robustness of the Min-Max algorithm.

### 1. Path Loss Calibration
[cite_start]To ensure accurate distance estimation, we calibrated the Path Loss Model parameters ($A$ and $n$) for each anchor individually using Line-of-Sight (LOS) experiments[cite: 26, 194].

| Anchor Node | Reference RSSI ($A$) | Path Loss Exponent ($n$) |
| :--- | :---: | :---: |
| **Anchor 1** | -46.75 dBm | 2.962 |
| **Anchor 2** | -44.90 dBm | 3.415 |
| **Anchor 3** | -50.20 dBm | 2.489 |

### 2. Signal Smoothing (Kalman Filter vs. Moving Average)
[cite_start]Raw RSSI signals fluctuated significantly (-52 dBm to -72 dBm) due to multipath effects[cite: 174]. The Kalman Filter successfully smoothed these fluctuations, reducing distance estimation error significantly compared to standard averaging.

**Performance at 2 meters:**
* **Standard Averaging:** Est. [cite_start]Distance = 2.69 m (Error: 0.69 m) [cite: 176]
* **Kalman Filter:** Est. [cite_start]Distance = **2.28 m** (Error: 0.28 m) [cite: 175]

> [cite_start]**Key Finding:** The Kalman filter consistently provided lower error rates, especially at distances greater than 7 meters, where standard averaging error spiked to ~7.3m while Kalman remained closer to ~6m.

### 3. Localization Accuracy: Min-Max vs. Trilateration
We tested the system at various known coordinates. [cite_start]The **Min-Max algorithm** outperformed Trilateration, which often produced unrealistic coordinates (outside the room boundaries) when RSSI noise was high [cite: 200-203].

| Actual Position (x,y) | Min-Max Estimate | Trilateration Estimate | Verdict |
| :--- | :--- | :--- | :--- |
| **(3.0, 3.0)** | **(2.05, 2.95)** | (5.82, 6.40) ❌ | Min-Max is closer; Trilateration failed |
| **(2.0, 2.0)** | **(1.82, 1.74)** | (0.78, 2.75) | Min-Max is closer |
| **(2.5, 3.0)** | **(1.36, 3.64)** | (6.32, 7.66) ❌ | Trilateration exceeded room limits |
| **(3.0, 4.0)** | **(1.73, 3.27)** | (5.54, 6.63) ❌ | Min-Max is more robust |

[cite_start]**Conclusion:** The Min-Max algorithm demonstrated greater robustness against outliers, maintaining accuracy even when individual range estimates were flawed[cite: 203].

### 4. Real-Time Web Visualization
The system hosts a web interface on the ESP32 server that allows:
1.  [cite_start]**Scanning:** Detects nearby BLE devices and MAC addresses[cite: 207].
2.  [cite_start]**Tracking:** User selects a target; anchors synchronize to track it[cite: 209].
3.  [cite_start]**Visualization:** Displays the estimated position on a 2D grid using both algorithms for comparison[cite: 214].

![Web Interface Localization](images/web_interface_plot.png)
*(Note: Replace this link with a screenshot of your Figure 3.11)*
