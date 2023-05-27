#include "textpropwidget.h"
#include "ui_textminwidget.h"
#include "textposdlg.h"
#include "symbdlg.h"
#include "meteo/commons/proto/meteo.pb.h"

#include <qgraphicsproxywidget.h>
#include <qcolordialog.h>
#include <qsettings.h>
#include <qdir.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

TextPropWidget::TextPropWidget(QWidget *parent) :
  PropWidget(parent),
  ui_(new Ui::TextMinWidget)
{
//  opt_ = new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/textprop.ini", QSettings::IniFormat);
  ui_->setupUi(this);
  setWindowFlags(Qt::Tool);
  alignDlg_ = new TextPosDlg();
  alignDlg_->resize(alignDlg_->sizeHint());
  alignDlg_->setCurrent(Position::kTopLeft);
  alignDlg_->setAllowedAlign(Position::kNoPosition, false );
  symbDlg_ = new SymbDlg();
  QFont font = font2qfont( prop_.font()) ;
  font.setBold(false);
  font.setItalic(false);
  font.setUnderline(false);
  font.setStrikeOut(false);
  symbDlg_->slotSetFont( font );
  ui_->delBtn->setEnabled(false);
  ui_->alignBtn->setIcon(QIcon(imgForAlign(alignDlg_->currentPos())));
  ui_->alignBtn->setIconSize(QSize(32,32));
  ui_->okBtn->setIcon(QIcon(":/meteo/icons/apply.png"));
  ui_->okBtn->setIconSize(QSize(32,32));
  ui_->delBtn->setIcon(QIcon(":/meteo/icons/delete-16.png"));
  ui_->delBtn->setIconSize(QSize(32,32));
  setDrawAlwaysIcon();
  ui_->drawAlwaysBtn->setIconSize(QSize(32,32));
  alignDlg_->hide();
  QObject::connect(ui_->colorBtn, SIGNAL(clicked(bool)), SLOT(slotShowColorDlg()));
  QObject::connect(ui_->symbBtn, SIGNAL(clicked(bool)), SLOT(slotSwitchSymb()));
  QObject::connect(ui_->fontComboBox, SIGNAL(currentFontChanged(QFont)), SLOT(slotChangeFont()));
  QObject::connect(ui_->fontSizeBox, SIGNAL(valueChanged(int)), SLOT(slotChangeFont()));
  QObject::connect(ui_->alignBtn, SIGNAL(clicked(bool)), SLOT(slotChangeFont()));
  QObject::connect(ui_->boldBtn, SIGNAL(clicked(bool)), SLOT(slotChangeFont()));
  QObject::connect(ui_->italicBtn, SIGNAL(clicked(bool)), SLOT(slotChangeFont()));
  QObject::connect(ui_->strikeBtn, SIGNAL(clicked(bool)), SLOT(slotChangeFont()));
  QObject::connect(ui_->undLineBtn, SIGNAL(clicked(bool)), SLOT(slotChangeFont()));
  QObject::connect(ui_->delBtn, SIGNAL(clicked(bool)), SLOT(slotDeleteCurrent()));
  QObject::connect(ui_->okBtn, SIGNAL(clicked(bool)), SLOT(slotFinish()));
  QObject::connect(ui_->priorBox, SIGNAL(valueChanged(int)), SLOT(slotPriorChanged()));
  QObject::connect(ui_->drawAlwaysBtn, SIGNAL(clicked(bool)), SLOT(slotDrawAlwaysChanged()));
  QObject::connect(ui_->ramkaBox, SIGNAL(currentIndexChanged(int)), SLOT(slotRamkaChanged()));
  QObject::connect(ui_->brushColorBtn, SIGNAL(clicked(bool)), SLOT(slotBrushColorChanged()));
  QObject::connect( symbDlg_, SIGNAL(symbChanged()), SLOT(slotSymbolInsert()));
  ui_->fontComboBox->installEventFilter(this);
  ui_->fontSizeBox->installEventFilter(this);

}

TextPropWidget::~TextPropWidget()
{
  delete ui_;
}
void TextPropWidget::initWidget()
{
  QGraphicsProxyWidget* parentPW = getProxyWidget();
  if ( nullptr == parentPW ) {
    return;
  }
  alignDlgPW_ = new QGraphicsProxyWidget( parentPW );
  alignDlgPW_->setWidget(alignDlg_);
  alignDlgPW_->setPos(190, 34);
  alignDlgPW_->hide();
  symbProxy_ = new QGraphicsProxyWidget(parentPW);
  symbProxy_->setWidget(symbDlg_);
  symbProxy_->setPos(0,0);
  symbProxy_->hide();
  QObject::connect(ui_->alignBtn, SIGNAL(clicked(bool)), SLOT(slotAlignSwitch()));
  QObject::connect(alignDlg_, SIGNAL(posChanged(int)), alignDlg_, SLOT(hide()));
  QObject::connect(alignDlg_, SIGNAL(posChanged(int)), SLOT(slotAlignChanged()));
  loadSettings();
}

meteo::Property TextPropWidget::toProperty() const
{
  return prop_;
}

void TextPropWidget::setProperty(const meteo::Property &prop)
{
  prop_ = prop;
  saveSettings();
}

void TextPropWidget::slotOnDelBtn()
{
  ui_->delBtn->setEnabled(true);
}

void TextPropWidget::slotOffDelBtn()
{
  ui_->delBtn->setEnabled(false);
}


void TextPropWidget::slotSetProperty( meteo::Property prop)
{
  setProperty(prop);
  setBackgroundColor( ui_->colorBtn, pen2qpen(prop_.pen()).color() );
  setBackgroundColor( ui_->brushColorBtn, brush2qbrush(prop_.brush()).color() );
  alignDlg_->setCurrent( prop_.pos() );
  ui_->alignBtn->setIcon( QIcon(imgForAlign(alignDlg_->currentPos())) );
  QFont font = font2qfont( prop_.font() );
  ui_->priorBox->setValue(prop_.priority());
  ui_->fontSizeBox->setValue(font.pointSize());
  ui_->fontComboBox->setCurrentFont(font);
  ui_->ramkaBox->setCurrentIndex(prop_.ramka());
  ui_->italicBtn->setChecked( font.italic() );
  ui_->boldBtn->setChecked( font.bold() );
  ui_->strikeBtn->setChecked( font.strikeOut() );
  ui_->undLineBtn->setChecked( font.underline() );
  ui_->drawAlwaysBtn->setChecked( prop_.draw_always() );
  saveSettings();
  emit valueChanged();
}

void TextPropWidget::slotShowColorDlg()
{
  QColorDialog dlg;
  if ( dlg.exec() == QDialog::Accepted ) {
    setBackgroundColor(ui_->colorBtn, dlg.currentColor());
    QPen pen = pen2qpen(prop_.pen());
    pen.setColor(ui_->colorBtn->palette().color(ui_->colorBtn->backgroundRole()));
    prop_.mutable_pen()->CopyFrom(qpen2pen(pen));
    saveSettings();
    emit valueChanged();
  }
}

void TextPropWidget::slotAlignChanged()
{
  ui_->alignBtn->setIcon(QIcon(imgForAlign(alignDlg_->currentPos())));
  prop_.set_pos(alignDlg_->currentPos());
  saveSettings();
  emit valueChanged();
}

void TextPropWidget::slotAlignSwitch()
{
  bool visible = alignDlg_->isVisible();
  alignDlg_->setVisible(!visible);
}

void TextPropWidget::slotSwitchSymb()
{
  if (nullptr == symbProxy_) {
    return;
  }
  if ( true == symbProxy_->isVisible() ) {
    symbProxy_->hide();
  }
  else {
    QFont font = font2qfont( prop_.font()) ;
    font.setBold(false);
    font.setItalic(false);
    font.setUnderline(false);
    font.setStrikeOut(false);
    symbDlg_->slotSetFont( font );
    symbProxy_->show();
  }
}

void TextPropWidget::slotSymbolInsert()
{
  if ( nullptr != symbDlg_ ) {
    emit insertSymb(symbDlg_->currentSymb());
  }
}

void TextPropWidget::slotRamkaChanged()
{
  prop_.set_ramka(static_cast<TextRamka>(ui_->ramkaBox->currentIndex()) );
  saveSettings();
  emit valueChanged();
}

void TextPropWidget::slotPriorChanged()
{
  prop_.set_priority(ui_->priorBox->value());
  saveSettings();
  emit valueChanged();
}

void TextPropWidget::slotDrawAlwaysChanged()
{
  setDrawAlwaysIcon();
  prop_.set_draw_always(ui_->drawAlwaysBtn->isChecked());
  saveSettings();
  emit valueChanged();
}

void TextPropWidget::slotBrushColorChanged()
{
  QColorDialog dlg;
  if ( dlg.exec() == QDialog::Accepted ) {
    setBackgroundColor(ui_->brushColorBtn, dlg.currentColor());
    QBrush brush;
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    brush.setColor(dlg.currentColor());
    prop_.mutable_brush()->CopyFrom(qbrush2brush(brush));
    saveSettings();
    emit valueChanged();
  }
}

void TextPropWidget::slotChangeFont()
{
  QFont font = font2qfont(prop_.font());
  font = ui_->fontComboBox->currentFont();
  font.setPointSize(ui_->fontSizeBox->value());
  font.setBold(ui_->boldBtn->isChecked());
  font.setItalic(ui_->italicBtn->isChecked());
  font.setUnderline(ui_->undLineBtn->isChecked());
  font.setStrikeOut(ui_->strikeBtn->isChecked());
  prop_.mutable_font()->CopyFrom( qfont2font(font) );
  saveSettings();
  emit valueChanged();
}

void TextPropWidget::slotDeleteCurrent()
{
  emit deleteCurrent();
}

void TextPropWidget::slotFinish()
{
  emit finish();
}

QString TextPropWidget::imgForAlign(int pos)
{
  QString imgPath;
  switch (pos) {
    case kTopLeft:
      imgPath = QString(":/meteo/icons/text/text-pos-top-left.png");
      break;
    case kLeftCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-left.png");
      break;
    case kBottomLeft:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-left.png");
      break;
    case kBottomCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-center.png");
      break;
    case kCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-center.png");
      break;
    case kTopCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-top-center.png");
      break;
    case kBottomRight:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-right.png");
      break;
    case kRightCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-right.png");
      break;
    case kNoPosition:
      imgPath = QString(":/meteo/icons/misc/critical-message.png");
      break;
    case kTopRight:
    default:
      imgPath = QString(":/meteo/icons/text/text-pos-top-right.png");
      break;
  }
  return imgPath;
}

void TextPropWidget::defaultProperty()
{
  setBackgroundColor(ui_->colorBtn, QColor("black"));
  setBackgroundColor(ui_->brushColorBtn, QColor("white"));
  QFont font = font2qfont(prop_.font());
  font = ui_->fontComboBox->currentFont();
  font.setPointSize(14);
  prop_.mutable_font()->CopyFrom( qfont2font(font) );
  slotAlignChanged();
  QPen pen = pen2qpen(prop_.pen());
  pen.setColor(ui_->colorBtn->palette().color(ui_->colorBtn->backgroundRole()));
  prop_.mutable_pen()->CopyFrom(qpen2pen(pen));
  QBrush brush = brush2qbrush(prop_.brush());
  brush.setColor(ui_->brushColorBtn->palette().color(ui_->brushColorBtn->backgroundRole()));
  prop_.mutable_brush()->CopyFrom(qbrush2brush(brush));
}

void TextPropWidget::setDrawAlwaysIcon()
{
  if ( true == ui_->drawAlwaysBtn->isChecked() ) {
    ui_->drawAlwaysBtn->setIcon(QIcon(":/meteo/icons/tools/layer_visible.png"));
    ui_->drawAlwaysBtn->setToolTip(QObject::tr("Текст всегда отображается"));
  }
  else {
    ui_->drawAlwaysBtn->setIcon(QIcon(":/meteo/icons/tools/set_layer_hidden.png"));
    ui_->drawAlwaysBtn->setToolTip(QObject::tr("Текст скрывается"));
  }
}

void TextPropWidget::loadSettings()
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    return;
  }
  if (true == opt_->contains("textprop")) {
    QByteArray prop = opt_->value("textprop").toByteArray();
    std::string string( prop.constData(), prop.length() );
    prop_.ParseFromString(string);
    slotSetProperty(prop_);
  }
  else {
    defaultProperty();
  }
}

void TextPropWidget::saveSettings()
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    return;
  }
  std::string prop;
  if (true == prop_.SerializeToString(&prop)) {
    opt_->setValue("textprop", QByteArray(prop.c_str(), prop.length()));
  }
  else {
    error_log << QObject::tr("Ошибка сериализациии настроек.");
    return;
  }
}

}
}


