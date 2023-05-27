#ifndef COMMONS_OBANAL_TFFIELDPARAMS_H
#define COMMONS_OBANAL_TFFIELDPARAMS_H

#include <qmap.h>

#include <cross-commons/singleton/tsingleton.h>
#include <commons/proto/tfield.pb.h>

namespace obanal {

class TField;

class TFieldParams
{
private:
  TFieldParams();
  ~TFieldParams();

public:
  void setFieldParams(TField* field);

  bool hasDescriptor(const int64_t descr) const { return params_.contains(descr); }

  float minvalue(const int64_t descr, const int level, const int type_level, bool* ok = nullptr) const;
  float maxvalue(const int64_t descr, const int level, const int type_level, bool* ok = nullptr) const;
  float step    (const int64_t descr, const int level, const int type_level, bool* ok = nullptr) const;

private:
  QMap<int64_t, proto::TField> params_;
  void loadParams();

  friend class TSingleton<TFieldParams>;
};

namespace singleton {
  typedef TSingleton<obanal::TFieldParams> TFieldParams;
}

}

#endif
