# IT2901 -  nRF91 Digital Lockbox

## Getting started

The lockbox system consists of two parts, the MQTT broker and the nRF9160 lockbox client.

Firstly set up the MQTT broker as descibed in [Setting up MQTT Broker on an Azure Virtual Machine](./MQTT_Broker.md)

At this point you should have the CA.crt file generated in the virtual machine on your local machine. You should also have MQTT credentials for your the nRF9160 saved somewhere.

After you have everything you need you can set up the lockbox client as described in [Setting up the lockbox client on a nRF9160
](./nRF9160_client.md)

## Documentation
The code on the nRF9160 is built from samples provided as part of the nRF Connect SDK. The sample is called Simple MQTT and is provided freely through the nRF Connect SDK. [nRF9160: Simple MQTT](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/0.4.0/nrf/samples/nrf9160/mqtt_simple/README.html)

It is also extended with the [NFC: Tag reader](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/samples/nfc/tag_reader/README.html) example to scan the NFC tag. 


### Codebase
The codebase can be split into 4 distinct sections:
#### MQTT

The MQTT code can mostly be found under _mqtt_helpers.c_ however some setup code can be found in _main.c_. Most of the code is from the _Simple MQTT_ sample. 

The setup processes starts with importing the brokers CA certificate, to enable TLS communication. Then main.c creates a MQTT client which subscribes to the the defined topic, as well as be able to publish on demand. 

There are 4 commands that the client can receive from the broker. These are defined as
```c
#define CMD_OPEN "OPEN"
#define CMD_KEY_STATUS "KEY_STATUS"
#define CMD_PING "PING"
#define CMD_LOCK_STATUS "LOCK_STATUS"
```

Whenever the client gets a new mqtt event it goes to
```c

void mqtt_evt_handler(struct mqtt_client *const c, 
                      const struct mqtt_evt *evt)
```
The event handler verifies the command type and calls the appropriate handler. An example of a command handler is the _CMD_PING_ handler. It is defined as

```c
int pong()
{
    LOG_INF("Ponging...");

    //Creates the data to be published
    char data[15];
    sprintf(data, "%d?PONG\0", CONFIG_LOCKBOX_ID);

    //Publishes the data to the broker
    int ret = data_publish(&client, 
                           MQTT_QOS_1_AT_LEAST_ONCE,
                           data,
                           strlen(data));
    if (ret)
    {
        LOG_ERR("Failed to pong: %d", ret);
    }
    else
    {
        LOG_INF("Pong sent");
    }

    return ret;
}
```

#### NFC
The NFC code can be found in the _nfc_helpers.c_ file. It is based on the _NFC: Tag reader_ example. The code is responsible for scanning the NFC tag and creating NFC events. It runs in a separate thread from the MQTT client and is started in the _main.c_ file.

#### Event Manager
Since the MQTT client and the NFC reader are running in different threads, there exists an [Application Event Manager](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/others/app_event_manager.html) to handle communication between the threads. By using this system we could easily expand the system to include more sensors and events. And it also made it easier to handle communication between the threads.

The event manager system consists of events and event listeners. There is defined three types of events.
##### NFC events

NFC events are created whenever the reader detects that the tag has beed placed back or removed. It is defined as

``` c
struct nfc_event 
{
    struct app_event_header header;
    bool is_present;
};
```

where the _is_present_ variable is wheter the tag is present or not.

##### Lock detect events

Lock detect events are created whenever the nRF9160 detects that the doot has been opened or closed. It is defined as

```c
struct lock_detect_event 
{
    struct app_event_header header;
    bool is_pressed;
};
```
where the _is_pressed_ variable is wheter the door is open or closed.

##### Publish events

Publish events are subscribed to both NFC and lock detect events and are created whenever one of these events are created. It publishes the event to the MQTT broker. It is defined as

```c
struct publish_event
{
    struct app_event_header header;
    char *data;
};
```

where the _data_ variable is the data to be published to the MQTT broker.

#### GPIO

The gpio code found in the _gpio_utils.c_ file, and has helper functions for reading button state, and turning on and off the LED. The button state reader is used to detect when the door is opened or closed. The LED is used to indicate when the door is locked or unlocked. It is also responsible for creating the lock detect events.