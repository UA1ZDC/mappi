#include "textposdlg.h"
//#include "ui_textposdlg.h"
#include <meteo/commons/ui/custom/.ui/ui_textposdlg.h>
#include <cross-commons/debug/tmap.h>


namespace meteo {
namespace map {

//static TMap<proto::Position,QPushButton*> gPosBtn;
QList <Position> allAlignsList() {
    //возвращает список всех существующих align'ов (сделано для использования в foreach)
    QList <Position> list;
    list.append(kTopLeft);
    list.append(kTopCenter);
    list.append(kTopRight);
    list.append(kLeftCenter);
    list.append(kCenter);
    list.append(kRightCenter);
    list.append(kBottomLeft);
    list.append(kBottomCenter);
    list.append(kBottomRight);
    list.append(kNoPosition);
    return list;
}

TextPosDlg::TextPosDlg(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui::TextPoswidget)
{
  ui_->setupUi(this);

  setWindowFlags(Qt::Tool);

  posbtn_ = TMap<Position,QPushButton*>()
    << qMakePair(kTopLeft, ui_->topLeftBtn)
    << qMakePair(kTopCenter, ui_->topCenterBtn)
    << qMakePair(kTopRight, ui_->topRightBtn)
    << qMakePair(kLeftCenter, ui_->midLeftBtn)
    << qMakePair(kCenter, ui_->midCenterBtn)
    << qMakePair(kRightCenter, ui_->midRightBtn)
    << qMakePair(kBottomLeft, ui_->bottomLeftBtn)
    << qMakePair(kBottomCenter, ui_->bottomCenterBtn)
    << qMakePair(kBottomRight, ui_->bottomRightBtn)
    << qMakePair(kNoPosition, ui_->noposBtn)
  ;

  TMap<Position,QPushButton*>::const_iterator cur = posbtn_.constBegin();
  TMap<Position,QPushButton*>::const_iterator end = posbtn_.constEnd();
  for ( ; cur!=end; ++cur ) {
    QObject::connect( cur.value(), SIGNAL(clicked(bool)), SLOT(slotCurrentChanged()) );
  }

  for ( auto pos : allAlignsList() ) {
    allowedAligns_.insert(pos, true);
  }
}

TextPosDlg::~TextPosDlg()
{
  delete ui_;
  ui_ = 0;
}

void TextPosDlg::setCurrent(Position pos)
{
  for (auto position : allAlignsList()) {
    posbtn_[position]->setChecked(false);
  }
  posbtn_[pos]->setChecked(true);
  pos_ = pos;
}

Position TextPosDlg::currentPos() const
{
  return pos_;
}

void TextPosDlg::setAllowedAligns(const QMap<Position, bool> &aligns)
{
  auto it = aligns.constBegin();
  auto end = aligns.constEnd();
  for ( ; it != end; ++it ) {
    allowedAligns_.insert( it.key(), it.value() );
  }

  TMap<Position,QPushButton*>::Iterator iter;
  for ( iter = posbtn_.begin(); iter != posbtn_.end(); ++iter ) {
    if ( allowedAligns_.value(iter.key()) == true ) {
      iter.value()->setVisible(true);
    }
    else {
      iter.value()->setVisible(false);
    }
  }
  if ( allowedAligns_.value(kNoPosition) == false ) {
    // если этого не сделать, horizontalSpacer займет пустую строку
    ui_->horizontalSpacer->changeSize(0,0);
  }
}

void TextPosDlg::setAllowedAlign(Position pos, bool allow)
{
  allowedAligns_.insert( pos, allow );
  TMap<Position,QPushButton*>::Iterator iter;
  for ( iter = posbtn_.begin(); iter != posbtn_.end(); ++iter ) {
    if ( allowedAligns_.value(iter.key()) == true ) {
      iter.value()->setVisible(true);
    }
    else {
      iter.value()->setVisible(false);
    }
  }
  if ( allowedAligns_.value(kNoPosition) == false ) {
    // если этого не сделать, horizontalSpacer займет пустую строку
    ui_->horizontalSpacer->changeSize(0,0);
  }
}

QMap<Position, bool> TextPosDlg::allowedAligns()
{
  return allowedAligns_;
}

void TextPosDlg::slotCurrentChanged()
{
  QPushButton* btn = qobject_cast<QPushButton*>(sender());
  if ( 0 == btn ) {
    return;
  }
  setCurrent(posbtn_.key(btn) );
  emit posChanged(pos_);
//  pos_ = posbtn_.key(btn);
}

} // map
} // meteo
