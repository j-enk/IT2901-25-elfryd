# Building and Flashing the Elfryd Hub

This guide provides detailed instructions for setting up the development environment and flashing the Elfryd Hub firmware to the nRF9160 Feather board by CircuitDojo. 

We recommend using a Linux or macOS system for this process, as Windows may require additional configuration with drivers. See the [CircuitDojo documentation](https://docs.circuitdojo.com/nrf9160-feather/sdk-setup-windows.html) for more information on debugging on Windows, we will not cover that here.

## Prerequisites

Before working with the `feather` project code, you need to aquire the necessary software and hardware for the CircuitDojo development environment:

1. Install Visual Studio Code from [here](https://code.visualstudio.com/download)
2. Install the [CircuitDojo Zephyr Tools extension](https://marketplace.visualstudio.com/items?itemName=circuitdojo.zephyr-tools) from the VSCode Marketplace
3. Install required dependencies:

   **macOS**:
   ```sh
   brew install git python3 ninja ccache
   ```

   **Linux**:
   ```sh
   sudo apt-get install git python3 cmake ninja-build ccache
   ```

4. Ensure you have a valid nRF9160 Feather board and a USB-C cable for flashing

## Environment Setup

### 1. Set Up Zephyr Tools

- Open VS Code in a new (empty) folder
- Open the command palette in VSCode (Cmd+Shift+P on Mac, Ctrl+Shift+P on Linux)
- Run `Zephyr Tools: Setup` and follow the prompts (use v0.16.4 for the SDK version)

### 2. Initialize the Repository

- Run `Zephyr Tools: Init Repo` from the command palette
- Use the URL: `https://github.com/circuitdojo/nrf9160-feather-examples-and-drivers.git`
- Select an empty folder for initialization

### 3. Integrate the Feather Project as a Sample

- Copy the `feather` project from this repo into the `nfed/samples` directory in the CircuitDojo setup
- Ensure the folder structure is correct:
  ```plaintext
  .../
  ├── nfed/
  │   ├── samples/
  │   │   ├── feather/
  │   │   │   ├── certs/
  │   │   │   ├── scripts/
  │   │   │   ├── src/
  │   │   │   ├── .gitignore
  │   │   │   ├── KConfig
  │   │   │   ├── prj.conf
  │   │   │   └── CMakeLists.txt
  ```

## Certificate Setup

The Elfryd Hub requires TLS certificates for secure MQTT communication:

1. Obtain the CA certificate for your MQTT broker (refer to the [Broker Documentation](../../broker/docs/vm_setup.md))
2. Place the certificate as `ca.crt` in the `certs` directory
3. During the build process, this will be converted to a C header file automatically

## Building and Flashing

Before flashing the firmware, connect the nRF9160 Feather board to your computer using a USB-C cable. The board should be recognized as a serial device.

> Note: There is no JLink support for this board, differing from Nordic's nRF9160 DK, so you won't be able to use JLink tools for flashing.

### 1. Select the Project

- Run `Zephyr Tools: Change Project` and select the hub project
- Run `Zephyr Tools: Change Board` and select `circuitdojo_nrf9160_feather_ns` (ns version is required)

### 2. Build the Firmware

- Run `Zephyr Tools: Build` to compile the code

### 3. Set Up the Flash Tool

- Run `Zephyr Tools: Setup Newtmgr`
- Select your device's serial port when prompted
- Select 1000000 as the BAUD rate (important for nRF9160 Feather)

### 4. Flash the Firmware

- Put the nRF9160 Feather into bootloader mode with the following steps:
  1. Hold the MODE button
  2. While holding the MODE button, press the RESET button
  3. Wait for the blue LED to light up (indicating bootloader mode)
  4. Release the MODE button
- Run `Zephyr Tools: Load via Bootloader` to flash the code

### 5. Monitor Output

- Run `Zephyr Tools: Serial Monitor` to view the output
- You should see boot messages followed by LTE connection and MQTT setup logs
- Note: you can also use the `Zephyr Tools: Load via Bootloader and Monitor` option to flash and monitor in one step

## Troubleshooting

### Unable to Connect to Device

If you cannot connect to the device:
- Make sure the board is in bootloader mode (blue LED on)
- Try using a different USB cable
- Check if you have the correct serial port selected
- Rerun `Zephyr Tools: Setup Newtmgr` to re-initialize the connection

### Build Errors

If you encounter build errors:
- Verify you have all dependencies installed
- Check that the `ca.crt` file is in the correct format
- Ensure the board selection is correct (`circuitdojo_nrf9160_feather_ns`)

### MQTT Connection Issues

If the device cannot connect to MQTT:
- Verify the MQTT broker settings in `prj.conf`
- Check that the certificate is correct and matches the broker's CA
- Ensure the device has cellular coverage

## Advanced Configuration

For more advanced configuration, edit the following files:

- `prj.conf`: Main configuration file with Kconfig options
- `Kconfig`: Custom Kconfig options specific to the Elfryd Hub
- `CMakeLists.txt`: Build system configuration

## Resources

- [CircuitDojo nRF9160 Feather Documentation](https://docs.circuitdojo.com/nrf9160-feather/getting-started.html)
- [Zephyr Project Documentation](https://docs.zephyrproject.org/latest/)