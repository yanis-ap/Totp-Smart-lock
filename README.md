# 🔐 TOTP Smart Lock

A secure, Arduino-based electromagnetic lock system powered by **Time-based One-Time Passwords (TOTP)** — designed for multi-user access in shared lab environments.

## Introduction

This is a mini-project taken up by me & a few of my colleagues while we were in the Automation & Robotics Club in college. The objective of this project is clear, Our club had a lab known as Tinkerer’s Lab which was also being used by members from other technical clubs. For unbeknownst reasons, the lab’s lock was broken, so we could no longer use the labroom to store our technical equipments. So a few of us took up this issue and decided to build a totp door lock, we built our own system tailored to support **daily-resetting access codes** for different clubs.

## Key Features

- **5 daily TOTP codes** generated using HMAC-SHA1, valid for 24 hours each
- **Secure key sharing**: Club leaders hold a specefic seed - that keeps generating code each 24hrs, they share only current-day code when a member needs access
- **Access logging** with timestamps to `logs.txt` on SD card
- **Reliable timekeeping** using DS3231 RTC (keeps sync during power loss)
- **Offline operation** — no internet or server needed
### Note:
- This project is a proof of concept prototype so we don't have the relay ouput in the code. In actual functioning the 5V relay will be hooked upto the door lock along with required power supply 12/6 V as required for the EM lock. Also note that the arduino doesn't have enough power to provide to all the components. So an external power supply would be needed. We used an external power supply of 5V and reinforced the voltage rail on the breadboard for proper functioning.
- As seen in the circuit diagram, we have used a voltage divider to connect the keypad as it reduces the amount of pins used on the arduino. For reference check out Hari Wiguna's video on Youtube https://www.youtube.com/watch?v=G14tREsVqz0

## Libraries used

- **Totp.h & Sha1.h**: This system is based on lucadentella's TOTP-Arduino Library (https://github.com/lucadentella/TOTP-Arduino)
- **DS3231.h**: Uses a third party Rinky-dink Electronics DS3231 library (http://www.rinkydinkelectronics.com/library.php?id=73)
- **LiquidCrystal_I2C.h**: LCD I2C display driver (https://github.com/johnrickman/LiquidCrystal_I2C)
- **SD.h**: Built-in library for SD module operation
- **wire.h**: Two wire I2C communication

## Hardware

- Arduino Uno
- DS3231 RTC Module + battery
- 4×4 Matrix Keypad
- I²C 16×2 LCD
- MicroSD card module + SD card
- Electromagnetic lock + 5V relay module
- Power Supply, Resistors, Capacitors, Breadboard

## How It Works

1. At boot, HMAC keys are read from `hmacKeys.bin` on the SD card.
2. RTC provides the current Unix time.
3. Five TOTP codes are generated based on 24-hour periods (one per seed).
4. User enters a 6-digit code on the keypad.
5. If matched, the lock opens and access is logged.
6. Old logs are cleared automatically after a set interval.

## Getting Started

1. Upload `TOTP_Lock.ino` to Arduino.
2. Place `hmacKeys.bin` (5×10-byte HMAC keys) on SD card.
3. Follow the circuit diagram as shown in image
4. Power on — LCD prompts for password.
5. Match the correct code? Green LED lights up for 5sec indicating correct password, log saved.

## Future-works / Todo

1. port this system into much faster, smaller microcontroller - Seeed studio xiao.
2. Check the time-drift on the RTC, & make sure it doesn't affect the TOTP functioning
