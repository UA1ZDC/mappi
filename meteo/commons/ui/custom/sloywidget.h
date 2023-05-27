#ifndef METEO_COMMONS_UI_CUSTOM_SLOYWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_SLOYWIDGET_H

#include <qwidget.h>
#include <qdatetime.h>
#include <QtScript/qscriptable.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptengine.h>

#include <meteo/commons/proto/surface_service.pb.h>

namespace Ui {
class SloyWidget;
}

class StationWidget;
class QComboBox;

class SloyWidget : public QWidget, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY( StationWidget* station READ stationWidget WRITE setExternalStationWidget )
  Q_PROPERTY( QComboBox* centerCombo READ centerCombo )

  Q_PROPERTY( QDateTime observationDt READ observationDt WRITE setObservationDt )
  Q_PROPERTY( int postNum READ postNum WRITE setPostNum )
  Q_PROPERTY( int postAlt READ postAlt WRITE setPostAlt )

  Q_PROPERTY( int center READ center )
  Q_PROPERTY( bool byStation READ isByStation WRITE setByStation )
  Q_PROPERTY( bool byAnalyse READ isByAnalyse WRITE setByAnalyse )

  Q_PROPERTY( bool postGrVisible READ isPostGrVisible WRITE setPostGrVisible )
  Q_PROPERTY( bool postAltVisible READ isPostAltVisible WRITE setPostAltVisible )
  Q_PROPERTY( bool postNumVisible READ isPostNumVisible WRITE setPostNumVisible )

public:
  enum Role { kCenterNameRole = Qt::DisplayRole, kCenterIdRole = Qt::UserRole };

  explicit SloyWidget(QWidget *parent = 0);
  ~SloyWidget();

  StationWidget* stationWidget() const { return stationWidget_; }
  void setExternalStationWidget(StationWidget* w);

  QComboBox* centerCombo() const;

  QDateTime observationDt() const;
  void setObservationDt(const QDateTime& dt);

  int center() const;

  bool isByStation() const;
  void setByStation(bool check);

  bool isByAnalyse() const;
  void setByAnalyse(bool check);

  bool isPostGrVisible() const;
  void setPostGrVisible(bool visible);

  bool isPostAltVisible() const;
  void setPostAltVisible(bool visible);

  bool isPostNumVisible() const;
  void setPostNumVisible(bool visible);

  int postNum() const;
  void setPostNum(int num);

  int postAlt() const;
  void setPostAlt(int alt);

  Q_INVOKABLE void setZondLabel(const QString& text);
  Q_INVOKABLE void setData(const QScriptValue& data);
  Q_INVOKABLE QScriptValue zondData() const;

  Q_INVOKABLE void hideZondColumn(int column);
  Q_INVOKABLE void showZondColumn(int column);

public slots:
  void slotSetCenters(const QStringList& names, const QList<int>& ids);

signals:
  void changed();

private:
  // служебные
  Ui::SloyWidget* ui_;
  StationWidget* stationWidget_;
};

#endif // METEO_COMMONS_UI_CUSTOM_SLOYWIDGET_H
