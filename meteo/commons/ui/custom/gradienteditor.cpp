#include "gradienteditor.h"
#include "ui_gradienteditor.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/custom/colorwidget.h>
#include <commons/funcs/tcolorgrad.h>

#include <qcolor.h>
#include <qevent.h>
#include <QtWidgets>

enum {
  kColorValue = 0, 
  kColor = 1,
  kMin = 2,
  kMax = 3,
  kTitle = 4
};

GradientWidget::GradientWidget(float min, const QColor& mnclr, float max, const QColor& mxclr, float step, QWidget* p ):
  QWidget(p)
{
  QColor minColor = mnclr;
  QColor maxColor = mxclr;
  // minColor.setAlpha(200);
  // maxColor.setAlpha(200);

  TColorGrad grad(min, max, minColor, maxColor);

  float val = min;
  if (step != 0) {
    while (val + step < max) {
      current_.append( TColorGrad( val, val+step, grad.color(val), grad.color(val) ) );
      val += step;
    }
  }
  current_.append( TColorGrad( val, max, grad.color(val), grad.color(val) ) );

  init();
}

GradientWidget::GradientWidget(const TColorGradList& gradient, QWidget* parent /*= 0*/):
  QWidget(parent),
  current_(gradient)
{
  init();
}

GradientWidget::GradientWidget(QWidget* parent)
  : QWidget(parent)
{
  init();
}

GradientWidget::~GradientWidget()
{
}

void GradientWidget::init()
{
  ui_ = new Ui::GradientEditor;
  ui_->setupUi(this);

  palette_ = new GradTreeWidget(this);
  QGridLayout* gridLayout = new QGridLayout(ui_->palette);
  gridLayout->addWidget(palette_, 0, 0, 1, 1);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  QStringList labels;
  labels << QObject::tr("Цвет") << QObject::tr("Цвет") << QObject::tr("Мин") << QObject::tr("Макс")<< QObject::tr("Текст");
  palette_->setHeaderLabels(labels);
  palette_->setProperty("showDropIndicator", QVariant(false));
  palette_->setSelectionMode(QAbstractItemView::SingleSelection);
  palette_->setRootIsDecorated(false);
  palette_->setItemsExpandable(false);
  palette_->setExpandsOnDoubleClick(false);
  palette_->header()->setStretchLastSection(true);
  QTreeWidgetItem *header = palette_->headerItem();
  header->setTextAlignment(3, Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
  header->setTextAlignment(2, Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
  header->setTextAlignment(1, Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
  palette_->hideColumn(kColorValue);
  
  slotSetDefault();

  QObject::connect(ui_->cancelButton, SIGNAL(clicked()), SIGNAL(reject()));
  QObject::connect(ui_->applyButton,  SIGNAL(clicked()), SIGNAL(apply()));
  QObject::connect(ui_->minColor, SIGNAL(clicked()), SLOT(changeMinColor()));
  QObject::connect(ui_->maxColor, SIGNAL(clicked()), SLOT(changeMaxColor()));
  QObject::connect(ui_->min,  SIGNAL(valueChanged(double)), SLOT(buildGradient()));
  QObject::connect(ui_->max,  SIGNAL(valueChanged(double)), SLOT(buildGradient()));
  QObject::connect(ui_->step, SIGNAL(valueChanged(double)), SLOT(buildGradient()));
  QObject::connect(palette_, SIGNAL(iconDoubleClicked(QTreeWidgetItem*, int)), SLOT(paletteDoubleClicked(QTreeWidgetItem*, int)));
  QObject::connect(ui_->autoFill, SIGNAL(clicked()), SLOT(buildGradient()));
  QObject::connect(ui_->addBtn, SIGNAL(clicked()), SLOT(addColor()));
  QObject::connect(ui_->removeBtn, SIGNAL(clicked()), SLOT(rmColor()));
  QObject::connect(ui_->removeAllBtn, SIGNAL(clicked()), SLOT(rmAllColor()));
}

void GradientWidget::buildGradient()
{
  TColorGradList gradlist;
  QColor minColor = minColor_;
  QColor maxColor = maxColor_;

  TColorGrad grad(ui_->min->value(), ui_->max->value(), minColor, maxColor);

  float val = ui_->min->value();
  int idx = 0;
  if (ui_->step->value() != 0) {
    while (val + ui_->step->value() < ui_->max->value()) {
      gradlist.append( TColorGrad(val, val + ui_->step->value(), grad.color(val), grad.color(val) ) );
      val += ui_->step->value();
      idx++;
    }
  }
  gradlist.append( TColorGrad(val, ui_->max->value(), grad.color(val), grad.color(val) ) );
  buildGradient(gradlist);
}

void GradientWidget::buildGradient(const TColorGradList& gradlist )
{
  palette_->clear();

  for (int idx = 0; idx < gradlist.size(); idx++) {
    QTreeWidgetItem* item = new QTreeWidgetItem(palette_);    
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    QPixmap pix(20,20);
    pix.fill( gradlist.at(idx).begcolor() );
    item->setText(kColorValue, QString::number( gradlist.at(idx).begcolor().rgba() ) );
    item->setIcon(kColor, QIcon(pix));
    item->setText(kMin, QString::number(gradlist.at(idx).begval() ) );
    item->setText(kMax, QString::number(gradlist.at(idx).endval() ) );
    //item->setText(kTitle,gradlist.at(idx).title() );
  }
}

//! установка параметров по умолчанию [public slot]
void GradientWidget::slotSetDefault()
{
  if ( current_.isEmpty() ) {
    palette_->clear();
    minColor_ = Qt::white;
    maxColor_ = Qt::black;
    ui_->minColor->setStyleSheet(QString("background: white"));
    ui_->maxColor->setStyleSheet(QString("background: black"));
    ui_->min->setValue(0);
    ui_->max->setValue(0);
    ui_->step->setValue(0);
    return;
  }

  minColor_ = current_.first().begcolor();
  maxColor_ = current_.last().endcolor();
  ui_->minColor->setStyleSheet(QString("background: %1").arg(minColor_.name()));
  ui_->maxColor->setStyleSheet(QString("background: %1").arg(maxColor_.name()));
  
  ui_->min->setValue(current_.first().begval());
  ui_->max->setValue(current_.last().endval());
  ui_->step->setValue(current_.first().endval() - current_.first().begval() );

  buildGradient(current_);
}

bool GradientWidget::slotApply()
{
  TColorGradList cur;
  for (int i = 0; i < palette_->topLevelItemCount(); ++i) {
    QTreeWidgetItem* item = palette_->topLevelItem(i);
    QColor color;
    color.setRgba(item->text(kColorValue).toUInt());
    cur.append( TColorGrad(item->text(kMin).toFloat(), item->text(kMax).toFloat(),
                           color, color) );
    if(false == item->text(kTitle).isEmpty()){
//	cur.last().setTitle(item->text(kTitle));
    }


    if (cur.at(i).begval() > cur.at(i).endval()) {
      QMessageBox::information(this, QObject::tr("Не верный диапазон"),
                               QObject::tr("В строке %1 минимальное значение %2 превышает максимальное %3").
                               arg(i+1).arg(cur.at(i).begval()).arg(cur.at(i).endval()));
      return false;
    }


    if (i != 0) {
      if (cur.at(i).begval() < cur.at(i-1).endval()) {
        QMessageBox::information(this, QObject::tr("Не верный диапазон"),
                                 QObject::tr("В строках %1 и %2 диапазоны значений перекрываются").
                                 arg(i).arg(i+1));
        return false;
      }
    }
  }

  current_.clear();
  current_ = cur;

  return true;
}

//! изменение цвета пользователем [public slot]
void GradientWidget::changeColor(QTreeWidgetItem* item, int col)
{
  if (col != kColor) {
    return;
  }
  
  IsoColorDialog* dlg = new IsoColorDialog(0, IsoColorDialog::NoOption, this);
  QColor c;
  c.setRgba(item->text(kColorValue).toUInt());
  dlg->setCurrentColor(c);
  dlg->show();
  if (dlg->exec() == QDialog::Accepted) {
    item->setText(kColorValue, QString::number(dlg->currentColor().rgba()));
    QPixmap pix(20,20);
    pix.fill(dlg->currentColor());
    item->setIcon(kColor, QIcon(pix));
  }
  delete dlg;
}

float GradientWidget::step() const
{
  return ui_->step->value();
}

void GradientWidget::setStep(float s)
{
  ui_->step->setValue(s);
}

float GradientWidget::min() const
{
  return ui_->min->value();
}

void GradientWidget::setMin(float n)
{
  ui_->min->setValue(n);
}

float GradientWidget::max() const
{
  return ui_->max->value();
}

void GradientWidget::setMax(float x)
{
  ui_->max->setValue(x);
}

//! добавление цвета
void GradientWidget::addColor()
{
  int index = 0;
  QTreeWidgetItem* prev = palette_->currentItem();
  if (0 != prev) {
    index = palette_->indexOfTopLevelItem(prev) + 1;
  }

  QTreeWidgetItem* item = new QTreeWidgetItem();   
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  palette_->insertTopLevelItem(index, item);
  QPixmap pix(20,20);
  QColor color = QColor(0,0,0);
  if ( !current_.isEmpty() ) {
    color = current_.first().begcolor();
  }
  float min = 0;
  float max = 0;
  if (0 != prev) {
    min = max = prev->text(kMax).toFloat();
  }
  
  pix.fill(color);
  item->setText(kColorValue, QString::number(color.rgba()));
  item->setIcon(kColor, QIcon(pix));
  item->setText(kMin, QString::number(min));
  item->setText(kMax, QString::number(max));
}

//! удаление цвета
void GradientWidget::rmColor()
{
  QTreeWidgetItem* item = palette_->currentItem();
  if (0 == item) {
    return;
  }
  
  palette_->takeTopLevelItem(palette_->indexOfTopLevelItem(item));
}

//! удаление всех цветов
void GradientWidget::rmAllColor()
{
  palette_->clear();
}

//! изменение строки палитры
void GradientWidget::paletteDoubleClicked(QTreeWidgetItem* item, int column)
{
  if (0 == item) return;

  switch (column) {
  case kColor:
    changeColor(item, column);
    
    break;
  }
}

void GradientWidget::changeMinColor()
{
  IsoColorDialog* dlg = new IsoColorDialog(0, IsoColorDialog::NoOption, this);
  //  dlg->show();
  dlg->setCurrentColor(minColor_);
  if( dlg->exec() == QDialog::Accepted  ){
    ui_->minColor->setStyleSheet(QString("background: %1").arg(dlg->currentColor().name()));
    minColor_ = dlg->currentColor();
  }
  delete dlg;

  buildGradient();
}

void GradientWidget::changeMaxColor()
{
  IsoColorDialog* dlg = new IsoColorDialog(0, IsoColorDialog::NoOption, this);
  //  dlg->show();
  dlg->setCurrentColor(maxColor_);
  if( dlg->exec() == QDialog::Accepted  ){
    ui_->maxColor->setStyleSheet(QString("background: %1").arg(dlg->currentColor().name()));
    maxColor_ = dlg->currentColor();
  }
  delete dlg;

  buildGradient();
}

GradTreeWidget::GradTreeWidget(QWidget* parent) :
  QTreeWidget(parent)
{
  setRootIsDecorated(false);
}

void GradTreeWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
  const QPoint clickedPosition    = ev->pos();
  int col = columnAt(clickedPosition.x());
  QTreeWidgetItem* item = itemAt(clickedPosition);
  if (col == kColor) {
    emit iconDoubleClicked(item, col);
  } else {
    QTreeWidget::mouseDoubleClickEvent( ev );
  }
}
