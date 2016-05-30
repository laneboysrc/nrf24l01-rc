* Binding only during the first 10 seconds after power on
    - This is necessary as the AliExpress nRF module outputs a lot of power
      even at -18dBm.
* LED
    - Always-on when powered on
    - Blinking when battery is low
* Battery voltage measurement and low battery alarm
    - Calibrate the ADC measurement (2k2 over 3k3 voltage divider)
    - How often to measure the battery voltage?
    - Additional smoothing required?
    - Log the voltage drop over time using a real battery setup
    - Regular beeps when battery is low
* Find more "music"