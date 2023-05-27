#ifndef METEO_COMMONS_UI_METEOTABLO_TABLOWIDGET_H
#define METEO_COMMONS_UI_METEOTABLO_TABLOWIDGET_H

#include <qhash.h>
#include <qtreewidget.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/meteotablo.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/puanson.h>

class TMeteoData;

namespace meteo {

namespace surf {
  class DataReply;
}

struct ValueDescr
{
  QString placeId;
  QString descr;
  double  value;
  QDateTime dt;
  QString text;
};

class TabloWidget : public QTreeWidget
{
  Q_OBJECT
public:
  enum Role { kMeteoDescrNameRole = Qt::UserRole + 1,
              kColumnTypeRole,
              kDataTypeRole,
              kRoleAlert,
              kRoleMeteoData,
              kRoleMeteoParamValue,
              kRoleAlert2,
              kRoleAlert3,
              kRoleRealDescriptor,
              kRoleDateTime
            };
  enum ColumnType { kColumnId,
                    kColumnName,
                    kColumnTime,
                    kColumnMeteoParam,
                  };

  explicit TabloWidget(QWidget* parent = 0);

  void setData(surf::DataReply* resp);

  QList<int> dataTypes() const;
  QStringList descriptors() const;
  QList<qint64> descriptorIds() const;
  QList<int> stations(int dataType = -1) const;
  QStringList airports(int dataType = -1) const;

  void adjustColumns(int offset = 0);

  tablo::Settings save() const;
  void restore(const tablo::Settings& opt);

  QTreeWidgetItem* findItem(int column, const QDateTime& dt) const;
  QTreeWidgetItem* findItem(int column, const QString& text) const;
//  QTreeWidgetItem* findItem(int column, const QVariant& data, int role) const;

  int findColumn(const QVariant& data, int role) const;

  void addMeteoparam(const QString& param);
  void setupHeader(int column, const QString& param);

  QList<ValueDescr> values() const;
  void setDescription(const QDateTime& dt, const QString& descrStr, const QString& text);
  void setDescription(const QString& placeId, const QString& descrStr, const QString& text);

  bool loadAlerts();
  void checkAlerts();
  void clearAlerts();

  void setArchive(bool archive){ isArchive_ = archive;}
  bool isArchive(){ return isArchive_;}

signals:
  void itemMoved();
  void placeAdded();
  void paramChanged();
  void settingsChanged();

public slots:
  void slotAddPlace();

protected:
  virtual void contextMenuEvent(QContextMenuEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);


private:
  void resetHeader();
  QString getMeteoparam();
  void setRowForeground(int row, const QBrush& brush);
  void setRowBackground(int row, const QBrush& brush);
  void sortMeteodataByDt(surf::DataReply* resp);

private:
  QHash<QString,::meteo::puanson::proto::CellRule>  templParams_; //!< метеопараметр и правило его отображения
  map::Puanson punchStub_;
  QTreeWidgetItem* movingItem_ = nullptr;
  bool moved_ = false;
  bool isArchive_ = false;
};

}

#endif
