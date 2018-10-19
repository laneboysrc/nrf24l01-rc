# 8-channel receiver

Beside the original HKR3000 3-channel protocol and our modified 4-channel protocol, the LPC812-based receiver now comes in a hardware variant that supports 8 servo channels.

Note: this protocol is only useful if you use the LANE Boys RC [Headless transmitter](https://github.com/laneboysrc/rc-headless-transmitter), which is the only transmitter supporting this protocol.



## nRF24 configuration
- Enhanced Shockburst with DPL (dynamic payload length)
- 5 byte address size
- 2 byte CRC
- Uni-directional data transfer (transmitter sends no-ack packets)
- Transmitter sends a packet train consisting of three packets every 5 ms: two stick (or failsafe) packets, one bind packet


## Stick and Failsafe packets:
- 250 KBps
- Cycles through 20 hop channels at each packet train (= changes channel every 5 ms, repeats after 100 ms)
- Failsafe packets are sent every 17th packet train
- 13 bytes of data, unsigned 12 bits per channel, corresponding to 476..1500..2523 us (500 ns resolution)

```
id l1 l2 l3 l4 l5 l6 l7 l8 h1 h3 h5 h7
```

``id``            Packet ID: 0x57 for stick packet, 0xac for failsafe packet

``l[1-8]``        Lower 8 bits of the 12 bit channel data for CH1..CH8

``h1..h7``        Higher 4 bits of the 12 bit channel data.
                  CH1 = (h1 & 0x0f) << 8 + l1
                  CH2 = (h1 & 0xf0) << 4 + l2
                  CH3 = (h3 & 0x0f) << 8 + l3
                  CH4 = (h3 & 0xf0) << 4 + l4
                  ...


## Bind packets:
- Fixed channel 81 for bind packets
- Fixed address 12:23:23:45:78
- 2 MBps (to keep air-time to a minimum)
- 27 bytes of data

```
ac 57 a1 a2 a3 a4 a5 ha hb hc hd he hf hg hh hi hj hk hl hm hn ho hp hq hr hs ht
```

``ac 57``         Packet ID for bind packet

``a[1-5]``        5 address bytes (unique for each model)

``h[a-t]``        20 hop channels
