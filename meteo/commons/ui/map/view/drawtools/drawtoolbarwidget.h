#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWTOOLBARWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWTOOLBARWIDGET_H

#include <qmap.h>
#include <qdialog.h>

namespace Ui {
class DrawToolbarWidget;
}

class QToolButton;
class QSettings;
class QLabel;
class ActionButton;

namespace meteo {
namespace map {

const QString kDrawToolsWidgetName = "drawtools";

class PropWidget;

//!
class DrawToolbarWidget : public QWidget
{
  Q_OBJECT
public:
  enum Tool {
    kCursor,
    kLine,
    kPolygon,
    kFront,
    kText,
    kGroup,
    kPixmap,
    kNoTool
  };

public:
  explicit DrawToolbarWidget(QWidget* parent = nullptr);
  virtual ~DrawToolbarWidget();

  Tool currentTool() const;
  void setCurrentTool(Tool tool);
  void setPixmapIcon( QString& str );

public slots:
  void slotSetupGui();
  void slotSwitchTool();
  void slotShowTool();
  void slotHideTool();

signals:
  void toolChanged(DrawToolbarWidget::Tool currentTool);
  void undo();
  void redo();
  void closed();
  void toolShow( DrawToolbarWidget::Tool tool );
  void toolHide( DrawToolbarWidget::Tool tool );
  void textHovered();
  void textLeave();

protected:
  virtual void resizeEvent(QResizeEvent* e);
  virtual void moveEvent(QMoveEvent* e);
  virtual void closeEvent(QCloseEvent* e);

private:
  void onShowSignals();
  void offShowSignals();

private:
  Ui::DrawToolbarWidget* ui_;
  Tool currentTool_;

  QSettings* opt_;
  QMap<Tool,ActionButton*> toolButtons_;
  ActionButton* lineBtn_ = nullptr;
  ActionButton* cursorBtn_ = nullptr;
  ActionButton* frontBtn_ = nullptr;
  ActionButton* textBtn_ = nullptr;
  ActionButton* polygonBtn_ = nullptr;
  ActionButton* pixmapBtn_ = nullptr;
  ActionButton* groupBtn_ = nullptr;
  ActionButton* undoBtn_ = nullptr;
  ActionButton* redoBtn_ = nullptr;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWTOOLBARWIDGET_H
