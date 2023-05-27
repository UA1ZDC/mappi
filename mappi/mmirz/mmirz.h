#ifndef MAPPI_MMIRZ_H
#define MAPPI_MMIRZ_H

#include <qdialog.h>
#include <coords.h>

namespace Ui {
  class Mmirz;
}


namespace mappi {

class MmirzRes;
class MmirzGraph;
  
    class MmirzWidget : public QDialog {
      Q_OBJECT
    public:
      MmirzWidget(QWidget* parent = 0);
      ~MmirzWidget();


    public slots:
      void cancel();
      void apply();
      void slotAddCity();
      void slotAddTime();
      void slotRmCity();
      void slotRmTime();

    private:
      void initWidgets();
      void clear();
      void keyPressEvent( QKeyEvent * e);
      void closeEvent(QCloseEvent *);
      QStringList calc(const Coords::GeoCoord& site, const QDateTime& dtBeg, const QDateTime& dtEnd, const QString& satName, const QString& text);
      QStringList calcCustom(const Coords::GeoCoord& site, const QDateTime& dtBeg,  const QDateTime& dtEnd, const QString& satName, const QString& text, QList<QTime> times);
      QStringList dmsFromString(const QString& s);
      float toDecDeg(QStringList list);


    private:
      Ui::Mmirz* _ui;
      mappi::MmirzRes* _uires;
      mappi::MmirzGraph* _uigraph;
    };

}



#endif
