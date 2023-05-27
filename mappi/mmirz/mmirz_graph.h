#ifndef MAPPI_MMIRZ_GRAPH_H
#define MAPPI_MMIRZ_GRAPH_H

#include <qdialog.h>
#include <coords.h>

class Widget;
namespace Ui {
  class MmirzGraph;
}

namespace mappi {
  
    class MmirzGraph : public QDialog {
      Q_OBJECT
    public:
      MmirzGraph(QWidget* parent = 0);
      ~MmirzGraph();
      void setDt(const QDateTime &dtbeg, const QDateTime &dtend);
      void setKA(const QString& str);
      void grafik(QVector<QStringList> list, QStringList times, QVector<QString> cities);
      void setupBars(QVector<QString> cities);

    private:
      Ui::MmirzGraph* _ui;
      Widget *wgt;

    };

}



#endif
