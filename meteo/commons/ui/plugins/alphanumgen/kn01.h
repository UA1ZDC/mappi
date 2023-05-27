#ifndef METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_KN01_H
#define METEO_COMMONS_UI_PLUGINS_ALPHANUMGEN_KN01_H

#include <qdialog.h>

class QDateTime;
class QLineEdit;
class QTreeWidgetItem;

namespace Ui {
  class Kn01;
  class SelectWeather;
}

namespace meteo {
  class AlphanumGen;
}

namespace meteo {

  class Kn01 : public QWidget {
    Q_OBJECT
    public:
    Kn01(QWidget* parent = nullptr, Qt::WindowFlags fl = 0);
    ~Kn01();

  private:
    void initWidgets();
    void setConnects();
    void setDefault();
    bool saveInnerDoc(const QString& name);
    bool checkParams(const QLineEdit& le, const QString& error);
    bool checkCorrectInput();

    void keyPressEvent( QKeyEvent * e) override;

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
    void slotVVChanged();
    void slotSSSChanged();
    void slotPrecpChanged();
    void slotWeatherChanged();
    void slotCloudsLayChanged();
    void slotIsTxChanged(int state);
    void slotIsTnChanged(int state);
    void slotIsTgChanged(int state);
    void slotSssChanged();
    void slotWindChanged();
    void slotIsPPPPChanged(bool checked);

    void slotSetddSuffix(int value);
    void slotSetww();

    void slotAddChanged();
    void slotAddMountainClouds();
    void slotNationalChanged();

  private:
    Ui::Kn01* ui_ = nullptr;
    AlphanumGen* gen_ = nullptr;
    QDialog* helpUi_ = nullptr;
  };


  class SelectWeather : public QDialog {
    Q_OBJECT
    public:
    SelectWeather();
    ~SelectWeather();
		    
    int weather() { return w_; }

  private slots:
    void slotApply();
    void slotSelected();
    void slotApplyItem(QTreeWidgetItem* item);

  private:
    void fillww();
            
  private:
    Ui::SelectWeather* ui_ = nullptr;
    int w_;
  };
}

#endif
