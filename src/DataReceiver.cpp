#include "../include/DataReceiver.h"
#include "../include/cyusb.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <unistd.h>

DataReceiver::DataReceiver() :FLAG(false),timeout(1),transferred(0),h1(NULL){

	int r;

	r = cyusb_open();
	if ( r < 0 ) {
	   std::cout<<"Error opening library\n";
       cyusb_close();
	}
	else if ( r == 0 ) {
		std::cout<<"No device found\n";
        cyusb_close();
	}
	else if ( r > 1 ) {
		std::cout<<"More than 1 devices of interest found. Disconnect unwanted devices\n";
        cyusb_close();
	}

	h1 = cyusb_gethandle(0);
	if ( cyusb_getvendor(h1) != 0x04b4 ) {
	  	std::cout<<"Cypress chipset not detected\n";
		cyusb_close();
	}
	r = libusb_kernel_driver_active(h1, 0);
	if ( r != 0 ) {
	   std::cout<<"kernel driver active. Exitting\n";
	   cyusb_close();
	}
	r = libusb_claim_interface(h1, 0);
	if ( r != 0 ) {
	   std::cout<<"Error in claiming interface\n";
	   cyusb_close();
	}
	else 
    {
        std::cout<<"Successfully claimed interface\n";
        FLAG = 1;
    };
}

bool DataReceiver::getFlag()
{
    return FLAG;
}

libusb_device_handle * DataReceiver::getHandle()
{
    return h1;
}

int DataReceiver::receiveData(unsigned char * buffer, int len=1024)
{
	int transferred = 0;
	memset(buffer,'\0',len);
	return libusb_bulk_transfer(h1, 0x86, buffer, len, &transferred, timeout * 1000);
}

int DataReceiver::sendData(unsigned char* buffer, long len = 1024)
{
    int transferred = 0;
    return libusb_bulk_transfer(h1, 0x02, buffer, len, &transferred, timeout * 1000);
    
}
DataReceiver::~DataReceiver()
{
    if (FLAG)
        cyusb_close();
}

