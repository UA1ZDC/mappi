#ifndef METEO_COMMONS_UI_CUSTOM_ZONDWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_ZONDWIDGET_H

#include <qtableview.h>
#include <qidentityproxymodel.h>

#include <meteo/commons/proto/surface_service.pb.h>

class QStandardItem;
class QStandardItemModel;


class ZondWidget : public QTableView
{
  Q_OBJECT

public:
  enum Column { kLvlType,
                kP,
                kH,
                kT,
                kTd,
                kD,
                kdd,
                kff,
              };

  enum Role { kQualRole = Qt::UserRole + 26 };

  ZondWidget(QWidget* parent = 0);

  void setLabel(const QString& text);

  int rowCount() const;
  int columnCount() const;

public slots:
  void setData(const meteo::surf::OneZondValueOnStation& data);
  meteo::surf::OneZondValueOnStation toZond() const;

  void slotItemChanged(QStandardItem* item);

protected:
  void contextMenuEvent(QContextMenuEvent *e);

private:
  void setModel(QAbstractItemModel *model) { QTableView::setModel(model); }

  int appendRow();
  void removeRows();

  double toFixed(double v, int w) const;


private:
  // параметры
  QList<int> lvlType_;

  // данные
  meteo::surf::OneZondValueOnStation  zond_;
  QStandardItemModel* model_;

  // служебные
  bool noData_;
};


class DataProxyModel : public QIdentityProxyModel
{
public:
  DataProxyModel(QObject* parent = 0);

  QVariant data(const QModelIndex &index, int role) const;
};

#endif // METEO_COMMONS_UI_CUSTOM_ZONDWIDGET_H
