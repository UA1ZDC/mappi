#ifndef TCONTROL_METHODS_H
#define TCONTROL_METHODS_H

#include <qhash.h>
#include <qstringlist.h>
#include <qdom.h>

#include <cross-commons/singleton/tsingleton.h>
#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>

class QDomDocument;
class QDomElement;
class TMeteoData;
class QString;

//#define PARSE_DEBUG_LOG

#define CONTROL_SETTINGS_DIR MnCommon::etcPath("meteo") + "/control/"

namespace control {
  enum CheckValueType {
    CODEFORM_CHECK = 0, //!< значение кодовой формы и для match, и для val
    PHYSVAL_CHECK = 1,  //!< величины и для match, и для val
    VAL_VALID = 2,	//!< значение валидное, не отсутсвующее и пр.
  };
}


class TControlMethodBase {
public:
  TControlMethodBase(const QString& dirName);
  virtual ~TControlMethodBase();

  virtual bool control(TMeteoData* data) = 0;
  virtual bool control(TMeteoData* cur, TMeteoData* prev) { Q_UNUSED(cur); Q_UNUSED(prev); return false; }
  virtual QString key() = 0;

  bool isReadSettings() {return _isReadSettings; }

protected:
  virtual bool readSettings();
  virtual bool readSettings(const QString& fileName);
  virtual bool readSettings(QDomDocument* ) { trc; error_log << "Func not realized"; return false;}

  void parseNum(const QStringList& strVal, QList<float>& val);
  void parseSegments(const QStringList& vals, QList<QPair<float, float> >& segments);

  virtual bool rulesMatch(const QPair<descr_t,QStringList>& type, TMeteoData* data);
  bool segmentMatch(const QList<QPair<float, float> >& segment, float value);
private:
  bool _isReadSettings;
  QString _dirName;
};


class TControlMethod {
public:
  TControlMethod();
  virtual ~TControlMethod();
  
  virtual bool control(TMeteoData* data);
  virtual bool control(TMeteoData* cur, TMeteoData* prev);
  bool addMethod(const QString& key, TControlMethodBase* method);

private:
  QHash<QString, TControlMethodBase*> _methods; //!< методы по имени для одного типа контроля
};




#endif
