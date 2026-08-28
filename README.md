---
publishDate: 2026-08-26T00:00:00Z
title: SENTRA — Sepsis Early Non-invasive Tracking & Risk Assessment
excerpt: A low-cost, non-invasive early-warning prototype that combines pulse, breathing, and mobility trends into one transparent bedside score.
image: sentra-cover.jpg
tags:
  - healthcare
  - embedded-systems
  - nextjs
---

> Three accessible sensors. One clear early-warning signal.

---

## Acknowledgements

SENTRA was developed for the MYOSA project challenge using the MYOSA motherboard and accessible I²C sensor modules. The project demonstrates how affordable hardware, thoughtful signal processing, and a clear user interface can make changes in a person's condition easier to notice.

SENTRA is an educational research prototype—not a certified medical device. It does not diagnose sepsis or replace professional medical assessment.

---

## Overview

Sepsis is a time-critical medical emergency. Early deterioration may appear as a combination of physiological and behavioural changes, yet continuous multi-parameter monitoring can be expensive or unavailable outside well-equipped clinical settings.

SENTRA explores a simple question: **Can low-cost, non-invasive sensors combine several weak signals into one understandable prompt for attention?**

The prototype continuously tracks three physiological proxies:

* **Pulse trend** using reflected-light changes from an APDS9960 optical sensor
* **Respiratory trend** using pressure variation captured by a BMP180 sensor
* **Mobility trend** using motion data from an MPU6050 accelerometer

The MYOSA board filters the signals, compares them with configured thresholds or a personal baseline, and assigns one point for each abnormal trend. The resulting score ranges from **0 to 3** and is shown on both the local OLED display and the responsive web dashboard.

SENTRA is designed for students, caregivers, community-health innovators, and researchers exploring affordable early-warning systems. Its goal is not to produce a diagnosis; its goal is to make multi-signal deterioration **visible, explainable, and difficult to overlook**.

**Key features:**

* Three-signal, non-invasive trend monitoring
* Transparent and explainable scoring logic
* Immediate bedside feedback on an OLED display
* Responsive dashboard for clearer visual monitoring
* Low-cost modular hardware with shared I²C wiring
* Baseline-aware mobility analysis

---

## Demo / Examples

### Images

<p align="center">
  <img src="sentra-dashboard.jpg" width="800"><br/>
  <i>SENTRA dashboard presenting live readings, sensor status, and the combined early-warning score</i>
</p>

<p align="center">
  <img src="sentra-hardware.jpg" width="800"><br/>
  <i>MYOSA motherboard connected to the optical, pressure, motion, and OLED modules</i>
</p>

<p align="center">
  <img src="sentra-score.jpg" width="800"><br/>
  <i>Explainable score view showing which signal contributed to the current alert</i>
</p>

### Videos

<video controls width="100%">
  <source src="sentra-demo.mp4" type="video/mp4">
</video>

The demonstration covers sensor placement, live data capture, dashboard updates, and the change in score when one or more monitored trends cross their configured limits.

---

## Features (Detailed)

### 1. Multi-signal monitoring instead of a single alarm

Pulse, breathing, and movement can each change for many reasons. SENTRA does not treat one reading as a diagnosis. It combines three independent trends so the user can see a broader picture while still understanding exactly what caused an alert.

### 2. Experimental non-invasive pulse estimation

The user rests a fingertip lightly over the APDS9960 optical window while shielding it from strong ambient light. Variations in reflected light are sampled, smoothed, and analysed for repeating peaks to produce an experimental beats-per-minute estimate.

The APDS9960 is not a medical pulse sensor, so SENTRA presents this output as a prototype trend rather than a clinical measurement.

### 3. Respiratory-trend tracking

The BMP180 is placed in a soft chest-worn pressure pocket. Expansion and relaxation during breathing create small pressure variations. After smoothing and peak detection, the system estimates a respiratory trend in breaths per minute.

This mechanism is intended to demonstrate low-cost respiratory trend sensing; its readings require calibration and are not equivalent to clinical respiratory monitoring.

### 4. Personal mobility baseline

The MPU6050 is secured to the wrist or torso. The system calculates motion magnitude and short-window variance, then compares current activity with a baseline recorded after startup. A sustained reduction in movement can contribute to the combined score.

Using a personal baseline makes the mobility signal more meaningful than applying the same fixed movement threshold to every user.

### 5. Transparent 0–3 early-warning score

Each abnormal trend contributes one point:

| Signal | Normal state | Score contribution |
| --- | --- | --- |
| Pulse trend | Within configured range | `+1` when outside range |
| Respiratory trend | Within configured range | `+1` when outside range |
| Mobility trend | Near personal baseline | `+1` after sustained decline |

The total is intentionally simple:

| Score | Prototype status | Suggested interface response |
| --- | --- | --- |
| `0` | No monitored trend is abnormal | Continue monitoring |
| `1` | One trend requires attention | Observe and recheck placement |
| `2` | Multiple trends require attention | Show a prominent warning |
| `3` | All monitored trends require attention | Show the highest-priority alert |

The dashboard displays each contributing signal beside the total. There is no hidden model and no unexplained prediction.

### 6. Local display and responsive dashboard

The SSD1306 OLED provides an immediate bedside view of the score and key readings, even without opening another device. The Next.js dashboard expands this into a responsive visual interface with signal cards, status indicators, sensor-placement guidance, and an explainable score summary for phones, tablets, and desktop screens.

### 7. Low-cost shared I²C architecture

All four modules have different I²C addresses, allowing them to share the same SDA and SCL lines without an additional multiplexer:

| Module | Role | I²C address |
| --- | --- | --- |
| APDS9960 | Optical pulse proxy | `0x39` |
| MPU6050 | Mobility sensing | `0x68` |
| BMP180 | Respiratory pressure proxy | `0x77` |
| SSD1306 OLED | Local display | `0x3C` |

This keeps the prototype modular, compact, and straightforward to reproduce.

### 8. Honest and safety-aware design

SENTRA clearly distinguishes experimental proxy measurements from clinical measurements. Sensor status, placement guidance, and score contributors remain visible so users can identify bad contact or unreliable input instead of trusting an unexplained number.

---

## Usage Instructions

### Hardware setup

1. Connect the APDS9960, MPU6050, BMP180, and SSD1306 OLED to the shared I²C bus on the MYOSA motherboard.
2. Place the BMP180 inside the soft chest-worn pressure pocket and secure it comfortably.
3. Secure the MPU6050 to the wrist or torso so the sensor does not move independently from the wearer.
4. Keep the APDS9960 optical window accessible for fingertip sampling and shield it from strong ambient light.
5. Place the MYOSA board and OLED in a safe nearby enclosure.
6. Power the system and remain at a normal activity level while it establishes the initial mobility baseline.
7. Open the dashboard and confirm that all three sensors report an active status.
8. Observe the individual trends and their contribution to the combined score.

### Run the dashboard locally

```bash
git clone https://github.com/diplodoculass/Sentry.git
cd Sentry
npm install
npm run dev
```

Open `http://localhost:3000` in a browser.

### Recommended demonstration sequence

1. Show the complete hardware and explain the purpose of each sensor.
2. Demonstrate correct sensor placement.
3. Show the three individual readings on the dashboard.
4. Trigger a controlled change in one proxy signal and show its score contribution.
5. Show how the OLED and dashboard present the same warning locally and remotely.
6. End by explaining the limitations and the next validation step.

> **Safety note:** SENTRA is for education and prototyping only. Do not use its readings to diagnose, exclude, or treat sepsis. Suspected sepsis requires immediate assessment by qualified medical professionals.

---

## Tech Stack

* **MYOSA motherboard** — sensor sampling, signal processing, and score calculation
* **APDS9960** — reflected-light pulse proxy
* **BMP180** — pressure-based respiratory proxy
* **MPU6050** — accelerometer-based mobility tracking
* **SSD1306 OLED** — local readings and alert display
* **Next.js 16** — web application framework
* **React 19** — component-based dashboard interface
* **TypeScript** — type-safe interface development
* **CSS** — responsive layout and visual design
* **Vercel** — dashboard deployment platform
* **GitHub** — source control and open-source collaboration

---

## Requirements / Installation

### Software requirements

* Node.js 20.9 or newer
* npm 10 or newer
* Git
* A modern web browser

```bash
git clone https://github.com/diplodoculass/Sentry.git
cd Sentry
npm install
npm run dev
```

### Hardware requirements

* 1 × MYOSA motherboard
* 1 × APDS9960 optical proximity and gesture sensor
* 1 × BMP180 pressure sensor
* 1 × MPU6050 accelerometer and gyroscope
* 1 × SSD1306 I²C OLED display
* Jumper wires
* Soft wearable straps and pressure pocket
* Safe non-conductive enclosure
* Suitable power source for the MYOSA board

### Submission media requirements

Place these files in the same folder as this Markdown file:

* `sentra-cover.jpg`
* `sentra-dashboard.jpg`
* `sentra-hardware.jpg`
* `sentra-score.jpg`
* `sentra-demo.mp4`

---

## File Structure

```plaintext
/sentra
  ├── sentra.md
  ├── sentra-cover.jpg
  ├── sentra-dashboard.jpg
  ├── sentra-hardware.jpg
  ├── sentra-score.jpg
  └── sentra-demo.mp4
```

Project source repository: [github.com/diplodoculass/Sentry](https://github.com/diplodoculass/Sentry)

---

## License

SENTRA is released under the MIT License for educational and research use. The software and prototype documentation are provided without medical certification or fitness for clinical use.

---
