* Postive is never 100% normalized, but only 99%

* Add some check to configuration, and have transmitter fail to startup and beep errors if it is wrong.
    *Still, programming box should function so that the user can recover
    * We can achieve this by storing a dummy configuration that outputs all channels as 0 (fixed mixer), on certain channels with a "invalid" model identifier. The PB would recognize this invalid configuration.
    * This way we don't have to have all modules that access config have to check whether the configuration is valid.

* Dynamic input configuration based on config
* Support for switch, pushbutton, etc