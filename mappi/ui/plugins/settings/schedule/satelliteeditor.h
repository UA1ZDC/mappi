#ifndef MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SATELLITEEDITOR_H
#define MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SATELLITEEDITOR_H

#include <qdialog.h>
#include <qmap.h>

#include <mappi/proto/reception.pb.h>

class QTableWidget;
class QTableWidgetItem;

namespace Ui {
  class SatelliteEditor;
}

namespace mappi {

class ReceiverEditor;

class SatelliteEditor : public QDialog
{
  Q_OBJECT
  enum Column { kMode, kFreq, kGain, kRate };

public:
  explicit SatelliteEditor(QWidget *parent = 0);
  ~SatelliteEditor();

signals:
  void saveSatellite(const conf::ReceptionParam& satellite);

public slots:
  void open(const conf::ReceptionParam& sat, QMap<mappi::conf::SatType, QString>& satType, const QStringList& existSat);

private slots:
  void slotAddReceiver();
  void slotRemoveReceiver();
  void slotSaveReceiver(const conf::ReceiverParam& recv);
  void slotSaveSatellite();
  void slotEditReceiver(int row, int col);

private:
  void loadSettings(const QString& filename);
  void saveSettings(const QString& filename) const;

private:
  Ui::SatelliteEditor*                ui_;
  ReceiverEditor*                     receiverEditor_;
  QMap<mappi::conf::SatType, QString> satType_;
  conf::ReceptionParam                       satellite_;
  QList<conf::ReceiverParam>              receivers_;
  conf::ReceiverParam*                    editRecv;
  int                                 editRow_;
  bool                                loadedSettings_ = false;

private:
  static void insertRow(QTableWidget* table, const conf::ReceiverParam& recv);
  static void fillRow(QTableWidget* table, int row, const conf::ReceiverParam& recv);

  static QTableWidgetItem* createCell(Column col, const conf::ReceiverParam& recv);
  static void fillCell(QTableWidgetItem *item, Column col, const conf::ReceiverParam& recv) ;
};

} //mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SATELLITEEDITOR_H
