Based on `samples/bluetooth/peripheral_hr`

How i build it:
```
cd ~/zephyrproject/zephyr
west build -b promicro_nrf52840/nrf52840/uf2 .
west flash
```

Can also flash manually by moving `build/zephyr/zephyr.uf2` onto the device