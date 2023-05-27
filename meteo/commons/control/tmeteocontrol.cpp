#include "tmeteocontrol.h"
#include "tcontrolmethods.h"

#include <cross-commons/debug/tlog.h>

namespace control {
  const float gamma = 0.0065;
  const float Rd = 287;
  const float g = 9.808;
  const float R = 0.287;
  const float Cp = 1.005; //удельная теплоёмкость при пост. давлении. сухой воздух = 1.005; 100% влажность = 1.0301 
}



template<> tmcontrol::TMeteoControl* TMeteoControl::_instance = 0;

namespace tmcontrol {

TMeteoControl::TMeteoControl()
{
}

TMeteoControl::~TMeteoControl()
{
  QMapIterator<control::ControlType, TControlMethod*> it(_methods);
  while (it.hasNext()) {
    it.next();
    TControlMethod* m = it.value();
    delete m;
  }
}

//! return - false - хотя бы при одном из методов контроля возникли ошибки
bool TMeteoControl::control(ushort ctrlMask, TMeteoData* data)
{
  if (!data) return false;

  bool ok = true;

  if (0 != (ctrlMask & control::LIMIT_CTRL)) {
    if (_methods.contains(control::LIMIT_CTRL)) {
      ok &= _methods[control::LIMIT_CTRL]->control(data);
    }
  }

  if (0 != (ctrlMask & control::CONSISTENCY_CTRL)) {
    if (_methods.contains(control::CONSISTENCY_CTRL)) {
      ok &= _methods[control::CONSISTENCY_CTRL]->control(data);
    }
  }

  if (0 != (ctrlMask & control::CONSISTENCY_SIGN)) {
    if (_methods.contains(control::CONSISTENCY_SIGN)) {
      ok &= _methods[control::CONSISTENCY_SIGN]->control(data);
    }
  }  

  return ok;
}

//! return - false - хотя бы при одном из методов контроля возникли ошибки
bool TMeteoControl::control(ushort ctrlMask, TMeteoData* cur, TMeteoData* prev)
{
  if (!cur || !prev) return false;

  bool ok = true;

  if (0 != (ctrlMask & control::TIME_CTRL)) {
    if (_methods.contains(control::TIME_CTRL)) {
      ok &= _methods[control::TIME_CTRL]->control(cur, prev);
    }
  }

  return ok;
}

bool TMeteoControl::registerMethod(control::ControlType type, TControlMethodBase* method)
{
  TControlMethod* m = 0;

  if (!_methods.contains(type)) {
    m = new TControlMethod;
    _methods.insert(type, m);
    // debug_log<< "add new method type="<<type;
  } else {
    m = _methods[type];
    //debug_log<<"exist method type="<<type;
  }
  return m->addMethod(method->key(), method);

}

}
