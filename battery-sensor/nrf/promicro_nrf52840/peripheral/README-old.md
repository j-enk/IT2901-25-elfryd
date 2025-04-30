Based on `samples/bluetooth/peripheral_hr`

How i build it:
```
cd ~/zephyrproject/zephyr
west build -b promicro_nrf52840/nrf52840/uf2 .
west flash
```

Can also flash manually by moving `build/zephyr/zephyr.uf2` onto the device


# Voltage
Voltage is measured from AIN0 (P0.02), which is defined in the overlay file in
the boards directory. It can be changed to VCC to test without using wires. To
test using a wire connect P0.02 to GND or 3V3, as in the picture.

![](wiring.png.png)

TODO: adjust measurement based on voltage divider.