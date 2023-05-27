#include "engine.h"


namespace meteo {

namespace auth {

Engine::Engine()
{
}

Engine::~Engine()
{
}

QString Engine::lastError()
{
  QString tmp(lastError_);
  lastError_.clear();

  return tmp;
}

void Engine::setLastError(const QString& msg)
{
  lastError_ = msg;
}

bool Engine::hasError() const
{
  return (!lastError_.isEmpty());
}

}

}
