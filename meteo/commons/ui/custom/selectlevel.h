#ifndef METEO_COMMONS_UI_CUSTOM_SELECTLEVEL_H
#define METEO_COMMONS_UI_CUSTOM_SELECTLEVEL_H

#include <qpair.h>
#include <qmap.h>
#include <qwidget.h>

namespace Ui {
  class SelectLevel;
}

class QAbstractButton;

class SelectLevel : public QWidget
{
  Q_OBJECT

  Q_PROPERTY( int level READ level WRITE setLevel )
  Q_PROPERTY( int typeLevel READ typeLevel WRITE setTypeLevel )
  Q_PROPERTY( ViewMode viewMode READ viewMode WRITE setViewMode )

  Q_ENUMS(ViewMode)
  public:
  enum ViewMode {
    kAero             = 0,
    kOcean            = 1,
    kCustom           = 2,
    kNoMode           = 3
  };

  public:
    SelectLevel( QWidget* parent = 0 );
    ~SelectLevel();

    ViewMode viewMode() const { return mode_; }

    int level() const { return level_; }
    int typeLevel() const { return type_level_; }

    void setViewMode( ViewMode mode );

    void setLevel( int lvl );
    void setTypeLevel( int tplvl );

    void setLevel( int level, int type_level, ViewMode mode );

  private:
    Ui::SelectLevel* ui_;
    ViewMode mode_;
    int level_;
    int type_level_;
    QMap< QAbstractButton*, QPair< int, int > > aerobuttons_;
    QMap< QAbstractButton*, QPair< int, int > > oceanbuttons_;
    QMap< QAbstractButton*, QPair< int, int > > custombuttons_;

  private slots:
    void slotAeroClicked( QAbstractButton* btn );
    void slotOceanClicked( QAbstractButton* btn );

  signals:
    void levelChanged( int type_level, int level );
};

#endif
