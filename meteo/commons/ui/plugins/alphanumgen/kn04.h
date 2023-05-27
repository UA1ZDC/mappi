#ifndef METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_KN04_H
#define METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_KN04_H

#include <qdialog.h>

class QDateTime;
class QRegExp;
class QLineEdit;
class QTableWidgetItem;

namespace Ui {
  class Kn04;
}

namespace meteo {
  class AlphanumGen;
}

namespace meteo {

  class Kn04: public QWidget {
    Q_OBJECT
    public:
    Kn04(QWidget* parent = nullptr, Qt::WindowFlags fl = 0);
    ~Kn04();

  private:
    void keyPressEvent( QKeyEvent * e);

    void initWidgets();
    void setConnects();
    void setDefault();
    void fillPartDefault(int part);
    bool saveInnerDoc(const QString& name);
    bool checkParams(const QLineEdit& le, const QString& error);
    bool checkCorrectInput();
    int setId();

  private slots:
    void slotSaveTlg();
    void slotSendTlg();
    void slotHelp(bool);

    void slotSetValue(int tval);
    void slotSetValue(double tval);
    void slotSetValue(const QString& tval);
    void slotGenerate();

    void slotDateTimeChanged(const QDateTime& dt);
    void slotCloudsChanged();
    void slotPartChanged(int);
    void slotShearChanged(bool);
    void slotGGggChanged(const QTime&);
    void slotLevelsChanged();
    
    void slotAddLevelLine(int type = -1, const int* level = nullptr);
    void slotRmLevelLine();

    void slotSetIntCell(int row, int col, int min, int max, const int* value = nullptr);
    void slotSetDoubleCell(int row, int col, double min, double max, int decimals);
    void slotSetRegexpCell(int row, int col, const QRegExp& rx);

    void slotSetLevelValue(const QString& name, int row, int col);
    void slotSetWind(const QString& ddname, const QString& ffname, int row);

  private:
    Ui::Kn04* ui_ = nullptr;
    AlphanumGen* gen_ = nullptr;
    QDialog* helpUi_ = nullptr;
  };

}

#endif
