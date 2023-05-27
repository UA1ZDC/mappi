#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_TEXTMINWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_TEXTMINWIDGET_H

#include "propwidget.h"

namespace Ui {
class TextMinWidget;
}

class QGraphicsProxyWidget;

namespace meteo {
namespace map {

class TextPosDlg;
class SymbDlg;

class TextPropWidget : public PropWidget
{
  Q_OBJECT

public:
  explicit TextPropWidget(QWidget *parent = 0);
  ~TextPropWidget();
  void initWidget();
  virtual meteo::Property toProperty() const;
  virtual void setProperty(const meteo::Property& prop);

public slots:
  void slotOnDelBtn();
  void slotOffDelBtn();

private slots:
  void slotSetProperty( meteo::Property prop);
  void slotShowColorDlg();
  void slotAlignChanged();
  void slotAlignSwitch();
  void slotSwitchSymb();
  void slotSymbolInsert();
  void slotRamkaChanged();
  void slotPriorChanged();
  void slotDrawAlwaysChanged();
  void slotBrushColorChanged();
  void slotChangeFont();
  void slotDeleteCurrent();
  void slotFinish();

private:
  QString imgForAlign( int pos );
  void defaultProperty();
  void setDrawAlwaysIcon();
  void loadSettings();
  void saveSettings();

private:
  Ui::TextMinWidget* ui_ = nullptr;
  QGraphicsProxyWidget* alignDlgPW_ = nullptr;
  QGraphicsProxyWidget* symbProxy_ = nullptr;
  TextPosDlg* alignDlg_ = nullptr;
  SymbDlg* symbDlg_ = nullptr;
  meteo::Property prop_;
};

}
}


#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_TEXTMINWIDGET_H
