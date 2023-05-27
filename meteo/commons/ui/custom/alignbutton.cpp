#include "alignbutton.h"

#include "textposdlg.h"

QIcon AlignButton::icoForAlign( int a )
{
  QIcon ico;
  switch (a) {
    case meteo::kTopLeft:
      ico = QIcon(":/meteo/icons/text/text-pos-top-left.png");
      break;
    case meteo::kLeftCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-mid-left.png");
      break;
    case meteo::kBottomLeft:
      ico = QIcon(":/meteo/icons/text/text-pos-bottom-left.png");
      break;
    case meteo::kBottomCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-bottom-center.png");
      break;
    case meteo::kCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-mid-center.png");
      break;
    case meteo::kTopCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-top-center.png");
      break;
    case meteo::kBottomRight:
      ico = QIcon(":/meteo/icons/text/text-pos-bottom-right.png");
      break;
    case meteo::kRightCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-mid-right.png");
      break;
    case meteo::kNoPosition:
      ico = QIcon(":/meteo/icons/misc/critical-message.png");
      break;
    case meteo::kTopRight:
    default:
      ico = QIcon(":/meteo/icons/text/text-pos-top-right.png");
      break;
  }
  return ico;
}

AlignButton::AlignButton( QWidget* p )
  : QToolButton(p),
  align_( meteo::kTopRight )
{
  QWidget::setMinimumSize( QSize(32,32) );
  QWidget::setMaximumSize( QSize(32,32) );
  QAbstractButton::setIconSize( QSize(32,32) );
  setupIcon();
  QObject::connect( this, SIGNAL( clicked() ), this, SLOT( slotClicked() ) );

  for ( auto pos : allAlignsList()) {
      allowedAligns_.insert(pos, true);
  }
}

AlignButton::~AlignButton()
{
}

void AlignButton::setAlign( int a )
{
  align_ = a;
  setupIcon();
}

void AlignButton::setAllowedAligns(const QMap<meteo::Position, bool> &aligns)
{
    // allowedAligns - отображение, в котором каждому варианту выравнивания текста
    // соответствует значение true (такое выравнивание можно использовать)
    // или false (такое выравнивание использовать нельзя и оно не отображается
    // при вызове TextPosdlg)
    // при создании AlignButton без указания разрешенных выравниваний переменная
    // allowedAligns_ заполняется значениями true для всех выравниваний
    // (в том числе чтобы не конфликтовать со старым кодом)

  auto it = aligns.constBegin();
  auto end = aligns.constEnd();
  for ( ; it != end; ++it ) {
    allowedAligns_.insert( it.key(), it.value() );
  }
}

void AlignButton::setAllowedAlign(meteo::Position pos, bool allow)
{
  allowedAligns_.insert( pos, allow );
}

QMap<meteo::Position, bool> AlignButton::allowedAligns()
{
  return allowedAligns_;
}

void AlignButton::setPoint(QPoint point)
{
  point_ = point;
  setPoint_ = true;
}

void AlignButton::setupIcon()
{
  QIcon ico = icoForAlign(align_);
  QToolButton::setIcon(ico);
}

void AlignButton::slotClicked()
{
  TextPosdlg dlg;
  if ( false != setPoint_ ) {
    dlg.move( mapToGlobal( point_ ) );
  }
  else {
    dlg.move( QWidget::mapToGlobal( QPoint(0,0) ) + QPoint( 0, height() ) );
  }
  dlg.setCurrent((meteo::Position)align_);
  dlg.setAllowedAligns(allowedAligns_);
  dlg.resize( dlg.sizeHint() );
  int res = dlg.exec();
  int pos = dlg.currentPos();
  if ( QDialog::Accepted == res ) {
    align_ = pos;
    setupIcon();
    emit posChanged(align_);
  }
}
