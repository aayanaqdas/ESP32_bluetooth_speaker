# ESP32 Bluetooth Speaker

## Project Description

This project is a Bluetooth speaker built with an ESP32 microcontroller. You pair it to your phone just like a normal Bluetooth speaker, and it plays music through a small speaker using an audio amplifier. It has a TFT screen that shows the current content title, artist/channel, and volume level, with a scrolling effect for long titles. It also has three physical buttons so you can pause or skip tracks without touching your phone, plus a potentiometer to control the volume. This is only tested on Iphone so i cant guarantee it will work with every device.


## Demo
See the speaker working: [Watch the YouTube Demo](https://youtube.com/shorts/CwvdIrRxzno?si=EUpTrKxraHuvu111)


## Components used
*   **ESP32 DevKit V1**
*   **MAX98357A Amplifier Module**
*   **1.8" TFT SPI Display (ST7735)**
*   **10k Potentiometer**
*   **3x Push Buttons**
*   **3W 8Ω Mini Speaker**
*   **Blue LED (with Resistor)**
*   **Jumper Wires**

---

## Wiring and Connections

### Power Connections

| Component / Pin | Power Source | Notes |
| :--- | :--- | :--- |
| **Potentiometer VCC** | `3.3V` | Use 3.3V to ensure the analog signal doesnt exceed the ESP32s 3.3V ADC limit. |
| **TFT Display VCC** | `3.3V` | Primary power for the display logic. |
| **TFT Display LED (BL)** | `3.3V` | Backlight power for the display. |
| **MAX98357A VCC/VIN** | `5V`  | Needs 5V for optimal speaker amplification footprint. |
| **Blue LED (Anode)** | `3.3V` | Power via ESP pin. Use a current-limiting resistor. |
| **All Ground Connections** | `GND` | Tie all component GNDs to the ESP32 GND. |

### Data and Signal Pin Mapping

#### TFT Display (ST7735)
| TFT Pin | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| **CS** | `GPIO 5` | Chip Select |
| **DC (A0)** | `GPIO 2` | Data/Command |
| **RST** | `GPIO 4` | Reset |
| **SCK** | `GPIO 18` | Hardware VSPI Clock |
| **SDA (MOSI)** | `GPIO 23` | Hardware VSPI Data |

#### Audio Amplifier (MAX98357A)
| MAX98357A Pin | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| **LRC** | `GPIO 26` | Left/Right Word Select framing Clock |
| **BCLK** | `GPIO 27` | Bit Clock |
| **DIN** | `GPIO 14` | Digital Audio Data In |
| **Speaker + / -** | N/A | Connect directly to the speaker |

#### Controls and Peripherals
| Component | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| **Button: Next** | `GPIO 25` | Connect the other side of the button to GND (Uses internal pull-up) |
| **Button: Previous** | `GPIO 32` | Connect the other side of the button to GND (Uses internal pull-up) |
| **Button: Play/Pause**| `GPIO 33` | Connect the other side of the button to GND (Uses internal pull-up) |
| **Potentiometer**| `GPIO 34` | Center pin of the pot (ADC input for volume reading) |
| **Blue Status LED** | `GPIO 13` | Controls connection status blinking |

---

## Libraries
To compile this project, the following libraries are required via PlatformIO:
*   [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) by pschatzmann (A2DP Sink and AVRCP Metadata)
*   [arduino-audio-tools](https://github.com/pschatzmann/arduino-audio-tools) by pschatzmann (Audio streaming framework)
*   [Adafruit ST7735 and ST7789 Library](https://github.com/adafruit/Adafruit-ST7735-Library) (Display Driver)
*   [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) (Graphics rendering and layout)