#ifndef MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_RECEIVEREDITOR_H
#define MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_RECEIVEREDITOR_H

#include <qdialog.h>
#include <qmap.h>

#include <mappi/proto/reception.pb.h>

namespace Ui {
class ReceiverEditor;
}

namespace mappi {

class ReceiverEditor : public QDialog
{
  Q_OBJECT

public:
  explicit ReceiverEditor(QWidget *parent = 0);
  ~ReceiverEditor();

signals:
  void saveReceiver(const conf::ReceiverParam& recv);

public slots:
  void open(const conf::ReceiverParam& recv, const QMap<mappi::conf::RateMode, QString>& recvMode);

private slots:
  void slotSaveReceiver();

private:
  Ui::ReceiverEditor* ui_;
  conf::ReceiverParam recv_;
  QMap<mappi::conf::RateMode, QString> recvMode_;
};

}

#endif // MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_RECEIVEREDITOR_H
