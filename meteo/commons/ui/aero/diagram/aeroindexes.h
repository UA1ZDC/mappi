#ifndef METEO_COMMONS_UI_AERODIAGRAM_AEROCORE_AEROINDEXES_H
#define METEO_COMMONS_UI_AERODIAGRAM_AEROCORE_AEROINDEXES_H

#include <qdialog.h>

namespace Ui {
  class AeroIndexes;
}

namespace zond {
  class Zond;
}

namespace meteo {
  namespace adiag {

    typedef bool (*FuncInstability)(const zond::Zond& zond, float* result);

    class CreateAeroDiagWidget;

    //! Индексы неустойчивости
    class AeroIndexes : public QDialog {
      Q_OBJECT
    public:
      AeroIndexes(CreateAeroDiagWidget* parent, Qt::WindowFlags fl = 0);
      ~AeroIndexes();

    public slots:
      void fill();

    private:
      void addIndex(const zond::Zond& zond, const QString& name, FuncInstability func, 
		    const QList<QPair<int, QString> >& estim);
      QString aeroIndexesEstimate(float val, const QList<QPair<int, QString> >& estim);

    private:
      CreateAeroDiagWidget* parent_;
      Ui::AeroIndexes* ui_;

    };

  }
}

#endif
