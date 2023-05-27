#ifndef METEO_NOVOST_UI_COLORALERT_COLORALERTWIDGET_H
#define METEO_NOVOST_UI_COLORALERT_COLORALERTWIDGET_H

#include <qwidget.h>
#include <meteo/commons/proto/meteotablo.pb.h>

class QDate;
class QDateTime;
class QFileSystemWatcher;

namespace Ui {
class ColorAlertWidget;
}

namespace meteo {

class ColorAlertWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ColorAlertWidget(QWidget *parent = nullptr);
  ~ColorAlertWidget();

  int radius() const;

  void activate()     { blockGui(false); }
  void deactivate()   { blockGui(true);  }
  void blockGui(bool block);
  bool isGuiBlocked() const;

  void setAlpha(int percent);
  void checkParams(bool checked);

  void setDataInfo(const QString &);
  QString descrUndeCursor() const;

  tablo::Settings pattern() const;
  QStringList paramList() const;

signals:
  void changed(const tablo::Settings& pattern);

public slots:
  void slotRadiusChanged(int value);
  void slotAlphaChanged(int value);
  void slotEmitChanged();
  void slotLoadSettings();
  void slotDistChanged(int );

private:
  void updateParamList(const QStringList& newParams);
  QStringList checkedParams() const;

private:
  Ui::ColorAlertWidget* ui_;
  QFileSystemWatcher* watcher_;
};

} // meteo

#endif // METEO_NOVOST_UI_COLORALERT_COLORALERTWIDGET_H
