# ADAS Autonomous Vehicle 🚗

A cost-effective Advanced Driver Assistance System (ADAS) prototype 
built using Raspberry Pi and Arduino, integrating sensor-based control 
with machine learning for real-world vehicle safety features.

---

## Features

| Feature | Method | Accuracy |

| Automatic Emergency Braking | HC-SR04 Ultrasonic Sensor | Stops within 25cm avg |

| Lane Tracking & Assist | 4× TCRT5000 IR Sensors | 95% under good lighting |

| Pedestrian Detection | YOLOv8 CNN (camera) | ~88% up to 3 meters |

| Traffic Light Recognition | Color segmentation + ML | ~92% for clear signals |

---

## System Architecture

A dual-processor design splits responsibilities:

- **Arduino Uno** → Real-time control: motor driving, ultrasonic 
  braking, IR lane tracking
- **Raspberry Pi 5** → ML inference: pedestrian detection, 
  traffic light recognition via camera
- Communication between both via **UART serial at 9600 baud**

---

## Hardware Used

- Raspberry Pi 5 (4GB RAM, Broadcom BCM2712)
- Arduino Uno (16 MHz)
- HC-SR04 Ultrasonic Sensor (obstacle detection, 2–400cm range)
- 4× TCRT5000 IR Reflective Sensors (lane tracking)
- Raspberry Pi Camera Module V2 (8MP Sony IMX219, 1080p/30fps)
- L298N Dual H-Bridge Motor Driver
- 6V Geared DC Motors
- 12V Regulated Power Supply

**Total build cost: ₹28,000**

---

## Tech Stack

- **Arduino C** — sensor interfacing, motor control, real-time actuation
- **Python** — ML inference on Raspberry Pi
- **OpenCV** — image processing, color segmentation
- **YOLOv8** — pedestrian detection (CNN-based)
- **TensorFlow** — traffic light classifier
- **UART Serial** — Arduino ↔ Raspberry Pi communication

---

---

## Key Results

- Integrated system achieved **90% success rate** across all modules running simultaneously
- Vision processing: ~150ms per frame → **6–7 fps** on Raspberry Pi
- Lane centering accuracy: **±5cm** on straight, **±8cm** on curved lanes
- Built entirely within a **₹28,000 budget** — proving ADAS can be democratized

---

## Acknowledgements

Funded by the **Internally Funded Student Project (IFSP)** scheme, 
SSN Trust. Built at the Department of Electrical and Electronics 
Engineering, SSN College of Engineering, Chennai.

**Team:** Anandha Laxmi Senthilkumar, R.S. Pooranaa, Rohith Kumar V  
**Advisor:** Dr. Sajjan Kumar
