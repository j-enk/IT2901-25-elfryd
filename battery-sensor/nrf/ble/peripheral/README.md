Based on `samples/bluetooth/peripheral_hr`

How i build it:
```
cd ~/zephyrproject/zephyr
west build -b promicro_nrf52840/nrf52840/uf2 /Users/jole/pro/IT2901-25-elfryd/battery-sensor/nrf/ble/peripheral
west flash
```

Can also flash manually by moving `build/zephyr/zephyr.uf2` onto the device