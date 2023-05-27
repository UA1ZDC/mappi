#ifndef METEO_COMMONS_UI_AV12CORE_AVAILABLESTATIONS_H
#define METEO_COMMONS_UI_AV12CORE_AVAILABLESTATIONS_H

#include <QWidget>
#include <qdatetime.h>
#include <meteo/commons/proto/surface_service.pb.h>

namespace Ui {
class AvailableStations;
}

namespace meteo {
namespace product {


class AvailableStations : public QWidget
{
  Q_OBJECT
public:
  enum Columns {
    kIndexColumn,
    kLatColumn,
    kLonColumn,
    kNameColumn,
    kColumnCount
  };
  explicit AvailableStations(QWidget *parent = nullptr);
  ~AvailableStations();

  void setDt(const QDateTime& dt);
  void fill();

signals:
  void addStation(surf::Point station);

private slots:
  void slotEmitStations();

private:
  Ui::AvailableStations *ui_ = nullptr;
  QDateTime dt_;
};

}
}

#endif // METEO_PRODUCT_UI_AV12PLUGIN_AVAILABLESTATIONS_H
