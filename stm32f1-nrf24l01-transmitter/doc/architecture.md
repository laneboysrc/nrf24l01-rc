# STM32F1 NRF24L01+ Headless Transmitter Architecture


## RF protocols

### HK300 / HK310 / X3S protocol
* 20 hop channels
* 5 byte address
* 3 servo channels
* Stick data sent every 5 ms
* Hop frequency changed every 5 ms
* Failsafe sent

### Other protocols
* Based on HK300 protocol
* 8 channels
* signed 12 bit, corresponding to 476..1500..2523 us (500 ns resolution)
    - 8 channels, 12 bits means 12 Bytes of data, plus one frame identifier
    - More channels can be added by introducing multiple frames. The higher channels could be sent at a slower update rate


## START SIMPLE

* Basic 4 channel Tx; output to 3 channel receiver
* Reversing
* EPA
* Sub-trim
* Expo
* Dual-rate
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
    * Two switches can be combined to form a trim function
    * Have a pull-up programmed
    * May be momentary or latching
    * Momentary buttons can be configured to make a multi-position switch
        * Transmitter beeps the current number
        * Long press support?
* **Toggle switches and trims need to remember their state across power cycles**


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

* The nRF24 address can serve as a unique model identifier so that the PB can find the corresponding model in its memory


## Mixer

* The mixer is derived from Deviation
* The mixer calculates signed 16 bit servo values
* Common templates like V-tail, Flaperons, 4-wheel steering


## RF protocol

* Every 5 ms:
    * Send stick data
    * Send stick data again (after NRF24 IRQ)
    * Send bind packet with fixed address at low power on channel 81
    * If connected:
        * Send setup packet using Enhanced Shockburst at 2 Mbps
    * else if first hop channel
        * Send setup packet with "free to connect" using Enhanced Shockburst at 2 Mbps
    * Change to the next hop channel
    * Sleep until the next 5 ms

* Programming box (PB):
    * Listens for bind backets on channel 81 and learns the address and hop sequence
    * Listens and responds to Setup packets on the first hop channel at 2 Mbps
        * Note that legacy HK310 Tx sends bind packets but no Setup packet, so the PB has to deal with that

* Setup protocol:
    * The Tx is the master on RF, but it is the slave regarding communication with the programming box (PB)
    * The Tx has to poll the PB if it has a command for it to execute.
    * The Tx must acknowledge each command in the next packet so that the PB knows the command was received and executed.
    * The setup packet uses dynamic payload up to 32 bytes at 2 Mbps (ARD must be set to 500 us)

* Establishing a connection:
    * The PB listens for Tx on the bind channel 81 at 250 kBps and learns the address and hop channel sequence
    * The PB listens for Setup packets on the first hop channel. It ACKs with a new address to use; this way there can only be a single connection between a Tx and a PB at a given point in time.
    * The Tx sends a setup packet with the (vehicle) address on the first hop channel (every 100 ms). The payload signals "Free to connect".
    * When the Tx receives an answer with a new address from the PB it is now "connected" and starts sending setup packets on the received address using the hop frequencies every 5 ms.
    * Once the PB has the ACK being taken by the Tx (= it received the "Free to connect") it listens to the adress it gave to the Tx. The Tx and PB are now connected.
        * Note that the PB may have taken the ACK, but the Tx may not have received it. In that case the PB would timeout after 600 ms as described in "Terminating a connection".

* Terminating a connection
    * If the Tx or the PB do not receive anything for 600 ms they consider the connection lost and terminate the connected state. The Tx returns to sending of "Free to connect" setup packets.
    * The PB can send the "Disconnect" command to the Tx. The Tx responds "Disconnecting now" and once it received the ACK from the PB it terminates the connection and returns sending "Free to connect" setup packets.
    * The PB considers the connection terminated after it received the "Disconnecting now" from the Tx, or after 600 ms if not received.

* PB -> Tx
    * PB puts command as ACK payload
    * Tx responds with "Acknowedledged"
    * If PB does not receive "Acknowedledged" it knows that the Tx could not receive the command and it has to resend it with the next ACK

* Commands
    * Free-to-connect [Tx->PB]
        - Only sent on first hop channel (= every 100 ms)
        - Sent on the vehicle address
        - Only allows Connect command from PB

    * Connect [PB->Tx]
        - Only sent on first hop channel
        - Sent on the vehicle address
        - Payload: address to use for the rest of the communication

    * Inquiry Tx->PB
        - Payload: stick data (raw? channel outputs? how to deal with multiple channels?)

    * Tx->PB Acknowledged
    * PB->Tx Disconnect
    * Tx->PB Disconnecting-now

    * Read data
        - Payload: address (uint16_t), count (uint8__t)
    * Write data
        - Payload: address (uint16_t), up to 28 bytes data


## Bandwidth

- 32 bytes every packet
- 1 packet every 5 ms -> 200 packets per second

  => 6400 Bytes/s => 51200 Kbps (best case, realistically will be less than half due to having to wait for acks from the TX)

So uploading/reading Mixer data will take about 750 ms.


## UART protocol

Ideally we can use the same protocol as RF over the UART. Of course we don't need to establish a connection, the TX can see the UART as always being connected.

One issue is that there is no way to automatically determine package boundaries over the UART. So we need to wrap the protocol.
We could send the number of bytes in the packet (1 Byte), and add a checksum (2 Bytes) to the end of the packet. The receiving side would shift bytes until it finds a matching CRC, then removes that packet from the buffer.
The STM32F103 supports CRC, but only 32 bit data words. Need to figure out how to deal with variable lengths packages yet still be able to use the hardware. Maybe we expand 8 bits with leading 0 to 32 bits? And use the lower 16 bits as result.

The (UART based) PB shall only send 1 packet of data after receiving a packet from the TX. This way the TX can time a packet every 5 ms and we don't get an issue with swamping the TX.


## Battery indication:

* LED lights up when powered
* LED blinks when battery is low
* Regular beep when battery is low, getting more frequent the lower the battery gets


## RC transmitter modes:

http://www.rc-airplane-world.com/rc-transmitter-modes.html

Mode 1:
    Left:   Elevator, Rudder
    Right:  Throttle, Ailerons

Mode 2:
    Left:   Throttle, Rudder
    Right:  Elevator, Ailerons

Mode 3:
    Left:   Elevator, Ailerons
    Right:  Throttle, Rudder

Mode 3:
    Left:   Throttle, Ailerons
    Right:  Elevator, Rudder



## Mixer

The mixer originates from the DeviationTX project, which in turn comes in part from the ER9x project.
However, in contrast to DeviationTx and ER9x the UI has always the sources on the left and the channel outputs on the right. I find this more intuitive.

    Src   Mixer     Condition   Mux   Dest
    --------------------------------------
    ST    Expo/Dr               =     CH1
    TH    Simple                =     CH2
          Custom    TH-hold     =

### Mixer types:
- Simple: Source, Curve, Scale, Offset
- Expo/Dr: Source, Curve, Scale; Switch, Curve (can be linked), Scale; Switch Curve (can be linked), Scale
- Custom: Source, Curve, Scale, Offset, Switch
- Cut: Value, SW (to implement Throttle cut, or TH-hold)

### Mux types:
- = Replace
- + Add
- * Multiply

UI issue: how to add and delete entries for a destination?

The above describes the mixer from a UI point of view.
Technically all the mixer types are implementing by one *mixer unit* and virtual channels (in the case of Expo/Dr).

### Mixer unit
The mixer unit is derived from the DeviationTx project. Each unit performs a simple function:

    if (Switch) then
        Destination  op  f(Curve, Source) * Scalar + Offset
    endif

Where:
  - Switch: a switch state that, if true, enables the mixer unit. If Switch is <None> then the mixer unit is always enabled.
  - Destination: The destination channel that receives the output of the mixer unit.
  - op: The operation to perform. Can be
    * = replace destination channel value with output of the mixer unit),
    * += add mixer unit output to current destination channel value
    * *= multiply mixer unit output with the current destination channel value
    * MAX (**?**)
    * MIN (**?**)
    * Delay (**?**)
  - f(): One of the curve functions applied to the input source
  - Source: The input source for the mixer unit. Can be
    - Phyiscal transmitter inputs (sticks, pots, switches, push-buttons)
    - Channels (output channels as sent to the receiver)
    - Virtual channels (10 available for user selection, 10 hidden for Expo/Dr)
    - Trainer port inputs (not supported but prepare...)
    - **Sources may be inverted**
  - Scaler: A scaling factor
  - Offset: An offset value to move the result up or down
  - Trim: **FIXME: trim: what does it do?**

The mixer unit also receives a *tag* that identifies how it is used in the UI, i.e. as what mixer type the UI it needs to be presented. This tag is only used by the PB and not the Tx. It allows the PB to reconstruct the model setup by reading the Tx without having the model in its own memory.

Multiple mixer units can be configured to operate on the same destination. The order of which the mixer units are configured plays an important role of the final resulting channel value.

### Mapping mixer units to mixers

Simple:
  - 1 mixer unit
  - Trim is enabled (depends on source?)
  - Switch is "None"

Custom:
  - 1 mixer unit
  - All fields exposed in the UI

Cut:
  - 1 mixer unit
  - Switch and Scale exposed, Source is *None*, Curve is *Fixed*, Offset is 0

Expo/Dr:
  - Up to 4 mixer units
  - Virtual setup:

    Src       Mixer       Condition   Mux    Dest
    -----------------------------------------------------
    (source)  f1/scalar1  None        =      (virtual)
    (source)  f2/scalar2  (switch1)   =      (virtual)
    (source)  f3/scalar3  (switch2)   =      (virtual)
    (virtual) 1:1/100%                (mux)  (destination)

  This way in the UI the Expo/Dr appears as a single mixer, which result can be mux'ed as normal.
  - UI shows:
    - 3 curves f1..3; f2/f3 can be set to "linked", which means they copy the value from f1
      - Note that "linked" can not be read back from the Tx. So if the user programs the same curve twice, when reading it back from the Tx it may be shown as linked.
        - Need to decide on the UI: is "linked" an entry in the curve, or is there a separate button like in the DeviationTx UI ("Mid-Rate", "Low-Rate")
    - 2 switches (switch1..2). If a switch is set to "None" that mixer unit is removed
    - 3 scalars to adjust the actual dual rate

### Mixer resources

Because Expo/Dr can use up to 4 mixer units, we need to figure out how to detect that we run out of mixer units or (hidden) virtual channels. The overflow can occur when we
  - Add a mixer to a destination
  - Change a mixer to Expo/Dr
DeviationTx has 88 mixers (12 + 10) * 4. However, in the UI one could assign 170 complex mixers (7 channels + 10 virtual ones, 10 mixer units each)
A possible solution would be to have the mixer type selection in such a way that when the user would change to Expo/Dr and not enough mixers are available, he would get a dialog and the setting remains. The user could still select the other types that use only one mixer unit.
Actually, since there are only 10 hidden virtual channels, the user can only add 10 Expo/Dr mixers.

Memory is not really an issue (20 KBytes on the MCU), but we have to read/write the settings over the air! Also the UI will be tricky if we allow too many mixer units.

10 Expo/DR = 40 mixer units

    struct Curve {
        enum curve_type;
        uint8_t points[13]
    } curve ;
    uint8_t source;
    uint8_t destination;
    uint8_t switch;
    int8_t  scalar;
    int8_t  offset;
    uint8_t tag;

= 22 bytes per mixer unit
So 100 mixers would be 2.2 KBytes

The PB must align the mixers in the TX so that they can be processed in one loop.


## What functions should the PB have access to when connected to a TX?

- TX hardware configuration (which inputs, their names and types, trims, invert)
    - Function to calibrate the sticks, pots
- Mixer configuration (applied in the next 5 ms!)
- Output configuration (reversing, endpoints, sub-trim)
- Live stick, pot, switch, push-button, trims and output channel data
- Battery state


## Output channel configuration

- Normal/Reverse
- Fail-safe on/off and value
- (Safety None/channel and value; **what does this do?**)
- Min/max limit (**just hard limits**)
- Scale -/+ (**end points, but influenced by sub-trim**)
- Sub-trim
- Speed (0..250, **what does this do?**)