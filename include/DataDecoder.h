#ifndef DATADECODER_H
#define DATADECODER_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include "DataReceiver.h"
#include "Event.h"

class DataDecoder
{
private:

    unsigned int COORD_MASK = 511;
    unsigned int TIME_MASK = 4095;

    unsigned int TIME_SHIFT = 20;
    unsigned int Y_SHIFT = 9;
    unsigned int P_SHIFT = 18;
    unsigned int R_SHIFT = 19;
    
public:
    DataDecoder();
    ~DataDecoder();
    bool decodeDataTime(unsigned char * buffer , std::vector<Event> & packet, unsigned int &last_timestamp, unsigned int &last_raw_timestamp, unsigned int & start_timestamp, unsigned int  end_timestamp, unsigned int &time_cycle, unsigned int &start, unsigned int &end);
    bool decodeDataCount(unsigned char * buffer , std::vector<Event> & packet, unsigned int &last_timestamp, unsigned int &last_count, 
    unsigned int next_count, unsigned int &time_cycle, unsigned int &start, unsigned int &end);
};


#endif