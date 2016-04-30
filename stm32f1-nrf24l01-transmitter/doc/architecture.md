# STM32F1 NRF24L01+ Headless Transmitter Architecture

* 20 hop channels
* 5 byte address
* 3 servo channels (Be able to extend to more channels; requires protocol change!)
* Stick data sent every 5 ms
* Hop frequency changed every 5 ms
* Failsafe sent


## START SIMPLE

* Basic 3 and 4 channel Tx; output to 3 channel receiver
* Reversing
* EPA
* Sub-trim
* Expo
* Battery state


## Headless transmitter (Tx) and Programming Box (PB)

* The PB allows live configuration of (sub)-trim values, end points, etc
* The PB can display the live stick position and channel outputs
* The PB shows the battery state of the Tx


## RC

* Up to 9 analog inputs
    * Analog inputs can be used as switch inputs too
    * Can be stick, steering wheel, trim, pot
    * May or may not have a center point
* Switch inputs
    * Each input is on or off only
    * Multiple inputs can be combined to form a multi-position switch
    * Have a pull-up programmed
    * May be momentary or latching
    * Momentary buttons can be configured to make a multi-position switch
        * Transmitter beeps the current number
        * Long press support?
* Toggle switches and trims need to remember their state across power cycles

### Model vs Transmitter hardware configuration

* Ideally we are able to drive the same car with different transmitter hardware, with the system automatically adapting to the transmitter hardware
* This implies that inputs are tagged (e.g. Aileron, Rudder, Elevator, Steering, Throttle, Gear, ST-Trim, ST-DR, TH-Hold ...) so that the mixer can read from the right values
* An input may have multiple tags (Steering, Rudder) but each tag must only be assigned once
* The inputs also have generic tags like A0, SW0... so that they can be used in mixers
* Inputs may have 0..100 on one transmitter, and -100..0..100 on another one, i.e. they may be with our without center point!
* If a mixer can not find its input, the input is considered value "0"
* The direction is normalized: forward, more, right = positive; backward, less, left = negative

* So the "mixer" describes the vehicle, but an input section describes the transmitter
* This should also allow us to extract "mixer" configurations from the 3XS model memory expansion EEPROM
* The PB can change mixer configuration as well as hardware configuration

* The address can serve as a unique model identifier so that the PB can find the corresponding model in its memory


## Mixer

* The mixer is derived from Deviation
* The mixer calculates signed 16 bit servo values
* Common templates like V-tail, Flaperons, 4-wheel steering


## RF protocol

* Every 5 ms:
    * Send stick data
    * Send stick data again (after NRF24 IRQ)
    * Send bind packet with fixed address at low power on channel 81
    * Send setup packet on the current hop channel using Enhanced Shockburst
    * Sleep until the next 5 ms

* Programming box (PB):
    * Listens for bind backets on channel 81 and learns the hop sequence
    * Listens and responds to Setup packets on the hop channels

* Setup protocol:
    * The Tx is the master on RF, but it is the slave regarding communication with the programming box (PB)
    * The Tx has to poll the PB if it has a command for it to execute.
    * The Tx must acknowledge each command in the next packet so that the PB knows the command was received and executed.
    * The setup packet uses dynamic payload up to 10 bytes

* Establishing a connection:
    * The PB listens for Tx on the bind channel 81 and learns the address and hop channel sequence
    * The PB calculates a setup address from the learned address and listens for setup packets on the hop channels. It ACKs with a new address to use.
    * The Tx sends a setup packet with a mangled address every 5 ms. The payload signals "Free to connect".
    * When the Tx receives an answer with a new address from the PB it is now "connected" and starts sending setup packets on the received address using the hop frequencies.
    * Once the PB has the ACK being taken by the Tx (= it received the "Free to connect") it listens to the adress it gave to the Tx. The Tx and PB are now connected

* Terminating a connection
    * If the Tx or the PB do not receive anything for 600 ms they consider the connection lost and terminate the connected state immediately. The Tx returns sending "Free to connect" setup packets.
    * The PB can send the "Disconnect" command to the Tx. The Tx responds "Disconnecting now" and once it received the ACK from the PB it terminates the connection and returns sending "Free to connect" setup packets.
    * The PB considers the connection terminated after it received the "Disconnecting now" from the Tx.

* PB -> Tx
    * PB puts command in ACK payload
    * Tx responds with "Acknowedledged"

* Commands
    * Tx->PB Inquiry (payload: stick data)
    * PB->Tx NOP
    * Tx->PB Acknowledged
    * Tx->PB Free-to-connect
    * PB->Tx Disconnect
    * Tx->PB Disconnecting-now


## Battery indication:

* LED lights up when powered
* LED blinks when battery is low
* Regular beep when battery is low, getting more frequent the lower the battery gets