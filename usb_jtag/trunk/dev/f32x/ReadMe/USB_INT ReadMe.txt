-------------------------------------------------------------------------------
 Readme.txt
-------------------------------------------------------------------------------

Copyright 2005 Silicon Laboratories, Inc.
http://www.silabs.com

Program Description:
-------------------

This application note covers the implementation of a simple USB application 
using the interrupt transfer type. Thisincludes support for device enumeration, 
control and interrupt transactions, and definitions of descriptor data. The
purpose of this software is to give a simple working example of an interrupt 
transfer application; it does not includesupport for multiple configurations, 
or other transfer types.

How To Test:
-----------

1) Download the code to any C8051F32x target board using the Silicon Labs IDE
2) Run the GUI application that comes with the firmware and test the features


Known Issues and Limitations:
----------------------------
	
1)  Firmware works with the Silicon Labs IDE v1.71 or later and the 
	Keil C51 tool chain. Project and code modifications will be 
	necessary for use with different tool chains.

2)  Compiler optimization emphasis is selected as "favor small code". 
	This selection is necessary for the project to be compiled with the 
	trial version of the Keil C51 Compiler (under 4k code space).

3)  If using the C8051F320TB target board, jumper J2 should be 
	installed if the board is wall-powered; 
	jumper J11 should be installed if bus-powered.

4)  Windows application and driver supports Win2K, XP, and 98SE only.
	

Target and Tool Chain Information:
---------------------------------

FID:            32X000029
Target:         C8051F32x
Tool chain:     Keil C51 7.50 / Keil EVAL C51
                Silicon Laboratories IDE version 2.6
Project Name:   F32x_USB_Interrupt


Command Line Options:
--------------------

Assembler : Default
Compiler  : Default
Linker    : Default 


File List:
---------

F32x_USB_Descriptor.c
F32x_USB_Descriptor.h
F32x_USB_ISR.c
F32x_USB_Main.c
F32x_USB_Main.h
F32x_USB_Register.h
F32x_USB_Standard_Requests.c
ReadMe.txt (this file)


Release Information:
-------------------

Release 1.3
  -Updated all files (GP)
  -14 DEC 2005

Release 1.2
  -Initial revisions by JS / CS / JM
  -JM revisions were on 25 MAY 2005
  -Before the new guidelines, so history is not available

Version 1.1
	Temperature measurement adjusted for smaller temperature changes.
	        
Version 1.0
	Initial release.

-------------------------------------------------------------------------------
 End Of File
-------------------------------------------------------------------------------