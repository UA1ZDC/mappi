#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <qabstractitemmodel.h>
#include <QTreeWidgetItem>

#include <meteo/commons/proto/map_service.pb.h>
#include <QMenu>

namespace Ui {
class DocTypeFilter;
}

class DocTypeFilter : public QWidget
{
  Q_OBJECT

public:
  explicit DocTypeFilter(QWidget *parent = nullptr);
  ~DocTypeFilter();
  QStringList getSelectedDocumentTypes();
  QStringList getSelectedMapTypes();

signals:
  void signalMapTypesReceived(QStringList mapTypes);
  void signalDocumentTypesReceived(QStringList documentTypes);

private slots:
  void slotMapTypesReceived(QStringList mapTypes);
  void slotDocumentTypesReceived(QStringList documentTypes);
  void slotSelectionChanged();

private:
  void addItem(QTreeWidgetItem* item, const QString& title);
  void getMapTypes();
  void getDocumentTypes();

private:
  QTreeWidgetItem* mapTopLevelItem;
  QList<QTreeWidgetItem*> mapTypes;

  QTreeWidgetItem* documentTopLevelItem;
  QList<QTreeWidgetItem*> documentTypes;

  Ui::DocTypeFilter *ui_;



};

#endif // FILTERDIALOG_H
