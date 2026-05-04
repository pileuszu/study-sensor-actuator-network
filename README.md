# 📡 Smart Sensor-Actuator Network (SSAN)

![C](https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c)
![Platform](https://img.shields.io/badge/Platform-Raspberry%20Pi-C51A4A?style=flat-square&logo=raspberry-pi)
![Linux](https://img.shields.io/badge/OS-Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)

A robust, real-time distributed system for sensor monitoring and actuator control using Raspberry Pi. This project implements a client-server architecture over TCP/IP to manage environmental data and trigger mechanical responses.

---

## 🚀 Key Features

- **Real-time Monitoring**: High-precision distance measurement and environmental sensing.
- **Distributed Control**: Decoupled sensor (server) and actuator (client) logic via TCP/IP sockets.
- **Multi-threaded Processing**: Concurrent sensor reading using POSIX threads.
- **Hardware Integration**: Direct sysfs-based GPIO and PWM control for motors and pumps.
- **Extensible Architecture**: Modular utility headers for networking, GPIO, and PWM.

---

## 🏗️ System Architecture

The system is divided into two primary subsystems, each following a **Producer-Consumer (Server-Client)** pattern:

1.  **Ultrasonic Distance Subsystem**
    *   **Server**: Samples dual ultrasonic sensors (HC-SR04) and broadcasts distance data.
    *   **Client**: Receives data and adjusts motor speed/position via PWM.
2.  **Water & Motion Subsystem**
    *   **Server**: Monitors water level and PIR motion sensors.
    *   **Client**: Triggers a water pump or auxiliary outputs based on environmental triggers.

---

## 📂 Project Structure

| File | Description |
| :--- | :--- |
| `ultra_server.c` / `ultra_client.c` | Ultrasonic sensing and motor control logic. |
| `water_server.c` / `water_client.c` | Water detection and pump control logic. |
| `gpio_utils.h` | Low-level sysfs GPIO mapping and control. |
| `network_utils.h` | Socket abstraction for TCP/IP communication. |
| `pwm_utils.h` | Pulse Width Modulation control for servos/motors. |
| `Makefile` | Build automation script. |

---

## 🔌 Hardware Configuration

### 1. Ultrasonic System
| Component | Pin (BCM) | Role |
| :--- | :--- | :--- |
| **Sensor 1** | Trig: 23 / Echo: 24 | Distance Input |
| **Sensor 2** | Trig: 5 / Echo: 6 | Distance Input |
| **Button** | 20 | Manual Override (Client) |
| **Motor** | 21 (PWM) | Actuator (Client) |

### 2. Water & Motion System
| Component | Pin (BCM) | Role |
| :--- | :--- | :--- |
| **Water Sensor** | 23 | Level Input |
| **Motion Sensor**| 26 | PIR Input |
| **Pump** | 21 | Actuator (Client) |
| **Aux Output** | 24 | Indicator (Client) |

---

## 🛠️ Build & Installation

Ensure you have `gcc` and `make` installed on your Raspberry Pi.

1. **Clone the repository**:
   ```bash
   git clone https://github.com/pileuszu/study-sensor-actuator-network.git
   cd study-sensor-actuator-network
   ```

2. **Compile the project**:
   ```bash
   make
   ```

---

## 🖥️ Usage

### Ultrasonic Distance System
1. **Start Server** (on sensor node):
   ```bash
   ./ultra_server 9090
   ```
2. **Start Client** (on actuator node):
   ```bash
   ./ultra_client <SERVER_IP> 9090
   ```

### Water & Motion System
1. **Start Server** (on sensor node):
   ```bash
   ./water_server 9091
   ```
2. **Start Client** (on actuator node):
   ```bash
   ./water_client <SERVER_IP> 9091
   ```

---

## 📜 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.