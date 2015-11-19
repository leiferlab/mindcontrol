// Inspired by:
// usb_rcfg.cpp : A simple USB DOS console application. Sample code.
// Author: Doug Lovett
// Birth: 02/07/2000
// 
// 
// Heavily modified  by Andrew Leifer

/*
 * Compile by running:
 *
 * gcc -v -Wall -o bin/Talk2Stage.exe MyLibs/Talk2Stage.c   -lsetupapi
 *
 */

#include <windows.h>
#include <objbase.h>
#include <setupapi.h> // you may have to manually include this library.
#include <initguid.h>
#include <conio.h>
#include <stdio.h>

// Defines
DEFINE_GUID(LEP_GUID, // LEP's global, unique identifier
		0x4d48f140, 0x44e2, 0x11d3, 0x9d, 0x64, 0x0, 0xe0, 0x29, 0x1d, 0xee, 0x58)
;
#define CTL_CODE( DeviceType, Function, Method, Access ) ( ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#define METHOD_BUFFERED 0
#define FILE_ANY_ACCESS 0
#define FILE_DEVICE_UNKNOWN 0x00000022
#define LEPUSB_IOCTL_VENDOR_INDEX 0x0800
#define IOCTL_LEPUSB_GET_READ_LENGTH CTL_CODE(FILE_DEVICE_UNKNOWN, LEPUSB_IOCTL_VENDOR_INDEX+1,METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LEPUSB_GET_VERSION_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, LEPUSB_IOCTL_VENDOR_INDEX+2,METHOD_BUFFERED, FILE_ANY_ACCESS)
// Function prototypes
int UsbScan(char*);

// Global Variables




// PrintVersion() - Gets and printf the driver version information.
void PrintVersion(HANDLE hUsb) {
	UCHAR Ver[512];
	ULONG nBytes;
	BOOLEAN Success;
	Success = DeviceIoControl(hUsb, IOCTL_LEPUSB_GET_VERSION_INFO, NULL, 0,
			Ver, sizeof(Ver), &nBytes, NULL);
	if (!Success)
		printf("ERROR: Read Driver Version.\n");
	Ver[nBytes] = NULL; // don't forget the NULL terminator
	printf("%s", Ver); // printf the Version
}
// --------------------------------------------------------------------- //

/*
 * Initializes the USB stage;
 */
HANDLE InitializeUsbStage(){




	/** If USB **/

		char DeviceName[MAX_PATH];

		// Scan for Usb Device
		if (!UsbScan(DeviceName)) {
			printf("No devices found!\n");
			printf("Moving on to try serial device!\n");
		} else {
			/** Device was found **/
			printf("Device Found: %s\n", DeviceName);
			HANDLE hUsb = CreateFile(DeviceName, GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (hUsb == INVALID_HANDLE_VALUE) {
				printf("Error %d: Failed to open USB file handle.\n", GetLastError());
				return NULL;
			} else
				printf("Device Opened: ");
			// Print driver version
			PrintVersion(hUsb);

			/** We were succesful so now we are done **/
			return hUsb;

		}





		printf("Checking for serial port!\n");

		/** If Serial **/

		/** This code is adapted from
		 *
		 * http://www.robbayer.com/files/serial-win.pdf
		 *
		 * **/

		/** Open the Serial Port **/
		HANDLE hSerial;
		hSerial = CreateFile("COM4", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hSerial==INVALID_HANDLE_VALUE){
			if(GetLastError()==ERROR_FILE_NOT_FOUND){
				//serial port does not exist.
				printf("ERROR: Serial port does not exist\n");
				return NULL;
			}
			printf("ERROR: Some other error with the serial port happend\n");
			//some other error occurred. Inform user.
			return NULL;
		}


		/** Set Serial Port Parameters **/
		DCB dcbSerialParams = {0};
		dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
		if (!GetCommState(hSerial, &dcbSerialParams)) {
			printf("Error getting serialport state\n");
			//error getting state
		}

		//CBR_9600 works
		//dcbSerialParams.BaudRate=CBR_9600;
		//CBR_115200 also works. //This matches what LUDL Says
		//CBR_256000 also works
		dcbSerialParams.BaudRate=CBR_115200; 
		dcbSerialParams.ByteSize=8;
		dcbSerialParams.StopBits=TWOSTOPBITS;
		dcbSerialParams.Parity=NOPARITY;

		if(!SetCommState(hSerial, &dcbSerialParams)){
			printf("Error setting serial port state\n");
			//error setting serial port state
		}


		/** Set TimeOuts **/
		COMMTIMEOUTS timeouts={0};
		timeouts.ReadIntervalTimeout=50;
		timeouts.ReadTotalTimeoutConstant=50;
		timeouts.ReadTotalTimeoutMultiplier=10;
		timeouts.WriteTotalTimeoutConstant=50;
		timeouts.WriteTotalTimeoutMultiplier=10;
		if(!SetCommTimeouts(hSerial, &timeouts)){
			//error occureed.
			printf("Error: unable to set serial port timeouts.\n");
		}
		printf("Successfully configured serial port\n");
		return hSerial;


}


/*
 *
 *  SendCommandToStage This is a wrapper of the Windows function WriteFile()
 *  See, e.g.  https://msdn.microsoft.com/en-us/library/windows/desktop/aa365747%28v=vs.85%29.aspx
 *
 */

BOOL SendCommandToStage(HANDLE hfile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD      lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped){
	BOOL bErrorFlag;
	bErrorFlag=WriteFile(hfile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	printf(" [SENT STAGE] `%s` \n",lpBuffer);
	return bErrorFlag;

}



/*
 If you don't clear some buffer and you are using the virtual com port driver, then 
 the stage stops responding to commands after around the 3300th command. By running this function
 you clear that mysterious buffer and everything works. See the email from rzingel@ludl.com
 */
void clearStageBuffer(HANDLE s){
	/* From an email from rzingel@ludl.com:
	 It looks like the characters that the MAC6000 is sending in response to the spin commands :A<LF> is causing a buffer overflow and that is causing the unit to stop responding.  It's possible that the proprietary driver handled this better then the virtual com.  There is a simple work around that seems to fix it.  I ran the test with out this code I added and it always failed around 3500 commands.  Once I made the change that I show below then I've had it running for over 15000 commands without the lockup issue.  Basically I just clear the comm errors and read any characters that are in the buffer.  */

	//Begin Buffer Clearing Fix from rzingel
		DWORD dwErrors; 
		DWORD Length;


        COMSTAT Status; 
        
        ClearCommError( s, &dwErrors, &Status);  // Windows function to clear a devices error flag
        Length = Status.cbInQue;        // get the rx data length in buffer 
        // Get data and put it in to pText 
        DWORD nRead; 
        char * pText; 
        pText = (char *)malloc(sizeof(char)*(Length + 2)); 
        ReadFile(s,pText, Length, &nRead,NULL); 
		printf(" [STAGE REPLY] `%s`\n",pText); // Display to console
        free(pText); 
 		return;

}



/*
 * Set the velocity of the stage
 *
 */
int spinStage(HANDLE s, int xspeed,int yspeed){
	

	 DWORD Length;
	 
	
	//Error handling modeled off of http://msdn.microsoft.com/en-us/library/windows/hardware/bb540534(v=vs.85).aspx
	BOOL bErrorFlag = FALSE;


	char* buff=(char*) malloc(sizeof(char)*1024);
	sprintf(buff,"SPIN X=%d Y=%d\r",-yspeed,-xspeed);
	bErrorFlag=SendCommandToStage(s, buff, strlen(buff), &Length, NULL);
	free(buff);

	if (FALSE == bErrorFlag)
    {
        printf("Failure: Unable to write to serial port.\n");
    }

	clearStageBuffer(s);
	return 0;

}



int haltStage(HANDLE s){
		DWORD Length;
		SendCommandToStage(s, "HALT\r", strlen("HALT\r"), &Length, NULL);
		clearStageBuffer(s);
		return 0;

}

int moveStageRel(HANDLE s, int xpos, int ypos){
	DWORD Length;
	char* buff=(char*) malloc(sizeof(char)*1024);
	sprintf(buff,"MOVEI X=%d Y=%d\r",xpos,ypos);
	SendCommandToStage(s, buff, strlen(buff), &Length, NULL);
	free(buff);
	clearStageBuffer(s);
	return 0;
}


int zeroStage(HANDLE s){
	DWORD Length;
	SendCommandToStage(s, "HERE X=0 Y=0\r", strlen("HERE X=0 Y=0\r"), &Length, NULL);
	clearStageBuffer(s);
	return 0;

}

int centerStage(HANDLE s){
	DWORD Length;
	SendCommandToStage(s, "CENTER X=30000 Y=30000\r", strlen("CENTER X=30000 Y=30000\r"), &Length, NULL);
	printf("Centering stage.\n This takes a really really long time.\n");
	printf("Hit enter when done. This will zero the stage.\n");
	scanf("");
	zeroStage(s);
	clearStageBuffer(s);
	return 0;

}




void steerStageFromNumberPad(HANDLE s, int speed, int input){
	  switch (input) {
	  /** Cardinal Directions **/
		case 6:
			printf("Right!\n");
			spinStage(s,speed,0);
			break;
		case 8:
			printf("Up!\n");
			spinStage(s,0,speed);
			break;
		case 4:
			printf("Left!\n");
			spinStage(s,-speed,0);
			break;
		case 2:
			printf("Down!\n");
			spinStage(s,0,-speed);
			break;
		/** Multiples of 45 **/
		case 9:
			printf("Up-Right!\n");
			spinStage(s,speed,speed);
			break;
		case 7:
			printf("Up-Left!\n");
			spinStage(s,-speed,speed);
			break;
		case 1:
			printf("Down-Left!\n");
			spinStage(s,-speed,-speed);
			break;
		case 3:
			printf("Down-Right!\n");
			spinStage(s,speed,-speed);
			break;
		case 5:
			printf("HALT!\n");
			haltStage(s);
			break;
		default:
			break;
	  }
	  return;
}


 // end of main()



// OLD WINXP PROPRIETORY DRIVER  INTERFACE:
//
// ------------------------------------------------------------------------ //
// UsbScan() - Scan for first USB device with a GUID that matches the one
// passed. Return 1 if device found, else 0.
// This function only detects the first match of the device found.
// If you have more than one controller attached you will have to
// scan for the device more than once. See the MFC windows
// example. You can also get the devicename from the registry.
int UsbScan(char* DeviceName) {



	// Get handle to the devices
	HDEVINFO hInfo = SetupDiGetClassDevs((LPGUID) & LEP_GUID, NULL, NULL,
			DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (hInfo == INVALID_HANDLE_VALUE)
		return (0);
	SP_INTERFACE_DEVICE_DATA Interface_Info;
	Interface_Info.cbSize = sizeof(Interface_Info); // Enumerate device
	if (!SetupDiEnumDeviceInterfaces(hInfo, NULL, (LPGUID) & LEP_GUID, 0,
			&Interface_Info)) {
		SetupDiDestroyDeviceInfoList(hInfo);
		return (0);
	}
	DWORD needed; // get the required length
	SetupDiGetInterfaceDeviceDetail(hInfo, &Interface_Info, NULL, 0, &needed,
			NULL);
	PSP_INTERFACE_DEVICE_DETAIL_DATA detail;
	detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(needed);
	if (!detail) {
		SetupDiDestroyDeviceInfoList(hInfo);
		return (0);
	}
	// fill the device details
	detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	if (!SetupDiGetInterfaceDeviceDetail(hInfo, &Interface_Info, detail,
			needed, NULL, NULL)) {
		free((PVOID) detail);
		SetupDiDestroyDeviceInfoList(hInfo);
		return (0);
	}
	strncpy(DeviceName, detail->DevicePath, MAX_PATH);
	free((PVOID) detail);
	SetupDiDestroyDeviceInfoList(hInfo);
	return (1); // return true, USB device found
}
// ---------------------------------------------------------------------- //

