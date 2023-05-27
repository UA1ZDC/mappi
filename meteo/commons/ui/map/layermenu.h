#ifndef METEO_COMMONS_UI_MAP_LAYERMENU_H
#define METEO_COMMONS_UI_MAP_LAYERMENU_H

#include <qobject.h>
#include <qlist.h>

#include <meteo/commons/proto/weather.pb.h>

class QAction;
class QMenu;
class QWidgetAction;
class QPixmap;
class QSpinBox;
class QComboBox;

class AlphaEdit;

namespace meteo {
namespace map {


class Layer;
class DoubleSpinAction;

class LayerMenu : public QObject
{
  Q_OBJECT
  public:
    LayerMenu( Layer* l, QObject* parent = 0 );
    virtual ~LayerMenu();

    virtual void addActions( QMenu* menu );

    void setLayersForRemove( const QList<Layer*>& list );

  protected:
    Layer* layer_;

  protected:
    int getNextLevel( int level ) const ;
    int getPrevLevel( int level ) const ;
    
    Layer* findLayer( const proto::WeatherLayer& info ) const ;
    void showLayer( const proto::WeatherLayer& info ) const ;
    Layer* createLayer( const proto::WeatherLayer& info ) const ;

  private:
    AlphaEdit* alphaedit_;

    QAction* actionactive_;
    QAction* actionrm_;
    QAction* actionup_;
    QAction* actiondown_;
    QAction* actionshow_;
    QAction* actionshowall_;
    QAction* actionhide_;
    QAction* actionhideall_;
    QAction* actionopacity_;
    QAction* actionrename_;

    QMenu* alphamenu_;

    QList<Layer*> foremove_;

  private slots:
    void slotSetActive();
    void slotRmLayer();
    void slotUp();
    void slotDown();
    void slotShowLayer();
    void slotShowAll();
    void slotHideLayer();
    void slotHideAll();
    void slotRename();

    void slotSetAlphaFixed();
    void slotSetAlphaCustom();

  protected slots:
    void slotLevelUp();
    void slotLevelDown();
    void slotSwitchHour();
};

class IsoMenu : public LayerMenu
{
  Q_OBJECT
  public:
    IsoMenu( Layer* l, QObject* parent = 0 );
    ~IsoMenu();

    void addActions( QMenu* menu );

    static QPixmap createPixmap(Qt::PenStyle style, int width, bool isSelected, QString text = QString());
    static void changeStylePixmap(QMenu* act, int val);
    static void changeWidthPixmap(QMenu* act, int oldval);

  private:
    DoubleSpinAction* stepedit_;
    DoubleSpinAction* minedit_;
    DoubleSpinAction* maxedit_;

    QAction* actionmin_;
    QAction* actionmax_;
    QAction* actionminval_;
    QAction* actionmaxval_;
    QAction* actionwidth_;
    QAction* actionstyle_;
    QAction* actionstep_;
    QAction* actionfill_;

    QAction* actionlevelup_;
    QAction* actionleveldown_;
    QAction* actionhour_;

    QMenu* menuwidth_;
    QMenu* menustyle_;
    QMenu* menustep_;
    QMenu* menuminval_;
    QMenu* menumaxval_;
    QMenu* menuhour_;

  private slots:
    void slotColorMinChanged();
    void slotColorMaxChanged();
    void slotWidthChanged();
    void slotStyleChanged();
    void slotColorFillDialog();
    void slotStepChanged();
    void slotStepCustomChanged();
    void slotMinValueChanged();
    void slotMaxValueChanged();
};

class PunchMenu : public LayerMenu
{
  Q_OBJECT
  public:
    PunchMenu( Layer* l, QObject* parent = 0 );
    ~PunchMenu();

    void addActions( QMenu* menu );

  private:
    QAction* actionlevelup_;
    QAction* actionleveldown_;
    QAction* actioncolor_;
    QAction* actionsize_;
    QAction* actionhour_;

    QMenu* menucolor_;
    QMenu* menusize_;
    QMenu* menuhour_;

  private slots:
    void slotChoosePunchColor();
    void slotClearPunchColor();
    void slotChoosePunchScale();
};

class AirportMenu : public PunchMenu
{
  Q_OBJECT
public:
  AirportMenu( Layer* l, QObject* parent = nullptr );
  ~AirportMenu();

  void addActions( QMenu* menu );

private:
  QAction* changeLabel_ = nullptr;
  QMenu* menu_ = nullptr;

private slots:
  void slotChangeLabel();
};

class BorderMenu : public LayerMenu
{
  Q_OBJECT
public:
  BorderMenu( Layer* l, QObject* parent = nullptr );
  ~BorderMenu();

  void addActions( QMenu* menu );

private slots:
  void slotChangeColor();
  void slotChangeWidth(int value);
  void slotChangeStyle();

private:
  void fillLineCombo();
  Qt::PenStyle lineStyle() const;
  void loadProperty();

private:
  QAction* changeColor_ = nullptr;
  QAction* changeWidth_ = nullptr;
  QAction* changeStyle_ = nullptr;
  QSpinBox* width_ = nullptr;
  QComboBox* style_ = nullptr;
  QMenu* menu_ = nullptr;

};

}
}

#endif
