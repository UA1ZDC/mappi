#include "themformat.h"

#include <commons/textproto/tprototext.h>
#include <cross-commons/debug/tlog.h>

using namespace mappi;
using namespace to;

namespace mappi {
  namespace to {
    namespace singleton {
      template<> mappi::to::ThemFormat* ThemFormat::_instance = 0;
    }
  }
}

ThemFormat::ThemFormat()
{
}

ThemFormat::~ThemFormat()
{
}

bool ThemFormat::registerThemHandler(conf::ThemType type, CreateThem func)
{
  if ( true == _themHandler.contains(type) ) {
    warning_log << QObject::tr("Обработчик для тематической обработки с типом %1 уже установлен").arg(type);
    return false;
  }

  _themHandler.insert(type, func);
  return true;
}

ThemAlg* ThemFormat::createThemAlg(conf::ThemType type,const std::string& themname, QSharedPointer<to::DataStore>& store) const
{
  if ( !_themHandler.contains(type)) {
    error_log << QObject::tr("Тип %1 не зарегистрирован").arg(type);
    return nullptr;
  }
  return _themHandler[type](type,themname, store);
}
