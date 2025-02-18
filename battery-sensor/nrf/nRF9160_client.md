# Setting up the lockbox client on a nRF9160

## Set up the toolsystem
To flash the nRF9160 you need some tools on your local machine. 

Firstly install the [nRF Connect for Desktop](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Connect-for-desktop).

When the program is intalled, and started. Install the following apps
- Tool Manager and latest SDK
- Programmer
- LTE Link Monitor.

## Set up nRF9160 DK

Follow this [Getting Started with nRF9160 DK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/working_with_nrf/nrf91/nrf9160_gs.html) from Nordic Semiconductor. It will guide you to update the kits firmware and setup the SIM-card.


## Configuring the NRF9160 Code

Before starting you need to have completed these things
- Set up MQTT Broker
- Have ca.crt and MQTT credentials ready.
- Development kit should have latest firmware
- IoT SIM-card should be active.

To connect to your new broker with the NRF9160 chip, you need to configure a few settings. 

1. Open the NRF9160 MQTT code with Visual Studio Code. 
2. Open the prj.conf file. 
3. Update these properties: 

```
CONFIG_MQTT_PUB_TOPIC= (Publish topic)
CONFIG_MQTT_SUB_TOPIC= (Subscribe topic)
CONFIG_MQTT_CLIENT_ID= (MQTT identifier)
CONFIG_MQTT_BROKER_HOSTNAME= (Broker URL)
CONFIG_MQTT_BROKER_PORT= (Broker port)
CONFIG_MQTT_BROKER_USERNAME= (MQTT username)
CONFIG_MQTT_BROKER_PASSWORD= (MQTT password)
CONFIG_LOCKBOX_ID= (Lockbox ID, as in database)

CONFIG_NFC_TAG_ID= (ID of NFC tag, can be found using mobile app)
 ```

*Username and Password must be from the /etc/mosquitto/passwd file.*


4. Open the **CA.crt** file you downloaded to your computer earlier, and copy the certificate inside into the **certificates.h** file in the src folder. You will need to edit the certificate so it conforms to  the **certificates.h** layout. 

5. In the VS-code extension panel, use **Pristine Build** to create the new program
6. Flash the new program onto the NRF9160 Chip. 
8. The chip should now be connected to your new broker. You can monitor the chip by using the LTE link monitor, installed earlier.
