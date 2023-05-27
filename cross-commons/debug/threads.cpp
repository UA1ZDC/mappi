#include "threads.h"


ThreadCounter* gThreadCounter()
{
  static ThreadCounter* threadCounter = new ThreadCounter;
  return threadCounter;
}
