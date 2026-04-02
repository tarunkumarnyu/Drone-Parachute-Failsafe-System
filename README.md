# Drone Failsafe Parachute System

![Parallax Propeller](https://img.shields.io/badge/Parallax-Propeller-blue)
![IMU](https://img.shields.io/badge/Sensor-MPU6050-green)
![Ultrasonic](https://img.shields.io/badge/Sensor-HC--SR04-orange)
![SolidWorks](https://img.shields.io/badge/CAD-SolidWorks-red)
![License](https://img.shields.io/badge/License-MIT-yellow)

An emergency recovery system for UAVs that detects in-flight failures and autonomously deploys a parachute and retractable landing gear for controlled descent — no pilot intervention required.

Built for the Mechatronics course (ROB-GY 5103) at NYU Tandon School of Engineering.

## Demo

<p align="center">
  <img src="assets/prototype-top.png" width="45%" alt="Prototype Top View"/>
  <img src="assets/prototype-side.png" width="45%" alt="Prototype Side View"/>
</p>

## How It Works

**Stage 1 — Failure Detection**

1. IMU (MPU6050) continuously monitors drone orientation and acceleration
2. Detects sudden instability, unusual tilting, motor failure, or ESC burnout
3. Triggers emergency sequence when imbalance exceeds safety threshold

**Stage 2 — Parachute Deployment**

1. Two servo motors (MG995) actuate topology-optimized plates on top of the drone
2. Plates rotate 90° to release the parachute
3. Parachute slows descent speed significantly

**Stage 3 — Landing Gear Deployment**

1. Ultrasonic sensors (HC-SR04) continuously measure ground distance
2. At critical altitude (200 cm), four servo motors deploy retractable landing gear
3. Spring-suspended 3D-printed gear absorbs remaining impact force

## System Architecture

```
┌─────────────────────────────────────────────┐
│              Parallax Propeller              │
│                (Main Controller)             │
├──────────┬──────────┬───────────────────────┤
│  P0/P1   │    P3    │     P4        P5      │
│  (I2C)   │  (GPIO)  │   (PWM)     (PWM)     │
│    │      │    │     │     │         │       │
│  ┌─┴─┐  ┌─┴──┐  ┌──┴──┐  ┌──┴──┐          │
│  │IMU│  │Ultra│  │4x   │  │2x   │          │
│  │6050│  │sonic│  │Servo│  │Servo│          │
│  └───┘  └────┘  │(Gear)│  │(Chute)│         │
│                  └─────┘  └──────┘          │
└─────────────────────────────────────────────┘
         ↑ 5V from Buck Converter
         ↑ 8.4V LiPo Battery
```

## Project Structure

```
├── firmware/
│   ├── main.spin              # Main control loop (Propeller SPIN)
│   ├── imu_driver.spin        # MPU6050 I2C interface
│   ├── ultrasonic.spin        # HC-SR04 distance sensing
│   └── servo_control.spin     # Servo actuation logic
├── cad/
│   ├── parachute_assembly/    # Topology-optimized plate mechanism
│   ├── drone_frame/           # Custom drone frame (PLA-CF)
│   └── landing_gear/          # Spring-suspended retractable gear
├── docs/
│   └── circuit_diagram.png    # Full wiring schematic
├── assets/                    # Images and demo videos
└── README.md
```

## Components

| Component | Model | Qty | Purpose |
|-----------|-------|-----|---------|
| Microcontroller | Parallax Propeller | 1 | Real-time multi-cog processing |
| IMU | MPU6050 | 1 | Instability detection |
| Ultrasonic Sensor | HC-SR04 | 1 | Ground proximity measurement |
| Servo Motor | MG995 | 6 | Parachute (2) + Landing gear (4) |
| Battery | 8.4V LiPo | 1 | Main power supply |
| Buck Converter | — | 1 | 8.4V → 5V regulation |
| Drone Components | Frame, FC, ESC, Motors | — | Flight platform |

**Total Estimated Cost: ~$332**

## Circuit Connections

| Peripheral | Pin | Protocol |
|-----------|-----|----------|
| IMU (SDA/SCL) | P0, P1 | I2C |
| Ultrasonic (Trig/Echo) | P3 | GPIO |
| Landing Gear Servos (x4) | P4 | PWM |
| Parachute Servos (x2) | P5 | PWM |

## Key Design Decisions

- **Dual IMU redundancy** — External IMU cross-referenced with flight controller IMU eliminates single-point-of-failure in crash detection
- **Mars-Rover-inspired descent sequencing** — Multi-stage deployment (detect → chute → gear) mirrors proven planetary landing strategies
- **Topology-optimized plates** — Reduced weight while maintaining structural integrity; minimizes drag from propeller airflow
- **PLA-CF frame** — Carbon-fiber-filled PLA for high strength-to-weight ratio on custom 3D-printed components
- **Spring-suspended landing gear** — Absorbs residual impact force that the parachute alone cannot eliminate

## Results

- Prototype successfully detects imbalance and deploys parachute autonomously
- Controlled tests show significant reduction in landing impact force
- Landing gear deployment triggers reliably at 200 cm altitude threshold
- System responds to simulated motor failure, ESC burnout, and signal loss scenarios

## Future Work

- AI-based predictive stability control for preemptive deployment
- Machine learning models to optimize deployment timing based on environmental conditions
- Deployable air cushions and shock-absorbing retractable gear
- Biodegradable parachute materials for sustainability
- Integration with commercial drone platforms for regulatory compliance

## Stack

`Parallax Propeller (SPIN)` · `MPU6050 IMU` · `HC-SR04 Ultrasonic` · `MG995 Servos` · `SolidWorks` · `3D Printing (PLA-CF)` · `LiPo Power System`

## Team

Tarunkumar Palanivelan · Sven Sunny · Abirami Palaniappan · Sirsabesan

## License

This project is licensed under the [MIT License](LICENSE).
