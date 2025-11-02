#include "../include/DataDecoder.h"
#include <memory>
#include <arpa/inet.h>
#include <thread>

DataDecoder::DataDecoder()
{
}
DataDecoder::~DataDecoder()
{
}

bool DataDecoder::decodeDataTime(unsigned char *buffer, std::vector<Event> &packet, unsigned int &last_timestamp, unsigned int &last_raw_timestamp, unsigned int &start_timestamp,
                                 unsigned int end_timestamp, unsigned int &time_cycle, unsigned int &start, unsigned int &end)
{
    unsigned int y, x, p, t;
    unsigned int r;
    unsigned int buffer_single = 0;
    int finger_print = 0;
    for (; start < end; start += 4)
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // 426 231 1 1 90cfb4d323ad539f
        memcpy(&buffer_single, buffer + start, 4);
        buffer_single = ntohl(buffer_single);

        // 时间
        t = (buffer_single >> TIME_SHIFT) & TIME_MASK;
        // std::cout << t << std::endl;
        last_raw_timestamp = t;
        t = t / 2;
        t = t + time_cycle * 2048;
        if (t < last_timestamp)
        {
            // if time_cycle>
            time_cycle++;
            t += 2048;
        }

        last_timestamp = t;
        if (time_cycle >= 2097000)
        {
            time_cycle = 0;
            t = 0;
            last_timestamp = 0;
            start_timestamp = 0;
        }

        if (t < start_timestamp)
        {
            start = end;
            return false;
        }

        if (t >= end_timestamp)
        {
            return true;
        }

        r = (buffer_single >> R_SHIFT) & 1;
        p = (buffer_single >> P_SHIFT) & 1;
    
        // y 为高
        y = (((buffer_single >> Y_SHIFT) & COORD_MASK)) % 480;

        if (!r)
        {
            x = ((buffer_single)&COORD_MASK) % 320;
            if (x + y == finger_print)
                continue;
            else
                finger_print = x + y;
            packet.emplace_back(x, y, t, p);
        }
        else
        {
            x = (((buffer_single)&COORD_MASK) + 320) % 640;
            if (x + y == finger_print)
                continue;
            else
                finger_print = x + y;
            packet.emplace_back(x, y, t, p);
        }
    }
    return false;
}

bool DataDecoder::decodeDataCount(unsigned char *buffer, std::vector<Event> &packet, unsigned int &last_timestamp, unsigned int &last_count,
                                  unsigned int next_count, unsigned int &time_cycle, unsigned int &start, unsigned int &end)
{
    unsigned int y, x, p, t;
    unsigned int r;
    unsigned int buffer_single = 0;
    int finger_print = 0;
   
    for (; start < end; start += 4)
    {
        if (last_count >= next_count)
            return true;

        // 426 231 1 1 90cfb4d323ad539f
        memcpy(&buffer_single, buffer + start, 4);
        buffer_single = ntohl(buffer_single);

        // 时间
        t = (buffer_single >> TIME_SHIFT) & TIME_MASK;
        t = t / 2;
        t = t + time_cycle * 2048;
        if (t < last_timestamp)
        {
            // if time_cycle>
            time_cycle++;
            t += 2048;
        }

        r = (buffer_single >> R_SHIFT) & 1;
        p = (buffer_single >> P_SHIFT) & 1;
        // y 为高
        y = (((buffer_single >> Y_SHIFT) & COORD_MASK)) % 480;

        if (!r)
        {
            x = ((buffer_single)&COORD_MASK) % 320;
            if (x + y == finger_print)
                continue;
            else
                finger_print = x + y;
            packet.emplace_back(x, y, t, p);
        }
        else
        {
            x = (((buffer_single)&COORD_MASK) + 320) % 640;
            if (x + y == finger_print)
                continue;
            else
                finger_print = x + y;
            packet.emplace_back(x, y, t, p);
        }
        ++last_count;
    }
    return false;
}
