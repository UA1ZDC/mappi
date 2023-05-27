#include "isosettingswidget.h"
#include <meteo/commons/ui/map/view/widgets/layerswidget.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/ui/custom/doublespinaction.h>

#include <meteo/commons/ui/map/layermenu.h>

namespace meteo{
namespace map{

static const QList< float > kStepFixed = QList< float >()
     <<  0.25 << 0.5 << 1 << 2 << 3  << 4 << 5 << 8 << 10 << 16;

IsoSettingsWidget::IsoSettingsWidget( meteo::app::MainWindow* parent) :
  QWidget(parent),
  mainwindow_(parent),
  width_(2),
  style_(Qt::SolidLine),
  colorMin_(Qt::black),
  colorMax_(Qt::black),
  stepEdit_(0),
  minEdit_(0),
  maxEdit_(0)
{
  QHBoxLayout* hbl = new QHBoxLayout(this);
  hbl->setMargin(0);
  hbl->setSpacing(2);
  label_ = new QLabel;
  btn_ = new QToolButton;
  btn_->setMinimumSize(QSize(26,26));
  btn_->setMaximumSize(QSize(26,26));
  btn_->setIcon(QIcon(":/meteo/icons/arrow/darkarrowdown.png"));
  btn_->setIconSize(QSize(12, 12));
  btn_->setToolTip(QObject::tr("Настроить"));
  hbl->addWidget(label_);
  hbl->addWidget(btn_);
  label_->setStyleSheet("background-color: white; border: 1px solid rgb(194, 194, 194); border-radius: 3px;");
  label_->setMaximumWidth(200);

  menu_ = new QMenu(this);

  {
    QMenu* menu = new QMenu( QObject::tr("Толщина линии"), this );
    widthmenu_ = menu;
    for ( int i = 1; i < 7; ++i ) {
      QLabel* l = new QLabel;
      l->setFrameShape(QFrame::StyledPanel);
      l->setFrameShadow(QFrame::Sunken);

      QPixmap pix = IsoMenu::createPixmap(Qt::SolidLine, i, false, QString::number(i));
      QWidgetAction* wa = new QWidgetAction(menu);
      l->setPixmap(pix);
      wa->setDefaultWidget(l);
      menu->addAction(wa);
      QObject::connect( wa, SIGNAL( triggered() ), this, SLOT( slotWidthChanged() ) );
      wa->setData(i);
      if (i == 1) {
        wa->trigger();
      }
    }
    menu_->addAction( menu->menuAction() );
  }

  {
    QMenu* menu = new QMenu( QObject::tr("Стиль линии"),this );
    stylemenu_ = menu;
    for ( int i = Qt::SolidLine; i < Qt::CustomDashLine; ++i ) {
      QLabel* l = new QLabel;
      //l->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
      l->setFrameShape(QFrame::StyledPanel);
      l->setFrameShadow(QFrame::Sunken);

      QPixmap pixmap = IsoMenu::createPixmap(Qt::PenStyle(i), 2, false);
      QWidgetAction* wa = new QWidgetAction(menu);
      l->setPixmap(pixmap);
      wa->setDefaultWidget(l);
      menu->addAction(wa);
      QObject::connect( wa, SIGNAL( triggered() ), this, SLOT( slotStyleChanged() ) );
      wa->setData(i);
      if (i == Qt::SolidLine) {
        wa->trigger();
      }
    }
    menu_->addAction( menu->menuAction() );
  }

  {
    QMenu* menu = new QMenu( QObject::tr("Шаг"), this );
    for ( int i = 0; i < kStepFixed.size(); ++i ) {
      QAction* a = menu->addAction(QString::number(kStepFixed.at(i)));
      QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotStepChanged() ) );
    }
    QWidgetAction* stepcust = new QWidgetAction(menu);
    QObject::connect( stepcust, SIGNAL( triggered() ), SLOT( slotStepChanged() ) );
    stepEdit_ = new DoubleSpinAction( menu, stepcust );
    stepEdit_->setMinimum(-90);
    stepEdit_->setMaximum(5000);
    stepEdit_->setDecimals(2);
    stepcust->setDefaultWidget(stepEdit_);
    menu->addAction(stepcust);
    menu_->addAction( menu->menuAction() );
  }

  {
    QMenu* menu = new QMenu( QObject::tr("Мин"), this );
    QWidgetAction* minval = new QWidgetAction(menu);
    QObject::connect( minval, SIGNAL( triggered() ), SLOT( slotMinChanged() ) );
    minEdit_ = new DoubleSpinAction( menu, minval );
    minEdit_->setMinimum(-90);
    minEdit_->setMaximum(5000);
    minEdit_->setSingleStep(5);
    minEdit_->setDecimals(2);
    minval->setDefaultWidget(minEdit_);
    menu->addAction(minval);
    menu_->addAction( menu->menuAction() );
  }

  {
    QMenu* menu = new QMenu( QObject::tr("Макс"),this );
    QWidgetAction* maxval = new QWidgetAction(menu);
    QObject::connect( maxval, SIGNAL( triggered() ), SLOT( slotMaxChanged() ) );
    maxEdit_ = new DoubleSpinAction( menu, maxval );
    maxEdit_->setMinimum(-90);
    maxEdit_->setMaximum(5000);
    maxEdit_->setSingleStep(5);
    maxEdit_->setDecimals(2);
    maxval->setDefaultWidget(maxEdit_);
    menu->addAction(maxval);
    menu_->addAction( menu->menuAction() );
  }

  QPixmap pix(20,20);
  pix.fill(Qt::black);
  actionMin_ = menu_->addAction( QIcon(pix), QObject::tr("Мин. цвет") );
  QObject::connect( actionMin_, SIGNAL( triggered() ), this, SLOT( slotColorMinChanged() ) );
  actionMax_ = menu_->addAction( QIcon(pix), QObject::tr("Макс. цвет") );
  QObject::connect( actionMax_, SIGNAL( triggered() ), this, SLOT( slotColorMaxChanged() ) );

  setDefault();
  QObject::connect( btn_, SIGNAL(clicked()), SLOT(slotExecMenu()) );
}

// QPixmap IsoSettingsWidget::createPixmap(Qt::PenStyle style, int width, bool isSelected, QString text /* = QString()*/)
// {
//   QFontMetrics fm(font());
//   int textWidth = fm.width("6");
//   if (text.isNull()) textWidth = 0;

//   QPixmap pixmap(80 + textWidth, 20);
//   QColor color = Qt::white;
//   if (isSelected) {
//     //    color = QColor(175, 215, 240);
//     color = QColor(200, 233, 252);
//   }
//   pixmap.fill(color);

//   QBrush brush;
//   brush.setStyle(Qt::SolidPattern);
//   QPen pen(brush, width, style);
//   QPainter pntr(&pixmap);
//   pntr.setBrush(brush);
//   pntr.setPen(pen);
//   pntr.drawLine(10, 10, 70, 10);
//   if (!text.isNull()) {
//     pntr.drawText(QRect(75, 5, textWidth, 10), Qt::AlignCenter, text);
//   }

//   return pixmap;
// }

// void IsoSettingsWidget::changeStylePixmap(QWidgetAction* act)
// {
//   if( 0 == act || 0 == act->parent()) {
//     return;
//   }

//   QList<QAction*> allact = qobject_cast<QWidget*>(act->parent())->actions();
//   for (int idx = 0; idx < allact.size(); idx++) {
//     QWidgetAction* wa = qobject_cast<QWidgetAction*>(allact.at(idx));
//     if( 0 != wa ){
//       QLabel* w = qobject_cast<QLabel*>(wa->defaultWidget());
//       if (0 != w) {
// 	if (wa == act) {
// 	  w->setPixmap(createPixmap(Qt::PenStyle(wa->data().toInt()), 2, true));
// 	} else if (wa->data().toInt() == style_) {
// 	  w->setPixmap(createPixmap(Qt::PenStyle(wa->data().toInt()), 2, false));
// 	}
//       }
//     }
//   }
// }

// void IsoSettingsWidget::changeWidthPixmap(QWidgetAction* act)
// {
//   if( 0 == act || 0 == act->parent()) {
//     return;
//   }

//   QList<QAction*> allact = qobject_cast<QWidget*>(act->parent())->actions();
//   for (int idx = 0; idx < allact.size(); idx++) {
//     QWidgetAction* wa = qobject_cast<QWidgetAction*>(allact.at(idx));
//     if( 0 != wa ){
//       QLabel* w = qobject_cast<QLabel*>(wa->defaultWidget());
//       if (0 != w) {
// 	if (wa == act) {
// 	  w->setPixmap(createPixmap(Qt::SolidLine, wa->data().toInt(), true, wa->data().toString()));
// 	} else if (wa->data().toInt() == style_) {
// 	  w->setPixmap(createPixmap(Qt::SolidLine, wa->data().toInt(), false, wa->data().toString()));
// 	}
//       }
//     }
//   }
// }

void IsoSettingsWidget::setPixmap(const QPixmap& pix)
{
  label_->setPixmap(pix);
}

void IsoSettingsWidget::setWidth(int width)
{
  width_ = width;
  paint();
  emit widthChange(width_);
}

void IsoSettingsWidget::setStep(float step)
{
  if (0 != stepEdit_) {
    stepEdit_->setValue(step);
  }
  emit stepChange(step);
}

void IsoSettingsWidget::setMin(float min)
{
  if (0 != minEdit_) {
    minEdit_->setValue(min);
  }
  emit minChange(min);
}

void IsoSettingsWidget::setMax(float max)
{
  if (0 != maxEdit_) {
    maxEdit_->setValue(max);
  }
  emit maxChange(max);
}

void IsoSettingsWidget::setColorMin(const QColor& color)
{
  colorMin_ = color;
  QPixmap pix(20,20);
  pix.fill(color);
  if( 0 != actionMin_ ){
    actionMin_->setIcon(QIcon(pix));
  }
   paint();
   emit colorMinChanged(colorMin_);
}

void IsoSettingsWidget::setColorMax(const QColor& color)
{
  colorMax_ = color;
  QPixmap pix(20,20);
  pix.fill(color);
  if( 0 != actionMax_ ){
    actionMax_->setIcon(QIcon(pix));
  }
  paint();
  emit colorMaxChanged(colorMax_);
}


void IsoSettingsWidget::setColorGradient(const QColor& color)
{
  colorMax_ = color;
  QPixmap pix(20,20);
  pix.fill(color);
  if( 0 != actionMax_ ){
    actionMax_->setIcon(QIcon(pix));
  }
  paint();
  emit colorMaxChanged(colorMax_);
}


void IsoSettingsWidget::setStyle(Qt::PenStyle style)
{
  style_ = style;
  paint();
  emit styleChange(style_);
}

void IsoSettingsWidget::setSettings(int level, int typeLevel, const proto::FieldColor& color )
{
  QPen pen = pen2qpen( color.pen() );

  width_ = pen.width();
  style_ = pen.style();
  IsoMenu::changeStylePixmap(stylemenu_, style_);
  IsoMenu::changeWidthPixmap(widthmenu_, width_);

  if (0 != stepEdit_) {
    stepEdit_->setValue(color.step_iso());
  }

  proto::LevelColor levColor = GradientParams::levelProto(level, typeLevel, color);
  colorMin_ = QColor::fromRgba( levColor.color().min_color() );
  colorMax_ = QColor::fromRgba( levColor.color().max_color() );
  if (levColor.has_step_iso() && 0 != stepEdit_) {
    stepEdit_->setValue(levColor.step_iso());
  }
  if (0 != minEdit_ && 0 != maxEdit_) {
    minEdit_->setValue(levColor.color().min_value());
    maxEdit_->setValue(levColor.color().max_value());
  }

  QPixmap pix(20,20);
  pix.fill(colorMax_);
  if( 0 != actionMax_ ){
    actionMax_->setIcon(QIcon(pix));
  }
  pix.fill(colorMin_);
  if( 0 != actionMin_ ){
    actionMin_->setIcon(QIcon(pix));
  }

  paint();
  //emit settingsChange();
}

void IsoSettingsWidget::setDefault()
{
  clearPixmap();
  QPixmap pixmap(20,20);
  pixmap.fill(colorMin_);
  if( 0 != actionMin_ ){
    actionMin_->setIcon(QIcon(pixmap));
  }
  pixmap.fill(colorMax_);
  if( 0 != actionMax_ ){
    actionMax_->setIcon(QIcon(pixmap));
  }
  stepEdit_->setValue(1);
  minEdit_->setValue(0);
  maxEdit_->setValue(0);
}

QColor IsoSettingsWidget::colorMin() const
{
  return colorMin_;
}

QColor IsoSettingsWidget::colorMax() const
{
  return colorMax_;
}

Qt::PenStyle IsoSettingsWidget::lineStyle() const
{
  return style_;
}

int IsoSettingsWidget::lineWidth() const
{
  return width_;
}

float IsoSettingsWidget::step() const
{
  return stepEdit_->value();
}

float IsoSettingsWidget::min() const
{
  return minEdit_->value();
}

float IsoSettingsWidget::max() const
{
  return maxEdit_->value();
}

void IsoSettingsWidget::update( LayerIso* iso )
{
  setSettings(  iso->level(), iso->typeLevel(), iso->fieldColor() );
}

void IsoSettingsWidget::paint()
{
  QPixmap pix(label_->width(),14);
  pix.fill(Qt::white);

  QLinearGradient grad(label_->rect().topLeft(),label_->rect().topRight());
  grad.setColorAt(0, colorMin_);
  grad.setColorAt(1, colorMax_);

  QBrush brush(grad);
  brush.setStyle(Qt::LinearGradientPattern);
  QPainter painter(&pix);
  painter.setBrush(brush);
  QPen pen(brush, width_, style_);
  painter.setPen(pen);
  painter.drawLine(5, 7, label_->width()-5, 7);
  label_->setPixmap(pix);
/*
  QList<QAction*> actions = menuStyle_->actions();
  foreach(QAction* act, actions ){
    menuStyle_->removeAction(act);
  }

  for (int i = Qt::SolidLine; i < Qt::CustomDashLine; i++)
  {
    QPixmap pixmap(80,30);
    pixmap.fill(Qt::white);
    QLabel* l = new QLabel;
    QLinearGradient grad(pixmap.rect().topLeft(),pixmap.rect().topRight());
    grad.setColorAt(0, colorMin_);
    grad.setColorAt(1, colorMax_);

    QBrush brush(grad);
    brush.setStyle(Qt::LinearGradientPattern);
    QPen pen(brush, width_, style_);
    pen.setStyle((Qt::PenStyle)i);
    QPainter painter2(&pixmap);
    painter2.setBrush(brush);
    painter2.setPen(pen);
    painter2.drawLine(10, 15, 70, 15);
    QWidgetAction* wa = new QWidgetAction(menuStyle_);
    l->setPixmap(pixmap);
    wa->setDefaultWidget(l);
    menuStyle_->addAction(wa);
    wa->setData(i);
    connect(wa, SIGNAL(triggered()), SLOT(slotStyleChanged()));
    wa->setIconVisibleInMenu(true);
  }*/

}

void IsoSettingsWidget::clearPixmap()
{
  return;
  if ( 0 >= width() ) {
    QPixmap pix(14,14);
    pix.fill(Qt::white);
    label_->setPixmap(pix);
  }
  else {
    QPixmap pix(width(),14);
    pix.fill(Qt::white);
    label_->setPixmap(pix);
  }

}

void IsoSettingsWidget::slotExecMenu()
{
  menu_->exec(QCursor::pos());
}

void IsoSettingsWidget::slotColorMaxChanged()
{
  QColorDialog* dlg = new QColorDialog(this);
  dlg->setCurrentColor(colorMax_);
  if( dlg->exec() == QDialog::Accepted  ){
    setColorMax(dlg->currentColor());
  }
}

void IsoSettingsWidget::slotColorMinChanged()
{
  QColorDialog* dlg = new QColorDialog(this);
  dlg->setCurrentColor(colorMin_);
  if( dlg->exec() == QDialog::Accepted  ){
    setColorMin(dlg->currentColor());
  }
}

void IsoSettingsWidget::slotStyleChanged()
{
  QWidgetAction* act = qobject_cast<QWidgetAction*>(sender());
  if( 0 != act) {
    IsoMenu::changeStylePixmap(qobject_cast<QMenu*>(act->parent()), act->data().toInt());
    setStyle((Qt::PenStyle)act->data().toInt());
  }
}

void IsoSettingsWidget::slotWidthChanged()
{
  QWidgetAction* act = qobject_cast<QWidgetAction*>(sender());
  if( 0 != act ) {
    IsoMenu::changeWidthPixmap(qobject_cast<QMenu*>(act->parent()), act->data().toInt());
    setWidth(act->data().toInt());
  }
}

void IsoSettingsWidget::slotStepChanged()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if( 0 == act ){
    return;
  }
  if ( 0 == stepEdit_ ) {
    return;
  }
  QString atext = act->text();
  atext.replace( "&", "" );
  if (!atext.isEmpty()) {
    setStep(atext.toFloat());
  }
  else {
    emit stepChange(stepEdit_->value());
  }
}

void IsoSettingsWidget::slotMinChanged()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if( 0 == act ){
    return;
  }

  emit minChange(minEdit_->value());
}

void IsoSettingsWidget::slotMaxChanged()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if( 0 == act ){
    return;
  }

  emit maxChange(maxEdit_->value());
}

}
}
