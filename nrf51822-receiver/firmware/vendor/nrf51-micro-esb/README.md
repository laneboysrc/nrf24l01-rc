nrf51-micro-esb
==================
DEPRECATED
----------
Unless legacy ShockBurst support is needed (no ACK, no retransmit), please use the nrf_esb library in nRF5_SDK v11 or later instead of this library. 
The new official ESB library is based on micro-esb, but has received several improvements and been thoroughly tested. 

Description
-----------
This project includes a library called micro_esb, which is a stripped down, simplified version of the Enhanced Shockburst protocol in the nRF51 SDK. 

The main benefits of this library over the official one are:
1) The source code is available, allowing anyone to make changes to the code if necessary
2) Packet timing is tighter, allowing quicker data transfer and higher overall data rate
3) Extended compatibility with certain ESB configurations, such as fixed payload length (the official lib supports dynamic payload length only)

The library can be used with or without a SoftDevice.

Requirements
------------
- nRF51 SDK version 8.0.0
- nRF51822 Development kit

The project may need modifications to work with later versions or other boards. 

To compile it, clone the repository in the /examples/proprietary_rf/ folder.

About this project
------------------
This application is one of several applications that has been built by the support team at Nordic Semiconductor, as a demo of some particular feature or use case. It has not necessarily been thoroughly tested, so there might be unknown issues. It is hence provided as-is, without any warranty. 

However, in the hope that it still may be useful also for others than the ones we initially wrote it for, we've chosen to distribute it here on GitHub. 

The application is built to be used with the official nRF51 SDK that can be downloaded from developer.nordicsemi.com.
