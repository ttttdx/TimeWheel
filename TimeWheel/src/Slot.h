#ifndef SLOT_HPP
#define SLOT_HPP
#include <memory>
#include <list>

typedef struct TimePos{
    int pos_ms;
    int pos_sec;
    int pos_min;
    int interval;
}TimePos_t;

typedef struct Event {
    void*(*func)(void*);
    void* arg;
    TimePos_t timePos;
}Event_t;

class TimeWheel {

typedef std::shared_ptr<TimeWheel> TimeWheelPtr;

public:
    TimeWheel();
    
    void initTimeWheel(int steps, int maxMin);
    static void* loopForInterval(void* arg);
private:
    
    std::list<std::list<Event>> m_slotList;
    TimePos_t m_timePos;
    pthread_t m_loopThread;

    int m_firstLevelCount;
    int m_secondLevelCount;
    int m_thirdLevelCount; 
    
    int m_steps;
};


#endif