#ifndef MAPPI_MMIRZ_RES_H
#define MAPPI_MMIRZ_RES_H

#include <qdialog.h>
#include <coords.h>

namespace Ui {
  class MmirzRes;
}

namespace mappi {

    class MmirzRes : public QDialog {
      Q_OBJECT
    public:
      MmirzRes(QWidget* parent = 0);
      ~MmirzRes();
      void setColumns(QStringList &str);
      void setRows(QStringList& str);
      void setKA(const QString &str);
      void addItem(int row, const QStringList &list);
      void setDt(const QDateTime& dtbeg,const QDateTime& dtend);
      void setRowCount(int num);

    private:
      Ui::MmirzRes* _ui;
    };

}



#endif
