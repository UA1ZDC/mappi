#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PANALWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PANALWIDGET_H

#include <qwidget.h>

class QLabel;
class QGridLayout;
class QPushButton;

namespace meteo {
namespace map {


class PanelHeader;


//!
class PanelWidget : public QWidget
{
  Q_OBJECT
public:
  enum Option  {
    kCollapseButton
  };

  explicit PanelWidget(QWidget* parent = 0);
  virtual ~PanelWidget(){}
  //!
  void setMainWidget(QWidget* widget);
  //!
  void setOption(Option opt, bool enable = true);
  //!
  bool isCollapsed() const;

public slots:
  void slotToggleCollapse();
  void slotSetCollapse(bool collapse);

protected:
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);
  virtual void mouseMoveEvent(QMouseEvent* e);

private:
  QPoint pressOffset_;
  bool movePanel_;

  PanelHeader* header_;
  QWidget* body_;
};


//!
class PanelHeader : public QWidget
{
  friend class PanelWidget;

  Q_OBJECT
public:
  explicit PanelHeader(QWidget* parent = 0);

  //!
  void setOption(PanelWidget::Option opt, bool enable = true);
  //!
  bool isCollapsed() const { return collapse_; }

signals:
  void collapsed();
  void expanded();

public slots:
  //!
  void slotSetCollapse(bool collapse);

private slots:
  void slotCollapseBtnClicked();

private:
  QGridLayout* layout_;

  QLabel* label_;
  QPushButton* collapseBtn_;

  bool collapse_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PANALWIDGET_H
