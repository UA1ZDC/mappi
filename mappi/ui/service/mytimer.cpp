#include "include/mytimer.h"
#include "include/console.h"

myTimer::myTimer(QObject *parent) : QTimer(parent)
{
}

void myTimer::timerAction()
{

}

void myTimer::timerStart()
{
    this->start();
}
