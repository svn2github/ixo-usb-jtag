//-----------------------------------------------------------------------------
// USB_Standard_Requests.h
//-----------------------------------------------------------------------------

#ifndef USB_STANDARD_REQUEST_H
#define USB_STANDARD_REQUEST_H

//-----------------------------------------------------------------------------
// Setup Packet Type Definition
//-----------------------------------------------------------------------------

typedef struct
{
	BYTE bmRequestType;					// Request recipient, type, direction
	BYTE bRequest;						// Specific standard request number
	WORD wValue;						// varies according to request
	WORD wIndex;						// varies according to request
	WORD wLength;						// Number of bytes to transfer
} Tsetup_buffer;						// End of Setup Packet Type

//-----------------------------------------------------------------------------
// Definition of Standard device request
//-----------------------------------------------------------------------------

// Device Request Direction (bmRequestType bit7)
#define DRD_MASK				0x80		// Mask for device request direction
#define DRD_OUT					0x00		// OUT: host to device
#define DRD_IN					0x80		// IN:	device to host

// Device Request Type (bmRequestType bit6-5)
#define DRT_MASK				0x60		// Mask for device request type
#define DRT_STD					0x00		// Standard device request
#define DRT_CLASS				0x20		// Class specific request
#define DRT_VENDOR				0x40		// Vendor specific request

// Device Request Recipient (bmRequestType bit4-0)
#define DRR_MASK				0x1F		// Mask for device request recipient
#define DRR_DEVICE				0x00		// Device
#define DRR_INTERFACE			0x01		// Interface
#define DRR_ENDPOINT			0x02		// Endpoint

// Define bmRequestType bitmaps
#define OUT_DEVICE			(DRD_OUT | DRT_STD | DRR_DEVICE)		// Request made to device,
#define IN_DEVICE			(DRD_IN  | DRT_STD | DRR_DEVICE)		// Request made to device,
#define OUT_INTERFACE		(DRD_OUT | DRT_STD | DRR_INTERFACE)		// Request made to interface,
#define IN_INTERFACE		(DRD_IN  | DRT_STD | DRR_INTERFACE)		// Request made to interface,
#define OUT_ENDPOINT		(DRD_OUT | DRT_STD | DRR_ENDPOINT)		// Request made to endpoint,
#define IN_ENDPOINT			(DRD_IN  | DRT_STD | DRR_ENDPOINT)		// Request made to endpoint,

#define OUT_CL_INTERFACE	(DRD_OUT | DRT_CLASS | DRR_INTERFACE)	// Request made to class interface,
#define IN_CL_INTERFACE		(DRD_IN  | DRT_CLASS | DRR_INTERFACE)	// Request made to class interface,

#define OUT_VR_INTERFACE	(DRD_OUT | DRT_VENDOR | DRR_INTERFACE)	// Request made to vendor interface,
#define IN_VR_INTERFACE		(DRD_IN  | DRT_VENDOR | DRR_INTERFACE)	// Request made to vendor interface,

// Standard Request Codes
#define GET_STATUS				0x00		// Code for Get Status
#define CLEAR_FEATURE			0x01		// Code for Clear Feature
#define SET_FEATURE				0x03		// Code for Set Feature
#define SET_ADDRESS				0x05		// Code for Set Address
#define GET_DESCRIPTOR			0x06		// Code for Get Descriptor
#define SET_DESCRIPTOR			0x07		// Code for Set Descriptor(not used)
#define GET_CONFIGURATION		0x08		// Code for Get Configuration
#define SET_CONFIGURATION		0x09		// Code for Set Configuration
#define GET_INTERFACE			0x0A		// Code for Get Interface
#define SET_INTERFACE			0x0B		// Code for Set Interface
#define SYNCH_FRAME				0x0C		// Code for Synch Frame(not used)

// Descriptor type (GET_DESCRIPTOR and SET_DESCRIPTOR)
#define DST_DEVICE				0x01		// Device Descriptor
#define DST_CONFIG				0x02		// Configuration Descriptor
#define DST_STRING				0x03		// String Descriptor
#define DST_INTERFACE			0x04		// Interface Descriptor
#define DST_ENDPOINT			0x05		// Endpoint Descriptor

// Define wValue bitmaps for Standard Feature Selectors
#define DEVICE_REMOTE_WAKEUP	0x01		// Remote wakeup feature(not used)
#define ENDPOINT_HALT			0x00		// Endpoint_Halt feature selector

//-----------------------------------------------------------------------------
// Definition of device and endpoint state
//-----------------------------------------------------------------------------

// Define device states
#define DEV_ATTACHED			0x00		// Device is in Attached State
#define DEV_POWERED				0x01		// Device is in Powered State
#define DEV_DEFAULT				0x02		// Device is in Default State
#define DEV_ADDRESS				0x03		// Device is in Addressed State
#define DEV_CONFIGURED			0x04		// Device is in Configured State
#define DEV_SUSPENDED			0x05		// Device is in Suspended State

// Define Endpoint States
#define EP_IDLE					0x00		// This signifies Endpoint Idle State
#define EP_HALT					0x01		// Endpoint Halt State (return stalls)
// for EP0
#define EP_TX					0x02		// Endpoint Transmit State
#define EP_RX					0x03		// Endpoint Receive State
// Endpoint Stall (send procedural stall next status phase)
#define EP_STALL				0x04

//-----------------------------------------------------------------------------
// Definition of Class specific request
//-----------------------------------------------------------------------------

// CDC ACM class specifc requests
#define SEND_ENCAPSULATED_COMMAND	0x00
#define GET_ENCAPSULATED_RESPONSE	0x01
#define SET_LINE_CODING				0x20
#define GET_LINE_CODING				0x21
#define SET_CONTROL_LINE_STATE		0x22
#define SEND_BREAK					0x23


//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

// USB Routines
void Handle_Setup(void);				// Handle setup packet on Endpoint 0
void Usb_Reset(void);					// Called after USB bus reset
void Usb_Suspend(void);					// This routine called when suspend
										// signalling on bus
//void Usb_Resume(void);					// This routine resumes USB operation
void Handle_EP_HALT(void);				// Handle endpoint HALT status, except EP0

void Standard_Device_Request(void);		// Standard Requests
void Class_Request( void );				// Class specific request
void Vendor_Request( void );			// Vendor request

void Handle_In(void);					// Handle in packet on Endpoint 1
void Handle_Out(void);					// Handle out packet on Endpoint 1/2

// Used for multiple byte reads of Endpoint FIFOs
void Fifo_Read (BYTE, BYTE, BYTE *);
// Used for multiple byte writes of Endpoint FIFOs
void Fifo_Write (BYTE, BYTE, BYTE *);


#endif	// USB_STANDARD_REQUEST_H

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
