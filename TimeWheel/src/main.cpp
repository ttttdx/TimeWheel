#include <iostream>
#include <functional>
#include "Slot.h"
using namespace std;

void fun100()
{
    cout << "func 100" << endl;
}
void fun200()
{
    cout << "func 200" << endl;
}
void fun500()
{
    cout << "func 500" << endl;
}

void fun1500()
{
    cout << "func 1500" << endl;
}

int main()
{
    TimeWheel wheel;
    wheel.initTimeWheel(100, 10);
    wheel.createTimingEvent(200, NULL);
    while (1)
    {
        /* code */
    }
    
    
}