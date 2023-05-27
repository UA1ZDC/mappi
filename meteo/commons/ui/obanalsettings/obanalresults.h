#ifndef METEO_COMMONS_UI_OBANALSETTINGS_OBANALRESULT_H
#define METEO_COMMONS_UI_OBANALSETTINGS_OBANALRESULT_H

#include <QWidget>

template <typename T> class QList;

namespace Ui {
  class ObanalResultsWidget;
} // Ui

namespace meteo {
namespace field {
  class DataDesc;
} // field

class ObanalResults : public QWidget
{
  Q_OBJECT

  enum column_t {
    DATE = 0,
    DESCR,
    HOUR,
    MODEL,
    LEVEL,
    CENTER,
    TYPE_NET,
    TYPE_LEVEL,
    COUNT_POINT,
    MAX_COLUMN_NUMBER
  };

public:
  explicit ObanalResults(QWidget* parent);
  ~ObanalResults();


protected:
  void showEvent( QShowEvent* e );

private slots:
  void slotUpdate();

private:
  void renderDatas(const QList<field::DataDesc>& datas);

private:
  Ui::ObanalResultsWidget* ui_;

};

} // meteo

#endif // METEO_COMMONS_UI_OBANALSETTINGS_OBANALRESULT_H
