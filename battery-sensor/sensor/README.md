SAADC Battery Monitor
==========================

Overview
--------
This project currently reads a voltage source through a voltage divider using the ADC on the nRF52480DK board with Zephyr RTOS. The code reads the ADC, converts the raw value to millivolts, and compensates for the voltage divider $(R1 = R2)$.

Project Structure
-----------------
- **CMakeLists.txt**  
  Top-level CMake configuration for the project.

- **prj.conf**  
  Zephyr project configuration file with build and runtime settings.

- **boards/**  
  Contains board-specific device tree overlays.
  - **nrf52480dk.overlay**: Overlay file for the nRF52480DK board.

- **src/**  
  Source code directory:
  - **main.c**: Main application logic.
  - **adc_driver.c**: Implementation of ADC driver functionality.
  - **adc_driver.h**: Header file declaring the ADC driver API.

- **README.md**  
  This documentation file.
