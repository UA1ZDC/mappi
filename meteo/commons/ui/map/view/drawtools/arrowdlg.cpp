#include "arrowdlg.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qmap.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

ArrowDlg::ArrowDlg(QWidget* parent)
  : QWidget(parent)
{
  setWindowFlags(Qt::Tool);

  QGridLayout* l = new QGridLayout(this);
  l->setMargin(1);
  l->setSpacing(1);

  const QList<QPair<ArrowType,QString> > kTypeIco = QList<QPair<ArrowType,QString> >()
    << qMakePair(kSimpleArrow, QString(":/meteo/icons/arrow/simple.png"))
     ;

  for ( int i=0,isz=kTypeIco.size(); i<isz; ++i ) {
    QToolButton* btn = new QToolButton(this);
    btn->setMinimumSize(QSize(24,24));
    btn->setMaximumSize(QSize(24,24));
    btn->setIconSize(QSize(24,24));
    btn->setAutoRaise(true);
    btn->setIcon(QIcon(kTypeIco[i].second));
    btn->setProperty("arr-type", kTypeIco[i].first);
    btn->setStyleSheet("border: 1px solid #999999;");

    QObject::connect( btn, SIGNAL(clicked(bool)), SLOT(slotItemSelected()) );
    QObject::connect( btn, SIGNAL(clicked(bool)), SLOT(hide()) );

    buttons_.append(btn);
  }

  for ( int i=0,isz=1; i<isz; ++i ) {
    for ( int j=0; j<1; ++j ) {
      int idx = j*isz+i;
      if ( idx < buttons_.size() ) {
        l->addWidget(buttons_[idx], j, i);
      }
    }
  }
}

QString ArrowDlg::icoForType(ArrowType type)
{
  QMap<ArrowType,QString> typeIco;
  typeIco.insert(kSimpleArrow, QString(":/meteo/icons/arrow/simple.png"));

  return typeIco[type];
}

void ArrowDlg::slotItemSelected()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn ) {
    return;
  }

  type_ = ArrowType(btn->property("arr-type").toInt());
  emit arrowTypeChanged();
}

} // map
} // meteo
