#ifndef CHANNELEDITOR_H
#define CHANNELEDITOR_H

#include <qdialog.h>
#include <qmap.h>
#include <qset.h>

#include <mappi/proto/satellite.pb.h>

namespace Ui {
  class ChannelEditor;
}

namespace mappi {

class ChannelEditor : public QDialog
{
  Q_OBJECT
  using InstrType = conf::InstrumentType;
  using ChKey = QPair<QString, InstrType>;

public:
  struct Channel {
    QString satellite;
    QString instrument;
    QString channel;
  };

public:
  explicit ChannelEditor(QWidget *parent = 0);
  ~ChannelEditor();

  void open(const Channel& channel);
  void init();

signals:
  void complite(const Channel&);

public slots:
  void slotOpen();

private slots:
  void slotSatChange(const QString& satellite);
  void slotInstrChange(const QString& satellite);
  void slotAdd();

private:
  Ui::ChannelEditor *ui_;

  Channel                        channel_;
  QSet<QString>                  satellites_;       //!< Список спутников
  QMap<QString, QSet<InstrType>> instruments_;      //!< Список приборов для каждого спутника
  QMap<ChKey,   QSet<QString>>   channels_;         //!< Список каналов для каждой пары спутник-прибор

private:
  static QMap<InstrType, QString> instrumentsName_;   //!< Отображение имени инструмента в его тип

private:
  static bool isValid(const conf::Instrument& instrument);
};

} // mappi

#endif // CHANNELEDITOR_H
