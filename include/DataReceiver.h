#ifndef DATARECEIVER_H
#define DATARECEIVER_H
#include <iostream>
#include <libusb-1.0/libusb.h>
#include "../include/cyusb.h"

class DataReceiver
{
private:
    bool FLAG = false;
    int timeout;
    int transferred;
    libusb_device_handle *h1;
public:
    DataReceiver();
    ~DataReceiver();
    libusb_device_handle * getHandle();
    int receiveData(unsigned char * buffer, int len);
    int sendData(unsigned char* buffer, long len);
    bool getFlag();
};

#endif