<b>USB CDC (Communication Device Class) implementation for 'F32x and 'F34x</b>

Posted to: http://www.cygnal.org/ubb/Forum9/HTML/000945.html

This code is an implementation of CDC ACM (Abstract Control Model) discussed in the topic, <a href="http://www.cygnal.org/ubb/Forum9/HTML/000935.html">"F320 USB"</a>.
(Sorry Don, it takes longer to implement it than expected.)

USB_CDC_skeleton_14.zip

History
&nbsp; &nbsp; v1.0 (July 19, 2006)
&nbsp; &nbsp; &nbsp; &nbsp; Initial release
&nbsp; &nbsp; v1.1 (July 21, 2006)
&nbsp; &nbsp; &nbsp; &nbsp; Added support for 'F34x
&nbsp; &nbsp; &nbsp; &nbsp; Renamed the project and file names, not to include 'F32x_'
&nbsp; &nbsp; &nbsp; &nbsp; Delete Low-speed support
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; Actually, low-speed is achieved using interrupt endpoints instead of bulk.
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; But it is out of spec for CDC ACM.
&nbsp; &nbsp; &nbsp; &nbsp; The temporary buffer in line coding handling was moved
&nbsp; &nbsp; &nbsp; &nbsp; from unused USB FIFO to IDATA, for 'F34x compatibility
&nbsp; &nbsp; v1.2 (Aug 14, 2006)
&nbsp; &nbsp; &nbsp; &nbsp; Revised USB interrupt setting after USB bus reset.
&nbsp; &nbsp; &nbsp; &nbsp; Deleted 'reset data toggle' code in Set_Configuration, because it is done 
&nbsp; &nbsp; &nbsp; &nbsp; by USB bus reset (from personal communication with SiLabs)
&nbsp; &nbsp; v1.3 (Aug 18, 2006)
&nbsp; &nbsp; &nbsp; &nbsp; Optimized setup stage handling in the device request handler.
&nbsp; &nbsp; &nbsp; &nbsp; This optimization reduces the code size about 150 bytes for Keil,
&nbsp; &nbsp; &nbsp; &nbsp; 230 bytes for SDCC
&nbsp; &nbsp; v1.4 (Sep 5, 2006)
&nbsp; &nbsp; &nbsp; &nbsp; Optimized class-specific request handling. As the check of interface number is common, 
&nbsp; &nbsp; &nbsp; &nbsp; it is done before dispatcher.
&nbsp; &nbsp; &nbsp; &nbsp; Reviced Usb0_Init in USB_Main.c : enabled USB reset interrupt.
&nbsp; &nbsp; &nbsp; &nbsp; Reviced Set_Configuration in USB_Standard_Requests.c : recovered 'reset data toggle'
&nbsp; &nbsp; &nbsp; &nbsp; Reviced Usb_Suspend in USB_ISR.c, to match to the peripheral initialization in Port_Init.
&nbsp; &nbsp; &nbsp; &nbsp; Commented out USB Resume interrupt handling, It does nothing in this implementation.
&nbsp; &nbsp; &nbsp; &nbsp; Bug fix: F32x occasionally fails SET_LINE_CODING on high-speed hub.
&nbsp; &nbsp; &nbsp; &nbsp; This bug was fixed by deleting DATAEND of SUEND event handling in Handle_Setup (USB_ISR.c)


Applying CDC ACM, the device is recognized as 'virtual' COM port using built-in device driver, in most of major OS including Windows, MacOS and Linux.

In this code, the USB side is fully implemented, but no UART is attached. Instead, support routines to connect peripherals to it are provided. The purpose of this implementation is to place a start point for further modification. For simple USB-serial conversion, I recommend you to use existing USB-serial chips.

The code is based on SiLabs USB_INT (release 1.3). Though the original comments by SiLabs are preserved to respect their work, this implementation has no relation to them. (I also have no relation to SiLabs ;)) Use it on your own risk as usual.

Thanks Patryk, your advices in <a href="http://www.cygnal.org/ubb/Forum9/HTML/000938.html">"Optimization of SiLabs USB examples in code size"</a> are fully reflected to this implementation. Also, thanks Maarten and Frieder, your suggestions in the topic, <a href="http://www.cygnal.org/ubb/Forum10/HTML/000025.html">"Help to convert sample code of USB_INT to compile under SDCC"</a> help me much on this implementation.

Tsuneo


<b>Note</b>
<b>A) Compilation</b>
This code was checked on SiLabs 'F320 and 'F340 dev board.
Select one of appropriate include file for the device in 'USB_CDC_Type.h'

#include &lt;c8051f320.h&gt;
//#include &lt;c8051f340.h&gt;

The code is compiled with Keil or SDCC. The conditional compilation on the code automatically detects the compiler. The work space files, 'Keil_USB_CDC_skeleton.wsp' and 'SDCC_USB_CDC_skeleton.wsp', are provided for SiLabs IDE.

SMALL memory model
Code size
&nbsp; &nbsp; &nbsp; Keil full version and 4K eval (SiLabs)
&nbsp; &nbsp; &nbsp; &nbsp; 3754 (F320 and F340) bytes
&nbsp; &nbsp; &nbsp; SDCC 2.6.0 #4290 (20060718-4290)
&nbsp; &nbsp; &nbsp; &nbsp; 4105 (F320 and F340) bytes

USB spec compliance was checked using <a href="http://www.usb.org/developers/tools/">USBCV R1.3 beta</a> on Chapter 9.

Unfortunately, 'F326/7 cannot be supported because these devices has only two EP other than EP0.


<b>B) Installation</b>
Download the code to 'F320DK and connect the board to an USB port of your PC. 'New Hardware' wizard asks you to locate an INF(.inf) file. Specify the 'CDC_ACM.inf' in the INF folder in the zip file.

When installed successfully, the device appears on your PC as a COM port.


<b>C) Demonstration</b>
This implementation simply echoes back the TX output of COM port to RX port. Using a terminal software, such as <a href="http://realterm.sourceforge.net/">RealTerm</a>, you can confirm it by keyboard and file transfer.

Following transfer speed was observed on loop back, simultaneous bi-direction transfer (WinXP SP2).
<pre><font size="2">       SYSCLK  Speed (Kbytes/sec)
F320DK  24MHz  150 
F340DK  48MHz  280</font></pre>
Though I didn't check about transfer speed of single direction transfer, it'll be much faster, from the record of bus analyzer.

LEDs and SWs on the 'F320DK are connected to RTS, DTR and CTS, DSR, respectively. But RTS and CTS don't work well because usbser.sys on Windows is weird as noted below.


<b>D) Modification</b>
VID/PID of this implementation was bought from <a href="http://www.voti.nl/shop/catalog.html?USB-PID-10">VOTI</a> to puslish it on the web. You can use this VID/PID in your lab, but get your own VID/PID when you distribute your products. VID/PID is defined in 'USB_Descriptor.h' and INF file.

TX and RX from PC over USB are attached to ring buffers respectively in 'USB_CDC_UART.c'.
The data received from PC is stored in TX buffer.
'bit TXReady' shows the TX buffer has any data
'UINT TXcount' holds the number of bytes on the TX buffer.
'BYTE COMGetByte(void)' retrieves a byte from the TX buffer, and decrements 'TXcount'

The data sent to PC is held in RX buffer.
'bit RXReady' shows that RX buffer has room to store another data
'UINT RXcount' holds the number of bytes on the RX buffer.
'void COMPutByte( BYTE )' put the data to RX buffer.

The size and memory space allocation for the TX and RX buffer are defined in 'USB_CDC_UART.h'.

In this implementation, these output and input of ring buffers are connected directly together in the main loop, 'USB_Main.c', for demonstration. In your modification, these main stream buffers will be connected to the input and output of your peripheral.

Ring buffer is easy to use, but its speed performance is not so good. To handle an IN transfer (device -> PC) of steady transfer rate, such as ADC, a double buffer is suitable. Also when the packet size of single transfer is always the same, a double buffer works better. You can find an example of double buffer in the topic <a href="http://www.cygnal.org/ubb/Forum9/HTML/000292.html">'256000 bytes/sec Isochronous transfer'</a> on this forum.

Other than these main data transfer path, you'll need to exchange 'trigger' and 'parameters' between the device and the PC.

In 'USB_CDC_UART.c',
- The COM port setting (baudrate, data bits, stop bits, parity) from PC is received in Set_Line_Coding(). 
- DTR setting from PC is received in Set_Line_State().
- Break signaling from PC is received in Send_Break().
- DSR and other UART status (parity, frame error, etc.) are returned to PC in Update_Line_State().
See following 'COM session examples' how these functions on the firmware corresponds to the PC COM APIs.

These handshake signals and setting parameters are completely independent from the main data transfer path. ie. Even if you set the baudrate to either 9600 or 115200 on the PC, it has nothing to do with the transfer rate of the TX and RX. Therefore, these signals and setting parameters are used for 'command' from PC and 'reply' from the device.

Using or modifying these functions, you can connect any peripherals to PC COM port.

Turning macro POLL_READ_BYTE/POLL_WRITE_BYTE into functions saves about 310 bytes in Keil, 130bytes in SDCC. When the code size is more essential for your project than execution speed, apply this modification. To convert these macro to function, comment out these macros at the bottom of USB_Register.h.

Edit the INF file, 'CDC_ACM.inf', in the INF folder with your favorite text editor.
The VID/PID specified in the INF file must match to that of the device.
See the comments in the INF file for another points to edit.


<b>Q and A</b>
Q1. What are the resources of C8051F340 which are used by to implement this functionality ?
A1. 
&nbsp; On-chip peripherals:
&nbsp; - USB0 - absolutely yes  
&nbsp; - Internal OSC - for USB clock with clock recovery
&nbsp; - - SYSCLK: no restriction, select appropriate one for your task
&nbsp; - USB0 interrupt - required, priority (high/low) - either will do

&nbsp; Memories (bytes) - v1.4
&nbsp; 'xdata' is used for RX/TX buffer - tunable on its size and memory assignment
&nbsp; KEIL - code: 3754, data: 73.5, xdata: 512
&nbsp; SDCC - code: 4105, data: 0x00 - 0x46, except stack, xdata: 512

&nbsp; For the demonstration only - not the absolute requirement
&nbsp; - Timer2 and Timer2 interrupt
&nbsp; - - To debounce the switch inputs on the dev board
&nbsp; - Ports on 'F34x dev board
&nbsp; - - P2.0 and P2.1 - switch
&nbsp; - - P2.2 and P2.3 - LED
&nbsp; The ports (P0,P1,P2) are configured to fit to the jumper setting of the dev board. But it is not the absolute requirement.

Q2. In addtion to virtual com port, can this device equip another USB functions at the same time?
A2. Unfortunately, Windows built-in device driver, usbser.sys, doesn't accept it.
As the USB CDC spec allows it, a custom device driver can support composite device of CDC and other USB class, such as HID, MSC. But I don't know any free device driver which replaces usbser.sys. You must make it by yourself, or buy a commercial one.


<b>E) Weird usbser.sys</b>
The usbser.sys (built-in device driver of Windows) is weird in several points.

WinXP SP2, usbser.sys (5.1.2699.2180)

When the transfer size is just the multiple of 64 bytes (max packet size of bulk IN EP), ReadFile doesn't finish until zero length packet is received, even if the actual transfer size is equal to the requested size.

The requests issued (or not issued) by usbser.sys are as follows. You'll find how usbser.sys is weird.
<pre><font size="2">
a) Just after device configuration
  GET_LINE_CODING
  SET_CONTROL_LINE_STATE(0x00) - RTS:0, DTR:0

b) COM session: example 1
  RTS doesn't match between PC and device until SetCommState.
  Even though usbser.sys issues SET_CONTROL_LINE_STATE(0x00) at the configuration, 
  its internal setting (fRtsControl field of DCB) doesn't reflect it.
  GetCommState and SetCommState issues redundant requests.
  CloseHandle issues redundant SET_CONTROL_LINE_STATE

- CreateFile
    no request
- PurgeComm
    no request
- GetCommState
    GET_LINE_CODING
    GET_LINE_CODING
- change baudrate on DCB
    fRtsControl is '1' as default when DCB is read out.
- SetCommState
    GET_LINE_CODING
    GET_LINE_CODING
    SET_LINE_CODING
    GET_LINE_CODING
    SET_CONTROL_LINE_STATE(0x02) - RTS:1, DTR:0
    SET_LINE_CODING
    GET_LINE_CODING
- ReadFile/WriteFile
    bulk transfer
- CloseHandle
    SET_CONTROL_LINE_STATE(0x02) - RTS:1, DTR:0

c) COM session: example 2
  EscapeCommFunction( SETRTS ) and EscapeCommFunction( CLRRTS ) issue no request.
  Only the fRtsControl field of DCB seems to be able to set/reset RTS 

- CreateFile
    no request
- EscapeCommFunction( SETDTR )
    SET_CONTROL_LINE_STATE(0x01) - RTS:0, DTR:1
- EscapeCommFunction( CLRDTR )
    SET_CONTROL_LINE_STATE(0x00) - RTS:0, DTR:0
- EscapeCommFunction( SETRTS )
    no request
- EscapeCommFunction( CLRRTS )
    no request
- EscapeCommFunction( SETBREAK )
    SEND_BREAK(0xFFFF)
- EscapeCommFunction( CLRBREAK )
    SEND_BREAK(0x0000)
- SetCommBreak
    SEND_BREAK(0xFFFF)
- ClearCommBreak
    SEND_BREAK(0x0000)
- GetCommModemStatus
    no request - (because the status is returned by the interrupt EP)
- ClearCommError
    no request - (because the status is returned by the interrupt EP)
- CloseHandle
    SET_CONTROL_LINE_STATE(0x00) - RTS:0, DTR:0

d) COM session: example 3
  CTS is always asserted regardless of actual input.

- CreateFile
    no request
- GetCommModemStatus
    stat = 0x10 (CTS: 1, DSR: 0)
- set DSR to 1
- GetCommModemStatus
    stat = 0x30 (CTS: 1, DSR: 1)
- set DSR to 0
- GetCommModemStatus
    stat = 0x10 (CTS: 1, DSR: 0)
- set CTS to 1
- GetCommModemStatus
    stat = 0x10 (CTS: 1, DSR: 0)
- set CTS to 0
- GetCommModemStatus
    stat = 0x10 (CTS: 1, DSR: 0)
- CloseHandle
    SET_CONTROL_LINE_STATE(0x00) - RTS:0, DTR:0
</font></pre>