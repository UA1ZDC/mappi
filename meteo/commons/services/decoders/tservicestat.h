#ifndef TSERVICE_STATUS_H
#define TSERVICE_STATUS_H

#include <QObject>
#include <qdatetime.h>
#include <qmap.h>
#include <qpair.h>

#include <meteo/commons/proto/state.pb.h>
#include <cross-commons/debug/tlog.h>

Q_DECLARE_METATYPE(::meteo::app::OperationStatus);

namespace service {

  enum StatFormat {
    kSimple = 1,
    kList   = 2
  };

  struct StatDescription {
    int value;    //!< Количественное значение
    meteo::app::OperationState state; //!< Состояние
    QDateTime dt; //!< Дата/время последнего изменения

    StatDescription(int val): value(val) {}
    StatDescription():value(0) {}
  };

  //!   Описание состояния сервиса (количественное).
  /*! 
    Kind - набор объектов, меняющих свое состояния (тип обрабатываемой информации, тип
    запроса, и т.п. ). 
    StatType - набор состояний, которые могут иметь объекты Kind.  
    Заполнение состояния происходит в том же порядке, в каком задавалось его описание функцией set()
  */

  template <class Kind, class StatType> class Status {
  public:
    struct Format {
      QString text;
      Kind kind;
      QList<StatType> type;
      StatFormat format;

      Format(const QString& atext, Kind akind, StatType atype, StatFormat aformat):text(atext), kind(akind),format(aformat) {
	type << atype;
      }
      Format(const QString& atext, Kind akind, const QList<StatType>& atype, StatFormat aformat):
	text(atext), kind(akind), type(atype), format(aformat) {}
      Format() {}
    };

    //! Установка описания состояния
    void set(Kind kind, StatType type, meteo::app::OperationState state, const QString& str) {
      _st[kind][type].state = state;
      _st[kind][type].dt = QDateTime::currentDateTime();
      // keys << QPair<Kind, StatType>(kind, type);
      _keys << Format(str, kind, type, kSimple);
    }
    void set(Kind kind, const QList<StatType>& type, meteo::app::OperationState state, const QString& str) {
      for (int idx = 0; idx < type.size(); idx++) {
	_st[kind][type.at(idx)].state = state;
	_st[kind][type.at(idx)].dt = QDateTime::currentDateTime();
      }
      // keys << QPair<Kind, StatType>(kind, type);
      _keys << Format(str, kind, type, kList);
    }

    //! Добавление значения описанию
    void add(Kind kind, StatType type, meteo::app::OperationState state, int cnt = 1) {
      if (_st.contains(kind) && _st.value(kind).contains(type)) {
	_st[kind][type].value += cnt;
	_st[kind][type].state = state;
	_st[kind][type].dt = QDateTime::currentDateTime();
      }
    }

    void createStatus(const Format& sf, meteo::app::OperationParam* param) const {
      if (param == 0 || sf.type.size() < 1) return;

      QString res;
      QDateTime dt;
      meteo::app::OperationState state;

      switch (sf.format) {
      case kSimple:
	if (sf.type.size() == 1) {
	  StatDescription d = _st.value(sf.kind).value(sf.type.at(0));
	  state = d.state;
	  dt = d.dt;
	  if (state != meteo::app::OperationState_NONE) {
	    res = QString::number(d.value);
	  } else {
	    res = "";
	  }
	}
	break;
      case kList:
	float ratio;
	for (int idx = 0; idx < sf.type.size(); idx++) {
	  StatDescription d = _st.value(sf.kind).value(sf.type.at(idx));

	  if (idx == 0) {
	    state = d.state;
	    dt = d.dt;
	    ratio = d.value;
	  } else {
	    if (d.value == 0) {
	      ratio = 0;
	    } else {
	      ratio /= d.value;
	    }
	  }

	  if (d.state != meteo::app::OperationState_NONE) {
	    res += QString::number(d.value) + "/";
	  } else {
	    state = d.state;
	    dt = d.dt;
	    res = "";
	    break;
	  }
	}
	if (res.size() > 1) {
	  res += QString::number(ratio, 'f', 2);
	  //res.remove(-1, 1);
	}

	break;
      default: {}
      }

      if (res.isNull()) {
	debug_log << QObject::tr("Не верно заданы параметры для отображения состояния");
      } else {
	param->set_value(res.toStdString());
	param->set_dt(dt.toString("dd.MM.yy hh:mm").toUtf8().constData());
	param->set_state(state);  
      }
    }

    //! Заполнение состояния
    void fillStatus(meteo::app::OperationStatus* status) const {
      if (!status) return;

      for (int i = 0; i < _keys.count(); i++) {
	meteo::app::OperationParam* param = status->add_param();

	param->set_title(_keys.at(i).text.toUtf8().constData());
	createStatus(_keys.at(i), param);
      }
    }

    //! Заполнение состояния только одного типа
    void fillStatus(Kind kind, meteo::app::OperationStatus* status) const {
      if (!status) return;

      for (int i = 0; i < _keys.count(); i++) {
    	if (_keys.at(i).kind != kind) continue;

    	meteo::app::OperationParam* param = status->add_param();
	param->set_title(_keys.at(i).text.toUtf8().constData());
	createStatus(_keys.at(i), param);
      }
    }
    
    private:
    QMap<Kind, QMap<StatType, StatDescription> > _st;
    //    QList<QPair<Kind, StatType> > keys;
    QList<Format> _keys;
  };
}

//! отправка статуса по таймеру
class TServiceStatus :  public QObject {
Q_OBJECT
public:
  TServiceStatus();
  virtual ~TServiceStatus();

  virtual bool isNeedUpdate() { return false; }
  virtual void setup() = 0;
  virtual void setAppId(meteo::app::OperationStatus* status) const = 0;
  virtual void send(const meteo::app::OperationStatus& status) = 0;
  
public slots:
  void init() { setup(); };
  void sendStatus(const meteo::app::OperationStatus& status) { send(status); }

signals:
  void updateStatus();

};


#endif
