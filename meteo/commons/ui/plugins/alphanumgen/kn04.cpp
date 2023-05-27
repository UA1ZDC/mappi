#include "kn04.h"
#include "ui_kn04.h"
#include "alphanumgen.h"

#include <qdir.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qfiledialog.h>

#include <meteo/commons/global/dateformat.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/ui/custom/saveiface.h>
#include <meteo/commons/proto/msgcenter.pb.h>

#include <math.h>

#define TEMPLATE_A_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn04a.templ")
#define TEMPLATE_C_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn04c.templ")
#define TEMPLATE_B_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn04b.templ")
#define TEMPLATE_D_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn04d.templ")
#define HELP_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn04.html")
#define CONF_FILE QDir::homePath() + "/.meteo/" + "kn04generate.conf"

using namespace meteo;

enum CloudMode {
  kSkyClear = 0,
  kSkyInvisible = 1,
  kSkyInvisOther = 2,
  kSkyNoMeas = 3,
  kSkyCustom = 4
};

enum LevelColumn {
  kType  = 0,
  kLevel = 1,
  kHeight = 2,
  kTemp  = 3,
  kDefic = 4,
  kdd    = 5,
  kff    = 6
};

enum LevelType {
  kDefault = -1,
  kStation = 0,
  kStandart = 1,
  kTropo    = 2,
  kMaxWind  = 3,
  kSign     = 4
};

Kn04::Kn04( QWidget* parent, Qt::WindowFlags fl ):
  QWidget(parent, fl),
  ui_(new Ui::Kn04),
  gen_(new AlphanumGen(TEMPLATE_A_FILE))
{
  ui_->setupUi(this);
  ui_->dt->setDisplayFormat(meteo::dtHumanFormatTimeShort);
  ui_->GGgg->setDisplayFormat(meteo::dtHumanFormatTimeOnly);
  QObject::connect(ui_->accept, SIGNAL(clicked()), SLOT(slotSaveTlg()));
  QObject::connect(ui_->sendInMsg, SIGNAL(clicked()), SLOT(slotSendTlg()));
  initWidgets();
  setConnects();
  setDefault();
  for (int idx = ui_->decodeView->columnCount() - 1; idx >=0; --idx) {
    ui_->decodeView->resizeColumnToContents(idx);
  }
}

Kn04::~Kn04()
{
  if (nullptr != ui_) {
    delete ui_;
    ui_ = nullptr;
  }

  if (nullptr != gen_) {
    delete gen_;
    gen_ = nullptr;
  }

  if (nullptr != helpUi_) {
    delete helpUi_;
    helpUi_ = nullptr;
  }
  
}

void Kn04::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    close();
  }
}

void Kn04::initWidgets()
{
  ui_->decodeView->hideColumn(1);

  QIntValidator* v = new QIntValidator(ui_->IIiii);
  v->setRange(1, 99999);
  ui_->IIiii->setValidator(v);

  // QRegExpValidator* vt1t2 = new QRegExpValidator(QRegExp("[A-Za-z][A-Za-z]"));
  // _ui->T1T2->setValidator(vt1t2);

  QRegExpValidator* va1a2 = new QRegExpValidator(QRegExp("[A-Za-z][A-Za-z]"));
  ui_->A1A2->setValidator(va1a2);

  // QRegExpValidator* vcccc = new QRegExpValidator(QRegExp("[A-Za-z]{4}"));
  // _ui->CCCC->setValidator(vcccc);

  QRegExpValidator* vbbb = new QRegExpValidator(QRegExp("(?:[rR]{2}|[Cc]{2}|[Aa]{2})[A-Za-z]|$"));
  ui_->BBB->setValidator(vbbb);

  QIntValidator* vii = new QIntValidator(ui_->ii);
  vii->setRange(0, 99);
  ui_->ii->setValidator(vii);
  

  for (int idx = 0; idx < ui_->CL->count() - 1; idx ++) {
    ui_->CL->setItemData(idx, QString::number(idx));
  }
  ui_->CL->setItemData(ui_->CL->count() - 1, "/");

  for (int idx = 0; idx < ui_->CM->count() - 1; idx ++) {
    ui_->CM->setItemData(idx, QString::number(idx));
  }
  ui_->CM->setItemData(ui_->CM->count() - 1, "/");

  for (int idx = 0; idx < ui_->CH->count() - 1; idx ++) {
    ui_->CH->setItemData(idx, QString::number(idx));
  }
  ui_->CH->setItemData(ui_->CH->count() - 1, "/");


  slotAddLevelLine(kStandart);

  ui_->levels->resizeColumnsToContents();

  ui_->shearMode2->setEnabled(false);
  // for (int i = 0, sz = _ui->levels->columnCount(); i < sz; ++i) {
  //   _ui->levels->resizeColumnToContents(i);
  // }

}

void Kn04::setConnects()
{
  QObject::connect(ui_->help, SIGNAL(toggled(bool)), SLOT(slotHelp(bool)));
  QObject::connect(ui_->cancel, SIGNAL(clicked()), SLOT(close()));

  QObject::connect(ui_->part, SIGNAL(currentIndexChanged(int)), SLOT(slotPartChanged(int)));

  QObject::connect(ui_->cancel, SIGNAL(clicked()), SLOT(close()));
  QObject::connect(ui_->IIiii, SIGNAL(textChanged(const QString&)), SLOT(slotSetValue(const QString&)));
  QObject::connect(ui_->dt, SIGNAL(dateTimeChanged(const QDateTime&)), SLOT(slotDateTimeChanged(const QDateTime&)));

  // QObject::connect(_ui->T1T2, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->A1A2, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->ii, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->CCCC, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->BBB, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));

  QObject::connect(ui_->cloudMode, SIGNAL(currentIndexChanged(int)), SLOT(slotCloudsChanged()));
  QObject::connect(ui_->Nh, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->h, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->CL, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->CM, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->CH, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));

  QObject::connect(ui_->sr, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->rara, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->sasa, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->GGgg, SIGNAL(timeChanged(const QTime&)), SLOT(slotGGggChanged(const QTime&)));

  QObject::connect(ui_->shearMode, SIGNAL(toggled(bool)), SLOT(slotShearChanged(bool)));
  QObject::connect(ui_->vbvb, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->vava, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->shearMode2, SIGNAL(toggled(bool)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->vbvb2, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->vava2, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));

  QObject::connect(ui_->levels, SIGNAL(cellChanged(int, int)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->a4, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));

  QObject::connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotAddLevelLine()));
  QObject::connect(ui_->removeBtn, SIGNAL(clicked()), SLOT(slotRmLevelLine()));
}

void Kn04::setDefault()
{
  ui_->dt->setDateTime(QDateTime::currentDateTimeUtc());
  ui_->IIiii->setText(QString::number(meteo::global::currentStationIndex()));
  ui_->ii->setText("0");
  ui_->CCCC->setText(meteo::global::wmoId());

  slotPartChanged(0);
}

//! Ячейка с целочисленными значениями
void Kn04::slotSetIntCell(int row, int col, int min, int max, const int* value /* = 0*/)
{
  QLineEdit *le = new QLineEdit(ui_->levels);
  le->setFrame(false);
  QIntValidator* leValid = new QIntValidator(min, max);
  le->setValidator(leValid);
  if (0 != value) {
    le->setText(QString::number(*value));
    le->setReadOnly(true);
  }
  connect(le, SIGNAL(textChanged(const QString&)), SLOT(slotLevelsChanged()));
  ui_->levels->setCellWidget(row, col, le);
}

//! Ячейка с значениями с плавающей запятой
void Kn04::slotSetDoubleCell(int row, int col, double min, double max, int decimals)
{
  QLineEdit *le = new QLineEdit(ui_->levels);
  le->setFrame(false);
  QDoubleValidator* leValid = new QDoubleValidator(min, max, decimals);
  leValid->setNotation(QDoubleValidator::StandardNotation);
  le->setValidator(leValid);
  connect(le, SIGNAL(textChanged(const QString&)), SLOT(slotLevelsChanged()));
  ui_->levels->setCellWidget(row, col, le);
}

//! Ячейка с валидатором, заданным регулярным выражением
void Kn04::slotSetRegexpCell(int row, int col, const QRegExp& rx)
{
  QLineEdit *le = new QLineEdit(ui_->levels);
  le->setFrame(false);
  QRegExpValidator* leValid = new QRegExpValidator(rx);
  le->setValidator(leValid);
  connect(le, SIGNAL(textChanged(const QString&)), SLOT(slotLevelsChanged()));
  ui_->levels->setCellWidget(row, col, le);

}

void Kn04::slotAddLevelLine(int levType /* = -1*/, const int* level /*= 0*/)
{
  int row = ui_->levels->rowCount();
  ui_->levels->setRowCount(row + 1);

  QList<QPair<QString, LevelType> > type;
  int partIdx = ui_->part->currentIndex();

  if (levType == kStation) {
    type << QPair<QString, LevelType>(QObject::tr("Станция"), kStation);
  } else if (levType == kStandart) {
    type << QPair<QString, LevelType>(QObject::tr("Стандартный"), kStandart);
  } else if (partIdx == 0 || partIdx == 2) {
    type << QPair<QString, LevelType>(QObject::tr("Тропопауза"),  kTropo)
        << QPair<QString, LevelType>(QObject::tr("Макс. ветер"), kMaxWind);
  } else {
    type << QPair<QString, LevelType>(QObject::tr("Особая точка"), kSign);
  }

  QComboBox* wtype = new QComboBox(ui_->levels);
  wtype->setStyleSheet("QComboBox {	   \
      border: 0px solid gray;		   \
      padding: 1px 18px 1px 3px;	   \
      min-width: 6em;				\
    }						\
    QComboBox::drop-down {			\
      width: 15px;				\
      border-left-width: 0px;			\
    }							\
    QComboBox::down-arrow {					\
      image: url(:/meteo/icons/arrow/ctrlarrowdown.png);	\
  }");

  
  int idx = 0;
  for (int fidx = 0; fidx < type.size(); fidx++) {
    wtype->insertItem(idx, type.at(fidx).first, type.at(fidx).second);
    idx++;
  }
  wtype->setCurrentIndex(0);
  connect(wtype, SIGNAL(currentIndexChanged(int)), SLOT(slotLevelsChanged()));
  ui_->levels->setCellWidget(row, kType, wtype);
  if (type.size() == 1) {
    wtype->setFrame(false);
  }


  if (ui_->part->currentIndex() == 2) {
    slotSetIntCell(row, kLevel, 0, 1999, level);
  } else {
    slotSetDoubleCell(row, kLevel, 0, 1999, 1);
  }

  slotSetRegexpCell(row, kHeight, QRegExp("-?[\\d/]{1,5}"));
  slotSetRegexpCell(row, kTemp, QRegExp("-?[\\d/]{1,2}(?:[.][\\d]|/)?"));
  slotSetDoubleCell(row, kDefic, 0, 49, 1);
  slotSetRegexpCell(row, kdd, QRegExp("~|//|[0-2]?[\\d]{1,2}|(?!360)3[0-5][0-9]"));
  slotSetRegexpCell(row, kff, QRegExp("///|[1-3]?[\\d]{1,2}"));

  slotLevelsChanged();
}

void Kn04::slotRmLevelLine()
{
  //  trc;
  int row = ui_->levels->rowCount() - 1; //_ui->levels->currentRow();
  if (row < 0) return;
  //var(row);

  QComboBox* cb = qobject_cast<QComboBox*>(ui_->levels->cellWidget(row, kType));
  if (0 == cb) return;
  LevelType type = LevelType(cb->itemData(cb->currentIndex()).toInt());
  //var(type);
  if (type == kTropo || type == kMaxWind || type == kSign) {
    ui_->levels->clearSelection();
    ui_->levels->removeRow(row);
    slotLevelsChanged();
  }
}


void Kn04::slotGenerate()
{
  if (0 == gen_) {
    return;
  }

  QString code = gen_->generate();
  if (code.isEmpty()) {
    ui_->codeView->setText(""); //TODO error text
  } else {
    ui_->codeView->setText(code);
  }

  QByteArray ba;
  ba.append(code);
  gen_->decode(ba, ui_->decodeView);
}


void Kn04::slotSetValue(const QString& tval)
{
  if (0 == gen_) {
    return;
  }

  QObject* obj = QObject::sender();
  if (tval.isEmpty()) {
    gen_->clearValue(obj->objectName());
  } else {
    gen_->setValue(obj->objectName(), tval);
  }

  slotGenerate();
}

void Kn04::slotSetValue(int tval)
{
  if (0 == gen_ || 0 == QObject::sender()) {
    return;
  }

  gen_->setValue(QObject::sender()->objectName(), tval);
  
  slotGenerate();
}

void Kn04::slotSetValue(double tval)
{
  if (0 == gen_ || 0 == QObject::sender()) {
    return;
  }

  gen_->setValue(QObject::sender()->objectName(), tval);
  
  slotGenerate();
}

bool Kn04::checkParams(const QLineEdit& le, const QString& error)
{
  QString text = le.text();
  int pos = 0;
  if (// text.isEmpty() ||
      (0 != le.validator() && QValidator::Acceptable != le.validator()->validate(text, pos)) ) {
    QMessageBox::information(this, QObject::tr("Не заполнены обязательные поля"),
                             QObject::tr("Некорректное значение %1").arg(error),
                             QMessageBox::Ok);
    return false;
  }

  return true;
}

void Kn04::slotDateTimeChanged(const QDateTime& dt)
{
  if (0 == gen_) {
    return;
  }
  gen_->setValue("YY", dt.date().day());
  gen_->setValue("GG", dt.time().hour());
  // _gen->setValue("YY1", dt.date().day());
  // _gen->setValue("GG1", dt.time().hour());
  // _gen->setValue("gg1", dt.time().minute());

  slotGenerate();
}

void Kn04::slotCloudsChanged()
{
  if (0 == gen_) {
    return;
  }

  if (ui_->cloudsGroup->isEnabled() == false) {
    // _gen->clearValue("h");
    // _gen->clearValue("N");
    // _gen->clearValue("CL");
    // _gen->clearValue("CM");
    // _gen->clearValue("CH");
    return;
  }

  if (ui_->cloudMode->currentIndex() == kSkyCustom) {
    ui_->Nh->setEnabled(true);
    ui_->h->setEnabled(true);
    ui_->CL->setEnabled(true);
    ui_->CM->setEnabled(true);
    ui_->CH->setEnabled(true);
    gen_->setValue("h", ui_->h->value());
    gen_->setValue("Nh", ui_->Nh->value());
    gen_->setValue("CL", ui_->CL->currentIndex());
    gen_->setValue("CM", ui_->CM->currentIndex());
    gen_->setValue("CH", ui_->CH->currentIndex());

    slotGenerate();
    return;
  }

  ui_->Nh->setEnabled(false);
  ui_->h->setEnabled(false);
  ui_->CL->setEnabled(false);
  ui_->CM->setEnabled(false);
  ui_->CH->setEnabled(false);

  switch (ui_->cloudMode->currentIndex()) {
  case kSkyClear:
    gen_->setCode("h", "9");
    gen_->setCode("Nh", "0");
    break;
  case kSkyInvisible:
    ui_->h->setEnabled(true);
    gen_->setValue("h", ui_->h->value());
    gen_->setCode("Nh", "9");
    break;
  case kSkyInvisOther: case kSkyNoMeas:
    gen_->clearValue("h");
    gen_->clearValue("Nh");
    break;
  }

  gen_->clearValue("CL");
  gen_->clearValue("CM");
  gen_->clearValue("CH");

  slotGenerate();
}

void Kn04::slotPartChanged(int part)
{
  //TODO message, что данные не сохранены/отправлены
  if (0 == gen_) return;

  ui_->levels->clearContents();
  ui_->levels->setRowCount(0);

  switch (part) {
  case 0:
    gen_->readTemplate(TEMPLATE_A_FILE);
    ui_->cloudsGroup->setEnabled(false);
    ui_->shearMode->setEnabled(true);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->a4->setEnabled(false);
    ui_->T1T2->setText("US");
    gen_->setCode("MMMM", "TTAA");
    break;
  case 1:
    gen_->readTemplate(TEMPLATE_B_FILE);
    ui_->cloudsGroup->setEnabled(true);
    ui_->shearMode->setEnabled(false);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->a4->setEnabled(true);
    gen_->setValue("a4", ui_->a4->currentIndex());
    gen_->setCode("MMMM", "TTBB");
    ui_->T1T2->setText("UK");
    break;
  case 2:
    gen_->readTemplate(TEMPLATE_C_FILE);
    ui_->cloudsGroup->setEnabled(false);
    ui_->shearMode->setEnabled(true);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->a4->setEnabled(false);
    gen_->setCode("MMMM", "TTCC");
    ui_->T1T2->setText("UH");
    break;
  case 3:
    gen_->readTemplate(TEMPLATE_D_FILE);
    ui_->cloudsGroup->setEnabled(false);
    ui_->shearMode->setEnabled(false);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->a4->setEnabled(false);
    gen_->setCode("MMMM", "TTDD");
    ui_->T1T2->setText("UQ");
    gen_->clearValue("a4");
    break;
  }

  slotDateTimeChanged(ui_->dt->dateTime());
  // _gen->setValue("CCCC", meteo::global::wmoId());
  gen_->setValue("IIiii", ui_->IIiii->text());
  // _gen->setValue("ii", _ui->ii->text());
  slotGGggChanged(ui_->GGgg->time());
  gen_->setValue("sr", ui_->sr->value());
  gen_->setValue("rara", ui_->rara->value());
  gen_->setValue("sasa", ui_->sasa->value());
  fillPartDefault(part);
  slotCloudsChanged();

  slotGenerate();
}

void Kn04::fillPartDefault(int part)
{
  if (part == 0 || part == 1) {
    slotAddLevelLine(kStation);
  }
  
  if (part == 1 || part == 3) {
    slotAddLevelLine(kSign);
  }

  QList<int> levels;
  if (part == 0) {
    levels << 1000 << 925 << 850 << 700 << 500 << 400 << 300 << 250 << 200 << 150 << 100;
  } else if (part == 2) {
    levels << 70 << 50 << 30 << 20 << 10;
  }

  int lidx = ui_->levels->rowCount() - 1;
  for (int idx = 0; idx < levels.count(); idx++) {
    slotAddLevelLine(kStandart, &levels.at(idx));
    ++lidx;

    // QTableWidgetItem* item = new QTableWidgetItem();
    // if (0 == item) return;
    // item->setText(QString::number(levels.at(idx)));
    // _ui->levels->setItem(lidx, kLevel, item);
  }
  
}

void Kn04::slotShearChanged(bool toggl)
{
  if (toggl && ui_->part->currentIndex() == 0) {
    ui_->shearMode2->setEnabled(true);
  } else {
    ui_->shearMode2->setEnabled(false);
  }

  slotLevelsChanged();
}

void Kn04::slotGGggChanged(const QTime& time)
{
  gen_->setValue("GGz", time.hour());
  gen_->setValue("ggz", time.minute());
  
  slotGenerate();
}

void Kn04::slotSetLevelValue(const QString& name, int row, int col)
{
  if (0 == gen_) { return; }

  QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
  if (0 == leLevel) return;

  QString level = leLevel->text();
  if (level.isEmpty()) return;

  QLineEdit* le = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, col));
  if (0 == le) return;

  gen_->setValue(name, le->text());
}

void Kn04::slotSetWind(const QString& ddname, const QString& ffname, int row)
{
  if (0 == gen_) { return; }

  QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
  if (0 == leLevel) return;

  QString level = leLevel->text();
  if (level.isEmpty()) return;

  QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
  if (0 == ledd) return;

  QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
  if (0 == leff) return;

  QString dd, ff;
  anc::func_ddfff(ledd->text(), leff->text(), &dd, &ff);
  if (!dd.isEmpty()) {
    gen_->setValue(ddname, dd);
  }
  if (!ff.isEmpty()) {
    gen_->setValue(ffname, ff);
  }
}

void Kn04::slotLevelsChanged()
{
  if (gen_ == 0) return;

  int lastP = 9999;

  if (ui_->part->currentIndex() == 0 || ui_->part->currentIndex() == 1 ) {
    gen_->clearValue("P0P0P0");
    gen_->clearValue("T0T0Ta0");
    gen_->clearValue("D0D0");
    gen_->clearValue("d0d0");
    gen_->clearValue("f0f0f0");
    if (ui_->part->currentIndex() == 1) {
      gen_->clearValue("P0P0P0_");
    }
  }

  if (ui_->part->currentIndex() == 1 || ui_->part->currentIndex() == 3) {
    gen_->clearGroupSet("signTemp");
    gen_->clearGroupSet("signWind");
  } else {
    gen_->clearGroupSet("standart");
    gen_->clearGroupSet("tropo");
    gen_->clearGroupSet("wind");
    gen_->clearValue("Id");
    lastP = setId();
  }

  // _gen->clearValue("");
  int nntemp = 0, nnwind = 0;
  int maxWindCnt = 0;

  for (int row = 0; row < ui_->levels->rowCount(); row++) {
    QComboBox* cb = qobject_cast<QComboBox*>(ui_->levels->cellWidget(row, kType));
    if (0 == cb) { continue; }
    LevelType type = LevelType(cb->itemData(cb->currentIndex()).toInt());
    switch (type) {
    case kStation:
      slotSetLevelValue("P0P0P0", row, kLevel);
      slotSetLevelValue("T0T0Ta0", row, kTemp);
      slotSetLevelValue("D0D0", row, kDefic);
      slotSetWind("d0d0", "f0f0f0", row);
      if (ui_->part->currentIndex() == 1 || ui_->part->currentIndex() == 3) {
        slotSetLevelValue("P0P0P0_", row, kLevel);
      }
      break;
    case kStandart: {
      QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
      QLineEdit* leh = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kHeight));
      QLineEdit* let = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kTemp));
      QLineEdit* ledt = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kDefic));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
      if (0 == leLevel || leLevel->text().isEmpty() || 0 == leh || 0 == let ||
          0 == ledt || 0 == ledd || 0 == leff) {
        break;
      }
      int idx = gen_->addGroupSetValue("standart", "P1P1", leLevel->text().toInt());
      if (!leh->text().isEmpty()) {
        int h = leh->text().toInt();
        if (h < 0) {
          h = 500 + abs(h);
        } else if (leLevel->text().toInt() < 500) {
          h = round(h/10.0);
        }
        gen_->setGroupSetValue("standart", idx, "h1h1h1", h % 1000);
      }
      if (!let->text().isEmpty()) { gen_->setGroupSetValue("standart", idx, "T1T1Ta1",let->text()); }
      if (!ledt->text().isEmpty()){ gen_->setGroupSetValue("standart", idx, "D1D1", ledt->text());  }
      QString dd, ff;
      anc::func_ddfff(ledd->text(), leff->text(), &dd, &ff);
      if (!dd.isEmpty()) {
        if (ledd->text() == "//") {
          gen_->setGroupSetRequire("standart", idx, "d1d1", true);
        }
        gen_->setGroupSetValue("standart", idx, "d1d1", dd);
      } else if (lastP <= leLevel->text().toInt()) {
        gen_->setGroupSetRequire("standart", idx, "d1d1", true);
        gen_->setGroupSetValue("standart", idx, "d1d1", "//");
      }
      if (!ff.isEmpty()) {
        if (leff->text() == "///") {
          gen_->setGroupSetRequire("standart", idx, "f1f1f1", true);
        }
        gen_->setGroupSetValue("standart", idx, "f1f1f1", ff);
      } else if (lastP <= leLevel->text().toInt()) {
        gen_->setGroupSetRequire("standart", idx, "f1f1f1", true);
        gen_->setGroupSetValue("standart", idx, "f1f1f1", "///");
      }
    }
      break;
    case kTropo: {
      QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
      QLineEdit* let = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kTemp));
      QLineEdit* ledt = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kDefic));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
      if (0 == leLevel || leLevel->text().isEmpty() || 0 == let ||
          0 == ledt || 0 == ledd || 0 == leff) {
        break;
      }
      int idx = gen_->addGroupSetValue("tropo", "PtPtPt", leLevel->text().toInt());
      if (!let->text().isEmpty()) { gen_->setGroupSetValue("tropo", idx, "TtTtTat",let->text()); }
      if (!ledt->text().isEmpty()){ gen_->setGroupSetValue("tropo", idx, "DtDt", ledt->text());  }
      QString dd, ff;
      anc::func_ddfff(ledd->text(), leff->text(), &dd, &ff);
      if (!dd.isEmpty()) {gen_->setGroupSetValue("tropo", idx, "dtdt", dd); }
      if (!ff.isEmpty()) {gen_->setGroupSetValue("tropo", idx, "ftftft", ff); }
    }
      break;
    case kMaxWind: {
      QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
      if (0 == leLevel || leLevel->text().isEmpty() || 0 == ledd || 0 == leff) {
        break;
      }
      
      int idx = gen_->addGroupSetValue("wind", "PmPmPm", leLevel->text().toInt());
      QString dd, ff;
      anc::func_ddfff(ledd->text(), leff->text(), &dd, &ff);
      if (!dd.isEmpty()) {gen_->setGroupSetValue("wind", idx, "dmdm", dd); }
      if (!ff.isEmpty()) {gen_->setGroupSetValue("wind", idx, "fmfmfm", ff); }
      if (ui_->shearMode->isEnabled() && ui_->shearMode->isChecked() && maxWindCnt == 0) {
        gen_->setGroupSetValue("wind", idx, "vava", ui_->vava->value());
        gen_->setGroupSetValue("wind", idx, "vbvb", ui_->vbvb->value());
      }
      if (ui_->shearMode2->isEnabled() && ui_->shearMode2->isChecked() && maxWindCnt == 1) {
        gen_->setGroupSetValue("wind", idx, "vava", ui_->vava2->value());
        gen_->setGroupSetValue("wind", idx, "vbvb", ui_->vbvb2->value());
      }
      maxWindCnt++;
    }
      break;
    case kSign: {
      QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
      QLineEdit* let = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kTemp));
      QLineEdit* ledt = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kDefic));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));

      if (0 == leLevel || leLevel->text().isEmpty() || 0 == let ||
          0 == ledt || 0 == ledd || 0 == leff) break;
      if (!let->text().isEmpty() || !ledt->text().isEmpty()) {
        ++nntemp;
        int idx = gen_->addGroupSetValue("signTemp", "P1P1P1", leLevel->text().toFloat());
        gen_->setGroupSetValue("signTemp", idx, "n1n1", QString::number(nntemp) + QString::number(nntemp));

        if (!let->text().isEmpty()) {
          gen_->setGroupSetValue("signTemp", idx, "T1T1Ta1", let->text());
        }
        if (!ledt->text().isEmpty()) {
          gen_->setGroupSetValue("signTemp", idx, "D1D1", ledt->text());
        }
      }
      if (!ledd->text().isEmpty() || !leff->text().isEmpty()) {
        ++nnwind;
        int idx = gen_->addGroupSetValue("signWind", "P1P1P1", leLevel->text().toInt());
        gen_->setGroupSetValue("signWind", idx, "n1n1", QString::number(nnwind) + QString::number(nnwind));
        QString dd, ff;
        anc::func_ddfff(ledd->text(), leff->text(), &dd, &ff);
        if (!dd.isEmpty()) {
          if (ledd->text() == "//") {
            gen_->setGroupSetRequire("signWind", idx, "d1d1", true);
          }
          gen_->setGroupSetValue("signWind", idx, "d1d1", dd);
        }
        if (!ff.isEmpty()) {
          if (leff->text() == "///") {
            gen_->setGroupSetRequire("signWind", idx, "f1f1f1", true);
          }
          gen_->setGroupSetValue("signWind", idx, "f1f1f1", ff);
        }
      }
    }
      break;
    default: {}
    }
    
  }

  slotGenerate();
}

//! Установка последней изобарической поверхности, по которой сообщаются данные о ветре
int Kn04::setId()
{
  int lastP = 9999;

  for (int row = 0; row < ui_->levels->rowCount(); row++) {
    QComboBox* cb = qobject_cast<QComboBox*>(ui_->levels->cellWidget(row, kType));
    if (0 == cb) { continue; }
    LevelType type = LevelType(cb->itemData(cb->currentIndex()).toInt());
    if (type != kStandart) {
      continue;
    }

    QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
    QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
    QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
    if (0 != ledd && 0 != leff && (!ledd->text().isEmpty() || !leff->text().isEmpty())) {
      lastP = leLevel->text().toInt();
    }
  }
  if (lastP == 250) lastP = 200;
  if (lastP == 150) lastP = 100;

  if (lastP != 9999) {
    if (lastP == 1000) {
      gen_->setValue("Id", 0);
    } else if (lastP < 100) {
      gen_->setValue("Id", lastP / 10);
    } else {
      gen_->setValue("Id", lastP / 100);
    }
  }

  return lastP;
}

bool Kn04::checkCorrectInput()
{
  if (!checkParams(*ui_->IIiii, QObject::tr("индекса станции"))) return false;
  //if (!checkParams(*_ui->T1T2, QObject::tr("T1T2"))) return false;
  if (!checkParams(*ui_->A1A2, QObject::tr("A1A2"))) return false;
  if (!checkParams(*ui_->ii, QObject::tr("ii"))) return false;
  //  if (!checkParams(*_ui->CCCC, QObject::tr("CCCC"))) return false;
  if (!checkParams(*ui_->BBB, QObject::tr("BBB"))) return false;
  return true;
}

void Kn04::slotSaveTlg()
{
  if (!checkCorrectInput()) return;
  QFileDialog diag;
  diag.setDirectory(QDir::home());
  diag.setFileMode( QFileDialog::AnyFile );
  diag.setAcceptMode( QFileDialog::AcceptSave );
  QString defaultName = ui_->T1T2->text() + ui_->A1A2->text() + ui_->ii->text().rightJustified(2, '0');
  defaultName = defaultName.toUpper();
  diag.selectFile(defaultName + ui_->dt->dateTime().toString("ddhhmm") + ".tlg");
  diag.setNameFilters(QStringList{"Метеорологические сообщения (*.tlg)","Все (*)"});
  if ( diag.exec() ) {
    auto files = diag.selectedFiles();
    auto fileName = files.first();
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
      error_log << msglog::kFileOpenError.arg(fileName).arg(file.error());
      QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
                            QObject::tr("Не удалось записать файл %1").arg(fileName));
      return;
    }
    file.write(ui_->codeView->toPlainText().toStdString().data());
    file.close();
    return;
  }
}

void Kn04::slotSendTlg()
{
  if (!checkCorrectInput()) return;
  QString defaultName = ui_->T1T2->text() + ui_->A1A2->text() +
      ui_->ii->text().rightJustified(2, '0') + ui_->dt->dateTime().toString("ddhhmm") + ".tlg";;
  saveInnerDoc(defaultName);
}

bool Kn04::saveInnerDoc(const QString& name)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  tlg::MessageNew req;
  tlg::Header* header = req.mutable_header();

  header->set_t1(ui_->T1T2->text().left(1).toStdString());
  header->set_t2(ui_->T1T2->text().right(1).toStdString());
  header->set_a1(ui_->A1A2->text().left(1).toUpper().toStdString());
  header->set_a2(ui_->A1A2->text().right(1).toUpper().toStdString());
  if (!ui_->BBB->text().isEmpty()) {
    header->set_bbb(ui_->BBB->text().toUpper().toStdString());
  }
  header->set_cccc(ui_->CCCC->text().toStdString());
  header->set_yygggg(ui_->dt->dateTime().toString("ddhhmm").toStdString());

  req.set_format(tlg::kWMO);
  req.set_isbinary(false);
  req.set_msgtype("alphanum");
  req.mutable_metainfo()->set_converted_dt(ui_->dt->dateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
  req.mutable_metainfo()->set_map_name(name.toStdString());

  req.set_msg(ui_->codeView->toPlainText().toStdString());

  meteo::msgcenter::CreateTelegramReply* reply;
  {
    meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(settings::proto::kMsgCenter);
    if (nullptr == ctrl){
      QApplication::restoreOverrideCursor();
      QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
                            QObject::tr("Ошибка сохранения в БД. Нет подключения к БД"));
      return false;
    }
    reply = ctrl->remoteCall(&msgcenter::MsgCenterService::CreateTelegram, req, 10000 );
  }
  if (nullptr == reply){
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
                          QObject::tr("Ошибка сохранения в БД. Нет ответа от ЦКС"));
    return false;
  }
  bool isOk = true;
  if (! reply->result() )   {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
                          QObject::tr("Ошибка сохранения в БД '%1'")
                          .arg(QString::fromStdString(reply->comment())));
    isOk = false;
  }
  delete reply;
  QApplication::restoreOverrideCursor();
  return isOk;
}

void Kn04::slotHelp(bool check)
{
  if (check == false) {
    if (0 != helpUi_) {
      helpUi_->hide();
    }
    return;
  }

  if (0 == helpUi_) {
    helpUi_ = new QDialog();
    helpUi_->resize(600, 400);
    QGridLayout* gridLayout = new QGridLayout(helpUi_);
    QTextEdit* te = new QTextEdit(helpUi_);
    te->setReadOnly(true);
    QByteArray ba;
    QFile file(HELP_FILE);
    if(file.open(QIODevice::ReadOnly)) {
      ba = file.readAll();
    }
    file.close();
    QString text(ba);
    te->setHtml(text);
    gridLayout->addWidget(te, 0, 1, 1, 1);
    connect(helpUi_, SIGNAL(finished(int)), ui_->help, SLOT(toggle()));
  }
  helpUi_->show();
}
