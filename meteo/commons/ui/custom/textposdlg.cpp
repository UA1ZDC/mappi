#include "textposdlg.h"
#include "ui_textposdlg.h"

#include <qdebug.h>

#include <cross-commons/debug/tmap.h>
#include <cross-commons/debug/tlog.h>


QList <meteo::Position> allAlignsList() {
    //возвращает список всех существующих align'ов (сделано для использования в foreach)
    QList <meteo::Position> list;
    list.append(meteo::kTopLeft);
    list.append(meteo::kTopCenter);
    list.append(meteo::kTopRight);
    list.append(meteo::kLeftCenter);
    list.append(meteo::kCenter);
    list.append(meteo::kRightCenter);
    list.append(meteo::kBottomLeft);
    list.append(meteo::kBottomCenter);
    list.append(meteo::kBottomRight);
    list.append(meteo::kNoPosition);
    return list;
}

TextPosdlg::TextPosdlg(QWidget* parent)
  : QDialog(parent),
    ui_(new Ui::TextPoswidget)
{
  ui_->setupUi(this);

  setWindowFlags(Qt::Popup);

  posbtn_ = TMap<meteo::Position,QPushButton*>()
    << qMakePair(meteo::kTopLeft, ui_->topLeftBtn)
    << qMakePair(meteo::kTopCenter, ui_->topCenterBtn)
    << qMakePair(meteo::kTopRight, ui_->topRightBtn)
    << qMakePair(meteo::kLeftCenter, ui_->midLeftBtn)
    << qMakePair(meteo::kCenter, ui_->midCenterBtn)
    << qMakePair(meteo::kRightCenter, ui_->midRightBtn)
    << qMakePair(meteo::kBottomLeft, ui_->bottomLeftBtn)
    << qMakePair(meteo::kBottomCenter, ui_->bottomCenterBtn)
    << qMakePair(meteo::kBottomRight, ui_->bottomRightBtn)
    << qMakePair(meteo::kNoPosition, ui_->noposBtn)
  ;

  TMap<meteo::Position,QPushButton*>::const_iterator cur = posbtn_.constBegin();
  TMap<meteo::Position,QPushButton*>::const_iterator end = posbtn_.constEnd();
  for ( ; cur!=end; ++cur ) {
    QObject::connect( cur.value(), SIGNAL(clicked(bool)), SLOT(slotCurrentChanged()) );
    QObject::connect( cur.value(), SIGNAL(clicked(bool)), SLOT(accept()) );
  }

  for ( auto pos : allAlignsList() ) {
    allowedAligns_.insert(pos, true);
  }
}

TextPosdlg::~TextPosdlg()
{
  delete ui_;
  ui_ = 0;
}

void TextPosdlg::setCurrent( meteo::Position pos)
{
  posbtn_[pos]->setChecked(true);
  pos_ = pos;
}

meteo::Position TextPosdlg::currentPos() const
{
  return pos_;
}

void TextPosdlg::setAllowedAligns(const QMap<meteo::Position, bool> &aligns)
{
  auto it = aligns.constBegin();
  auto end = aligns.constEnd();
  for ( ; it != end; ++it ) {
    allowedAligns_.insert( it.key(), it.value() );
  }

  TMap<meteo::Position,QPushButton*>::Iterator iter;
  for ( iter = posbtn_.begin(); iter != posbtn_.end(); ++iter ) {
    if ( allowedAligns_.value(iter.key()) == true ) {
      iter.value()->setVisible(true);
    }
    else {
      iter.value()->setVisible(false);
    }
  }
  if ( allowedAligns_.value(meteo::kNoPosition) == false ) {
    // если этого не сделать, horizontalSpacer займет пустую строку
    ui_->horizontalSpacer->changeSize(0,0);
  }
}

void TextPosdlg::setAllowedAlign(meteo::Position pos, bool allow)
{
  allowedAligns_.insert( pos, allow );
}

QMap<meteo::Position, bool> TextPosdlg::allowedAligns()
{
  return allowedAligns_;
}

void TextPosdlg::slotCurrentChanged()
{
  QPushButton* btn = qobject_cast<QPushButton*>(sender());
  if ( 0 == btn ) {
    return;
  }

  pos_ = posbtn_.key(btn);
}
