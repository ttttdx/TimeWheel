#include "Slot.h"

#include <iostream>
#include <memory.h>
#include <chrono>
#include <thread>

TimeWheel::TimeWheel():m_steps(0),m_firstLevelCount(0),m_secondLevelCount(60),m_thirdLevelCount(0)
{}

// can not allocate 1000ms to this slot
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

    m_slotList.resize(m_firstLevelCount+m_secondLevelCount+m_thirdLevelCount);
    int ret = pthread_create(&m_loopThread, NULL, loopForInterval, this);
    if(ret != 0) {
        printf("create thread error:%s\n", strerror(errno));
        return;
    }

    pthread_join(m_loopThread, NULL);


}