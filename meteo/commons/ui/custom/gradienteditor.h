#ifndef METEO_COMMONS_UI_CUSTOM_GRADIENTEDITOR_H
#define METEO_COMMONS_UI_CUSTOM_GRADIENTEDITOR_H

#include <meteo/commons/ui/custom/customtreewidgetitem.h>

#include <qlayout.h>
#include <qdialog.h>
#include <qstandarditemmodel.h>

class QTreeWidgetItem;

namespace Ui {
class GradientEditor;
}

class GradTreeWidget : public QTreeWidget
{
  Q_OBJECT

public:
  GradTreeWidget(QWidget* parent = 0);

protected:
  void mouseDoubleClickEvent(QMouseEvent *event);

signals:
  void iconDoubleClicked(QTreeWidgetItem* item, int col);
};

class GradientWidget : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(float step READ step WRITE setStep)
  Q_PROPERTY(float min READ min WRITE setMin)
  Q_PROPERTY(float max READ max WRITE setMax)
  Q_PROPERTY(QColor minColor READ minColor WRITE setMinColor NOTIFY minColorChanged)
  Q_PROPERTY(QColor maxColor READ maxColor WRITE setMaxColor NOTIFY maxColorChanged)

public:
  GradientWidget(float min, const QColor& minColor, float max, const QColor& maxColor, float step, QWidget* parent = 0);
  GradientWidget(const TColorGradList& gradient, QWidget* parent = 0);
  GradientWidget(QWidget* parent = 0);
  virtual ~GradientWidget();

  float step() const;
  void setStep(float s);

  float min() const;
  void setMin(float n);

  float max() const;
  void setMax(float x);

  QColor minColor() const { return minColor_; }
  void setMinColor(const QColor& color) { minColor_ = color; }

  QColor maxColor() const { return maxColor_; }
  void setMaxColor(const QColor& color) { maxColor_ = color; }

  TColorGradList gradient() const { return current_; }
  void setGradient(const TColorGradList& grad) { current_ = grad; }

public slots:
  void slotSetDefault();
  bool slotApply();
  void buildGradient();
  void paletteDoubleClicked(QTreeWidgetItem* item, int column);
  void changeColor(QTreeWidgetItem* item, int col);
  void addColor();
  void rmColor();
  void rmAllColor();
  void changeMinColor();
  void changeMaxColor();

signals:
  void apply();
  void reject();
  void minColorChanged(const QColor& color);
  void maxColorChanged(const QColor& color);

private:
  void init();
  void buildGradient( const TColorGradList& grad );

private:
  Ui::GradientEditor* ui_;
  GradTreeWidget* palette_;

  TColorGradList current_;
  QColor minColor_;
  QColor maxColor_;
};


namespace meteo {
namespace map {

class GradientEditor : public QDialog
{
  Q_OBJECT

public:
  GradientEditor(float min, const QColor& minColor, float max, const QColor& maxColor, float step, QWidget* parent = 0)
    : QDialog(parent)
  {
    ui_ = new GradientWidget(min,minColor,max,maxColor,step,parent);
    connect(ui_, &GradientWidget::apply, this, &GradientEditor::slotApply);
    connect(ui_, &GradientWidget::reject, this, &GradientEditor::reject);

    QLayout* l = new QGridLayout(this);
    l->addWidget(ui_);
  }

  GradientEditor(const TColorGradList& gradient, QWidget* parent = 0)
    : QDialog(parent)
  {
    ui_ = new GradientWidget(gradient,parent);
    connect(ui_, &GradientWidget::apply, this, &GradientEditor::slotApply);
    connect(ui_, &GradientWidget::reject, this, &GradientEditor::reject);

    QLayout* l = new QGridLayout(this);
    l->addWidget(ui_);
  }

  virtual ~GradientEditor() {}

  float min() const { return ui_->min(); }
  float max() const { return ui_->max(); }
  float step() const { return ui_->step(); }

  TColorGradList gradient() const { return ui_->gradient(); }

public slots:
  void slotApply()  { ui_->slotApply(); accept(); }
  void setDefault() { ui_->slotSetDefault(); }
  void buildGradient() { ui_->buildGradient(); }
  void paletteDoubleClicked(QTreeWidgetItem* item, int column)  { ui_->paletteDoubleClicked(item,column); }
  void changeColor(QTreeWidgetItem* item, int col)              { ui_->changeColor(item,col); }
  void addColor()   { ui_->addColor(); }
  void rmColor()    { ui_->rmColor(); }
  void rmAllColor() { ui_->rmAllColor(); }
  void changeMinColor() { ui_->changeMinColor(); }
  void changeMaxColor() { ui_->changeMaxColor(); }

private:
  GradientWidget* ui_;
};

} // map
} // meteo


#endif
