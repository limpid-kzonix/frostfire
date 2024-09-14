# Frostfire

Frostfire is an ESP32-based project that allows you to remotely control the power switch of your PC via a relay module. The goal is to provide an easy way to power on and off your PC from a distance, offering convenience for setups where the PC is not easily accessible.
Features

    Remote PC Power Control: Turn your PC on or off remotely.
    ESP32 with Arduino Framework: Built using the ESP32 microcontroller, programmed with the Arduino framework.
    Relay Module Control: Uses a relay to physically switch the PC's power pins.
    Wi-Fi Connectivity: Control your PC power through Wi-Fi from any device.

Components

    ESP32: The main microcontroller responsible for managing the power signals.
    Relay Module: Used to switch the power button pins of your PC.
    Jumper wires: To connect the ESP32 to the relay.
    Power Supply: Ensure that the ESP32 is powered either via USB or external supply.

Setup
Hardware

    Connect the relay module to the ESP32.
        VCC of relay to 3.3V of ESP32.
        GND of relay to GND of ESP32.
        IN1 of relay to a digital pin (e.g., D5) of ESP32.

    Connect the relay to your PC’s power button pins on the motherboard.

    Power the ESP32 via USB or an external 5V power supply.

Software

    Install the Arduino IDE and set up the ESP32 board.
        Add the ESP32 board in the Arduino IDE by following these instructions.

    Clone this repository:

    bash

    git clone https://github.com/yourusername/Frostfire.git

    Open Frostfire.ino in the Arduino IDE and configure your Wi-Fi credentials in the code.

    Upload the code to your ESP32.

Usage

    Ensure that the ESP32 is powered and connected to your Wi-Fi network.
    Use a web interface, mobile app, or custom script to send commands to the ESP32 to toggle the relay and control your PC power.
    Your PC can now be turned on or off remotely!

Future Enhancements

    Integration with smart home systems (Home Assistant, etc.)
    Mobile app for easier control.
    Advanced scheduling for automatic power control.

License

This project is licensed under the MIT License - see the LICENSE file for details.
