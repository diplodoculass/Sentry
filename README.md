---
publishDate: 2026-08-26T00:00:00Z

title: SENTRA — Sepsis Early Non-invasive Tracking & Risk Assessment

excerpt: A low-cost, non-invasive prototype that combines pulse, breathing, and mobility trends into a simple bedside early-warning score.

image: sentra-cover.jpg

tags:
  - healthcare
  - embedded-systems
  - nextjs
---

> Three accessible sensors. One clearer early-warning signal.

---

## Acknowledgements

SENTRA was developed for the MYOSA project challenge using the MYOSA motherboard and commonly available I²C sensor modules. The project is an educational prototype and is not a certified medical device.

## Overview

Sepsis can progress quickly, and visible deterioration may appear after important physiological changes have already begun. SENTRA explores whether inexpensive, non-invasive hardware can make those changes easier to notice.

The prototype continuously observes three proxy signals:

* **Pulse rate** using the APDS9960 optical proximity sensor
* **Respiratory rate** using pressure variation from the BMP180
* **Mobility trend** using the MPU6050 accelerometer

The MYOSA board filters these signals, compares them with simple thresholds or a personal baseline, and produces an understandable risk score from 0 to 3. Current readings and warnings can be shown on the local OLED screen and on the web dashboard.

SENTRA is intended for demonstrations and early research into accessible bedside monitoring. It does not diagnose sepsis and should not be used to make medical decisions.

## Demo / Examples

### Images

<!-- TODO: Add the required JPG files before submitting. -->

<p align="center">
  <img src="/assets/images/sentra/sentra-dashboard.jpg" width="800"><br/>
  <i>SENTRA live monitoring dashboard</i>
</p>

<p align="center">
  <img src="/assets/images/sentra/sentra-hardware.jpg" width="800"><br/>
  <i>MYOSA board and sensor prototype</i>
</p>

### Videos

<!-- TODO: Add your local MP4 file. YouTube links are not accepted by MYOSA. -->

<video controls width="100%">
  <source src="/sentra-demo.mp4" type="video/mp4">
</video>

### Live Website

<!-- TODO: Replace this placeholder after deploying to Vercel. -->

**Deployment:** `https://your-sentra-project.vercel.app`

## Features (Detailed)

### 1. Non-invasive pulse estimate

The user rests a fingertip over the APDS9960 optical window. Changes in reflected light are sampled as a rough photoplethysmography signal, filtered, and used to estimate beats per minute. This is suitable for a prototype demonstration but is less reliable than a dedicated pulse sensor.

### 2. Respiratory-rate tracking

The BMP180 is secured flat against the upper chest inside a soft strap. Small pressure changes produced by the breathing cycle are processed to estimate breaths per minute.

### 3. Mobility baseline

The MPU6050 is worn firmly on the wrist or torso. Accelerometer variance is compared with an observed baseline to identify a meaningful reduction in movement.

### 4. Transparent risk score

Each abnormal proxy signal contributes one point to a score from 0 to 3. The dashboard clearly shows the readings that contributed to the score rather than presenting an unexplained prediction.

### 5. Local and web display

The SSD1306 OLED provides an immediate bedside view. The responsive Next.js interface presents live-looking telemetry, hardware placement, sensor status, and the risk summary on phones, tablets, and desktop screens..

### 6. Simple shared wiring

All four modules use unique I²C addresses and can share the same SDA and SCL lines without a multiplexer:

| Module | Role | I²C address |
| --- | --- | --- |
| APDS9960 | Pulse proxy | `0x39` |
| MPU6050 | Mobility | `0x68` |
| BMP180 | Respiration proxy | `0x77` |
| SSD1306 OLED | Local display | `0x3C` |

## Usage Instructions

1. Secure the BMP180 flat against the upper chest in a soft strap.
2. Secure the MPU6050 to the wrist or torso so it does not move independently.
3. Rest an index fingertip lightly over the APDS9960 during pulse sampling and shield it from strong ambient light.
4. Keep the MYOSA board and OLED in a nearby enclosure.
5. Power the board and allow the firmware to establish a movement baseline.
6. Open the web dashboard to view the three signals and the current proxy score.

To run the dashboard locally:

```bash
npm install
npm run dev
```

Then open `http://localhost:3000`.

## Tech Stack

* **MYOSA motherboard** — sensor sampling and local processing
* **BMP180** — pressure-based respiratory proxy
* **MPU6050** — accelerometer-based mobility tracking
* **APDS9960** — optical pulse proxy
* **SSD1306 OLED** — local status display
* **Next.js 16 and React 19** — responsive web dashboard
* **TypeScript and CSS** — interface implementation
* **Vercel** — web deployment

## Requirements / Installation

### Dashboard

* Node.js 20.9 or newer
* npm 10 or newer

```bash
git clone https://github.com/your-username/sentra.git
cd sentra
npm install
npm run dev
```

### Hardware

* 1 × MYOSA motherboard
* 1 × BMP180 pressure sensor
* 1 × MPU6050 accelerometer/gyroscope
* 1 × APDS9960 optical sensor
* 1 × SSD1306 I²C OLED screen
* Jumper wires, wearable straps, and a safe enclosure

## File Structure

```plaintext
/sentra
  ├── app/
  │   ├── globals.css
  │   ├── layout.tsx
  │   └── page.tsx
  ├── public/
  │   ├── sentra-cover.jpg       # add before submission
  │   └── sentra-demo.mp4        # add before submission
  ├── package.json
  ├── tsconfig.json
  └── README.md
```

## License

This project is available under the MIT License. Medical use is expressly excluded; SENTRA is an educational prototype only.

## Contribution Notes

Contributions are welcome for improved filtering, real hardware transport, accessibility, and calibration. Please document the hardware used and clearly distinguish measured results from simulated dashboard data.
