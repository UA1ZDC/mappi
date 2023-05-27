#ifndef METEO_COMMONS_UI_CUSTOM_PREPAREWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_PREPAREWIDGET_H

#include <qdialog.h>
#include <qdatetime.h>
#include <qradiobutton.h>

namespace Ui {
class PrepareWidget;
}

class QSpinBox;
class QCheckBox;
class QComboBox;
class HoursEdit;
class StationWidget;

class PrepareWidget : public QDialog
{
  Q_OBJECT

  Q_ENUMS(Mode)

  Q_PROPERTY(Mode mode READ mode WRITE setMode)
  Q_PROPERTY(QDateTime begDt READ beginDateTime WRITE setBeginDateTime)
  Q_PROPERTY(QDateTime endDt READ endDateTime )

  Q_PROPERTY(bool byStation READ isByStation WRITE setByStation )

  Q_PROPERTY(bool stationGroupVisible READ isStationGroupVisible WRITE setStationGroupVisible)

  Q_PROPERTY(StationWidget* stationEdit READ stationEdit )

  Q_PROPERTY(QCheckBox* checkT READ checkT )
  Q_PROPERTY(QCheckBox* checkTd READ checkTd )
  Q_PROPERTY(QCheckBox* checkU READ checkU )
  Q_PROPERTY(QCheckBox* checkP READ checkP )
  Q_PROPERTY(QCheckBox* checkPQNH READ checkPQNH )
  Q_PROPERTY(QCheckBox* checkWind READ checkWind )

  Q_PROPERTY(QRadioButton* analyseRadio READ analyseRadio )
  Q_PROPERTY(QRadioButton* stationRadio READ stationRadio )

  Q_PROPERTY(QComboBox* centerCombo READ centerCombo )
  Q_PROPERTY(QComboBox* levelCombo READ levelCombo )

public:
  enum Mode { kTransfer, kMeteogram, kSloy };

  explicit PrepareWidget(QWidget *parent = 0);
  virtual ~PrepareWidget();

  Mode mode() const { return mode_; }
  void setMode(Mode m) { mode_ = m; setupGui(); }

  QDateTime beginDateTime() const;
  void setBeginDateTime(const QDateTime& dt);

  QDateTime endDateTime() const;
  void setEndDateTime(const QDateTime& dt);

  bool isByStation() const;
  void setByStation(bool v);

  QCheckBox* checkT() const;
  QCheckBox* checkTd() const;
  QCheckBox* checkU() const;
  QCheckBox* checkP() const;
  QCheckBox* checkPQNH() const;
  QCheckBox* checkWind() const;

  StationWidget* stationEdit() const;
  StationWidget* setStationWidget(StationWidget* w);

  bool isForecastTime() const;

  QRadioButton* analyseRadio() const;
  QRadioButton* stationRadio() const;

  QComboBox* centerCombo() const;
  QComboBox* levelCombo() const;

  bool isStationGroupVisible() const;
  void setStationGroupVisible(bool visible);

  void setMargins(int left, int top, int right, int bottom);

signals:
  void dateTimeChanged(const QDateTime& beg, const QDateTime& end, bool forecastTime);
  void paramChanged();

  void run();
  void close();

private slots:
  void slotDateTimeChanged();
  void slotUpdateAnalyseState(bool toggled);

private:
  void setupGui();

private:
  Mode mode_;
  Ui::PrepareWidget* ui_;
};

#endif // METEO_COMMONS_UI_CUSTOM_PREPAREWIDGET_H
