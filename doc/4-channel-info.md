# 4-channel adaptation

As of August 2018 the receiver supports a variant of the RF protocol that supports 4 channels.

Note: this mode is only useful if you use the LANE Boys RC [Headless transmitter](https://github.com/laneboysrc/rc-headless-transmitter), which is the only transmitter supporting this protocol.

The original HKR3000 protocol uses a payload of 10 bytes to transmit stick- and failsafe data. Two of the bytes (6 and 9) are unused. We make use of these bytes to transmit data for a 4th channel.

The receiver hardware has four connectors: 3 channels and a battery/pre-processor output.
In 4-channel mode we use the battery/preprocessor output as CH4, so no hardware modification is necessary.

In order to let the receiver know whether it should operate in 3 or 4 channel mode, we use different packet id fields for 4-channel mode and use a different marker for the first bind packet. The remainder of the protocol is identical with the HKR3000.


Stick data uses packet id 0x56 (instead of 0x55).
Failsafe data uses packet id 0xab (instead of 0xaa).

The bind packages are formatted as follows:
```
ff ab 56 a1 a2 a3 a4 a5 .. ..
cc cc 00 ha hb hc hd he hf hg
cc cc 01 hh hi hj hk hl hm hn
cc cc 02 ho hp hq hr hs ht ..
```
``ff ab 56``     Special marker for the first bind packet (Note: HKR3000 uses ``ff aa 55``)

``a[1-5]``       The 5 address bytes

``cc cc``        The 16 bit sum of bytes a1..a5

``h[a-t]``       20 channels for frequency hopping

``..``           Not used


## 3-channel mode (HKR3000 protocol)

Stick data:
```
0     1     2     3     4     5     6    7    8    9
ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h xxx  0x55 0x67 xxx
                                         ^^^^
                                         Packet id
```

Failsafe data:
```
0     1     2     3     4     5     6    7    8    9
ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h xxx  0xaa 0x5a xxx   (failsafe on)
                                              0x5b       (failsafe off)
                                         ^^^^
                                         Packet id
```

## 4-channel mode

Stick data:
```
0     1     2     3     4     5     6     7    8    9
ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h CH4-l 0x56 0x67 CH4-h
                                          ^^^^
                                          Packet id
```

Failsafe data:
```
0     1     2     3     4     5     6     7    8    9
ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h CH4-l 0xab 0x5a CH4-h (failsafe on)
                                               0x5b       (failsafe off)
                                          ^^^^
                                          Packet id
```
