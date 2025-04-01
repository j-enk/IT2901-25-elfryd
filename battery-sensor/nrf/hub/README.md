# nRF9160 Feather Chip Code with CircuitDojo Tooling

## Prerequisites

Before working with the `hub` code, you need to set up the CircuitDojo development environment. We will focus on setting up the environment on Mac OS in this guide, but you can find instructions for other platforms in the CircuitDojo documentation [here](https://docs.circuitdojo.com/nrf9160-feather/sdk-setup-mac.html) (view the left sidebar for other platforms).

1. Install Visual Studio Code from [here](https://code.visualstudio.com/download)
2. Install the [CircuitDojo Zephyr Tools extension](https://marketplace.visualstudio.com/items?itemName=circuitdojo.zephyr-tools) from the VSCode Marketplace
3. Install required dependencies using Homebrew (Mac OS):
   ```sh
   brew install git python3 ninja ccache
   ```
4. Ensure you have a valid nRF9160 Feather board and a USB-C cable for flashing
5. Obtain a valid `ca.crt` file for TLS connections (see the [Azure Broker README](../azure_broker/README.md) for more information)

## Environment Setup

### 1. Set Up Zephyr Tools
- Open VS Code in a new (empty) folder
- Open the command palette in VSCode (Cmd+Shift+P on Mac)
- Run `Zephyr Tools: Setup` and follow the prompts (use v0.16.4 for the SDK version)

### 2. Initialize the Repository
- Run `Zephyr Tools: Init Repo` from the command palette
- Use the URL: `https://github.com/circuitdojo/nrf9160-feather-examples-and-drivers.git`
- Select an empty folder for initialization

## Integrating Feather Chip Code

1. Within the `hub` project, add the ca.crt file in the `certs` directory (format should be equivalent to a .pem file)
2. Copy the `hub` project from this repo into the `nfed/samples` directory in the circuitdojo setup as a sample

## Building and Flashing

1. Run `Zephyr Tools: Change Project` and select the hub project
2. Run `Zephyr Tools: Change Board` and select `circitdojo_nrf9160_feather_ns` (ns version is required)
3. Run `Zephyr Tools: Build` to compile the code
4. Set up newtmgr for flashing:
   - Run `Zephyr Tools: Setup Newtmgr`
   - Select your device's serial port
   - Select 1000000 as the BAUD rate (important for nRF9160 Feather)
5. Run `Zephyr Tools: Load via Bootloader` to flash the code
6. Run `Zephyr Tools: Serial Monitor` to view the output

## Functionality

This code connects to a configured MQTT broker and publishes messages at different QoS levels every few seconds. 

The application:
- Connects to the MQTT broker using TLS with client certificates
- Publishes periodic telemetry data
- Demonstrates different QoS level handling
- Manages reconnection with the broker if connectivity is lost

To configure the MQTT broker settings, modify the parameters in [prj.conf](./prj.conf).

## Important Notes (Limitations)

- Overlay files can not be configured in the CircuitDojo Zephyr Tools extension. You will need to manually modify the `prj.conf` file to add the `ca_cert` path.
- The CircuitDojo nRF9160 Feather board does not support J-Link debugging. It will not show up as a device with Nordic's VSCode extension as a connected device.
- We can generate the merged.hex file directly through west in the terminal, but we won't be able to flash it using the Zephyr Tools extension (it expects a .bin file). Instead, we need to use the `newtmgr` tool for flashing the device.
-  When trying to flash using the nRF Connect Programmer, the device will show up, but with "no actions available".