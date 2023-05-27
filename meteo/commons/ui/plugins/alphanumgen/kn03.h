#ifndef METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_KN03_H
#define METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_KN03_H

#include <qdialog.h>

class QDateTime;
class QRegExp;
class QLineEdit;

namespace Ui {
  class Kn03;
}

namespace meteo {
  class AlphanumGen;
}

namespace meteo {

  class Kn03: public QWidget {
    Q_OBJECT
    public:
    Kn03(QWidget* parent = nullptr, Qt::WindowFlags fl = 0);
    ~Kn03();

  private:
    void keyPressEvent( QKeyEvent* e);

    void initWidgets();
    void setConnects();
    void setDefault();
    void fillPartDefault(int part);
    bool saveInnerDoc(const QString& name);
    bool checkParams(const QLineEdit& le, const QString& error);
    void setStandartLevel(const QList<int>& mark, const QList<QString>& level, const QList<QString>& dd, const QList<QString>& ff);
    void setSignLevel(const QList<int>& height, const QList<QString>& dd, const QList<QString>& ff, bool signStation);
    bool checkCorrectInput();

  private slots:
    void slotSaveTlg();
    void slotSendTlg();
    void slotHelp(bool);

    void slotSetValue(int tval);
    void slotSetValue(double tval);
    void slotSetValue(const QString& tval);
    void slotGenerate();

    void slotDateTimeChanged(const QDateTime& dt);
    void slotPartChanged(int);
    void slotShearChanged(bool);
    void slotLevelsChanged();
    
    void slotAddLevelLine(int type = -1, const int* level = nullptr, const int* height = nullptr);
    void slotRmLevelLine();

    void slotSetIntCell(int row, int col, int min, int max, bool readOnly = false, const int* value = nullptr);
    void slotSetDoubleCell(int row, int col, double min, double max, int decimals);
    void slotSetRegexpCell(int row, int col, const QRegExp& rx);

    void slotSetLevelValue(const QString& name, int row, int col);
    void slotSetWind(const QString& ddname, const QString& ffname, int row);

  private:
    Ui::Kn03* ui_ = nullptr;
    AlphanumGen* gen_ = nullptr;
    QDialog* helpUi_ = nullptr;
  };

}

#endif
