#pragma once

#include "context.h"
#include <mappi/global/streamheader.h>


namespace mappi {

namespace receiver {

/**
 * Файловое хранилище:
 * - создание целевого raw-файл;
 * - наведение ссылки на raw-файл.
 * - дописывание StreamHeader.
 *
 * Базовый пути берутся из конфигурации.
 */
class FileStorage
{
public :
  FileStorage(Context* ctx);
  ~FileStorage();

  bool create() const;
  bool setHeader(const meteo::global::StreamHeader& header) const;

  QString sessionFile() const;

private :
  Context* ctx_;
};

}

}
