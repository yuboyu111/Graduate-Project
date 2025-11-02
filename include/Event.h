#ifndef EVENT_H
#define EVENT_H
struct Event{
    unsigned short x;
    unsigned short y;
    unsigned int t;
    bool p;
    Event(unsigned int x_, unsigned int y_, unsigned int t_, unsigned int p_):x(x_),y(y_),t(t_),p(p_){}
};
#endif