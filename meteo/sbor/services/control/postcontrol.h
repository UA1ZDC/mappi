#ifndef METEO_SBOR_SERVICES_CONTROL_POSTCONTROL_H
#define METEO_SBOR_SERVICES_CONTROL_POSTCONTROL_H

#include <qstring.h>
#include <qmap.h>
#include <qdatetime.h>

class TMeteoData;

namespace meteo {
  class Dbi;
  class ConnectProp;
  //!контроль данных записанных в БД
  class PostControl {
  public:
    //!ключ для выборки
    struct DataKey {
      QString station; //!< станция
      int type; //!< тип станции
      QDateTime dt; //!< дата/время измерения
      DataKey(const QString& astation, int atype, const QDateTime& adt) :
        station(astation), type(atype), dt(adt) {}

      bool operator<(const DataKey& other) const {
        if (type < other.type) return true;
        if (dt < other.dt) return true;
        if (station < other.station) return true;
        return false;
      }
    };



    PostControl();
    ~PostControl();

    bool timeControl(const QDateTime& dateStart, const QDateTime& dateEnd);

  private:
    void init();

    bool timeGetData(const QDateTime& dtStart, const QDateTime& dtEnd,
                     QMap< PostControl::DataKey, TMeteoData >* md);
    bool timePrevData(const QString& descrs, const PostControl::DataKey& key, TMeteoData* md);
    bool timeControl(TMeteoData* cur, TMeteoData* prev);
    QString createUpdateParam(const TMeteoData& md);
    bool timeControlUpdate(const QString& updparam);

  private:
    meteo::Dbi* db_ = nullptr;
  };
}

#endif
