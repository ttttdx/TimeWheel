#include "Slot.h"

#include <iostream>
#include <memory.h>
#include <chrono>
#include <thread>

TimeWheel::TimeWheel() : m_steps(0), m_firstLevelCount(0), m_secondLevelCount(60), m_thirdLevelCount(0),
                         m_increaseId (0){}

void* TimeWheel::loopForInterval(void* arg)
{
    if(arg == NULL) {
        printf("valid parameter\n");
        return NULL;
    }
    TimeWheel* timeWheel = reinterpret_cast<TimeWheel*>(arg);
    while(1) {
       std::this_thread::sleep_for(std::chrono::milliseconds(timeWheel->m_steps));
        printf("wake up\n");
        TimePos pos = {0};
        TimePos m_lastTimePos = timeWheel->m_timePos;
        //update slot of current TimeWheel
        timeWheel->getTriggerTimeFromInterval(timeWheel->m_steps, pos);
        timeWheel->m_timePos = pos;
        {
            std::unique_lock<std::mutex> lock(timeWheel->m_mutex);
            // mean enter a new mintue, int the start of a new mintue
            if (pos.pos_min != m_lastTimePos.pos_min)
            {
                std::list<Event_t>* eventList = &timeWheel->m_eventSlotList[timeWheel->m_timePos.pos_min + timeWheel->m_firstLevelCount + timeWheel->m_secondLevelCount];
                timeWheel->processEvent(*eventList);
                eventList->clear();
           }
            else if (pos.pos_sec != m_lastTimePos.pos_sec)
            {
                std::list<Event_t>* eventList = &timeWheel->m_eventSlotList[timeWheel->m_timePos.pos_sec + timeWheel->m_firstLevelCount];
                timeWheel->processEvent(*eventList);
                eventList->clear();
            }
            else if (pos.pos_ms != m_lastTimePos.pos_ms)
            {
                std::list<Event_t>* eventList = &timeWheel->m_eventSlotList[timeWheel->m_timePos.pos_ms];
                timeWheel->processEvent(*eventList);
                eventList->clear();
            }
        }
     
    }

    return nullptr;
}


void TimeWheel::initTimeWheel(int steps, int maxMin)
{
    if (1000 % steps != 0){
        printf("invalid steps\n");
        return;
    }
    m_steps = steps;
    m_firstLevelCount = 1000/steps;
    m_thirdLevelCount = maxMin;

    m_eventSlotList.resize(m_firstLevelCount + m_secondLevelCount + m_thirdLevelCount);
    int ret = pthread_create(&m_loopThread, NULL, loopForInterval, this);
    if(ret != 0) {
        printf("create thread error:%s\n", strerror(errno));
        return;
    }
    pthread_join(m_loopThread, NULL);
}

void TimeWheel::createTimingEvent(int interval, EventCallback_t* callback){
    if(interval < m_steps || interval % m_steps != 0 || interval >= m_steps*m_firstLevelCount*m_secondLevelCount*m_thirdLevelCount){
        printf("invalid interval\n");
        return;
    }
    
    Event_t event = {0};
    event.interval = interval;
    event.cb = *callback;
    event.timePos.pos_min = m_timePos.pos_min;
    event.timePos.pos_sec = m_timePos.pos_sec;
    event.timePos.pos_ms = m_timePos.pos_ms;
    event.id = createEventId();  

    //insert it to a slot of TimeWheel
    insertEventToSlot(interval, event);
}
int TimeWheel::createEventId() {
    return m_increaseId++; 
}


void TimeWheel::getTriggerTimeFromInterval(int interval, TimePos_t &timePos) {
    //get current time: ms
    int curTime = getCurrentMs(m_timePos);

    //caculate which slot this interval belong to 
    int futureTime = curTime + interval;

    int minPos =  (futureTime/1000/60)%m_thirdLevelCount;
    int secPos =  ((futureTime/1000)%60);
    int msPos = (futureTime%1000)/m_steps;

    return;
}

int TimeWheel::getCurrentMs(TimePos_t timePos) {
    return m_steps * timePos.pos_ms + timePos.pos_sec*1000 +  timePos.pos_min*60*1000;
}

int TimeWheel::processEvent(std::list<Event_t> &eventList){
    printf("processEvent\n");
    //process the event for current slot
    for(auto event = eventList.begin(); event != eventList.end(); event ++) {
        //caculate the current ms
        int currentMs = getCurrentMs(m_timePos);
        //caculate last  time(ms) this event was processed
        int lastProcessedMs = getCurrentMs(event->timePos);

        //caculate the distance between now and last time(ms)
        int distanceMs = (currentMs - lastProcessedMs + (m_secondLevelCount+1)*60*1000)%((m_secondLevelCount+1)*60*1000);

        //if interval == distanceMs, need process this event
        if (event->interval == distanceMs)
        {
            //process event
            printf("process event\n");
            //get now pos as this event's start point
            event->timePos = m_timePos;
            //add this event to slot
            insertEventToSlot(event->interval, *event);
        }
        else
        {
            // although this event in this positon, but it not arriving timing, it will continue move to next slot caculate by distance ms.
            insertEventToSlot(distanceMs, *event);
        }
    }
    return 0;
}

void TimeWheel::insertEventToSlot(int interval, Event_t event)
{
    TimePos_t timePos = {0};

    //caculate the which slot this event should be set to
    getTriggerTimeFromInterval(interval, timePos);
     {
        std::unique_lock<std::mutex> lock(m_mutex);
        // if not in same
        if (timePos.pos_min != m_timePos.pos_min)
        {
            m_eventSlotList[m_firstLevelCount + m_secondLevelCount + timePos.pos_min].push_back(event);
        }
        else if (timePos.pos_sec != m_timePos.pos_sec)
        {
            m_eventSlotList[m_firstLevelCount + timePos.pos_sec].push_back(event);
        }
        else if (timePos.pos_ms != m_timePos.pos_ms)
        {
            m_eventSlotList[timePos.pos_ms].push_back(event);
        }
     }
    return;
}