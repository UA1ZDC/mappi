#include "kn03.h"
#include "ui_kn03.h"
#include "alphanumgen.h"

#include <qdir.h>
#include <qfiledialog.h>

#include <meteo/commons/global/dateformat.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>

#include <qmessagebox.h>
#include <qevent.h>

#include <math.h>

#define TEMPLATE_AC_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn03ac.templ")
#define TEMPLATE_B_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn03b.templ")
#define TEMPLATE_D_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn03d.templ")
#define HELP_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn03.html")
#define CONF_FILE QDir::homePath() + "/.meteo/" + "kn03generate.conf"

using namespace meteo;

enum LevelColumn {
  kType  = 0,
  kLevel = 1,
  kHeight = 2,
  kdd    = 3,
  kff    = 4
};

enum LevelType {
  kDefault   = -1,
  kStation   =  0,
  kHStation  =  1,
  kStandart  =  2,
  kHStandart =  3,
  kMaxWind   =  4,
  kHMaxWind  =  5,
  kSign      =  6,
  kHSign     =  7
};

Kn03::Kn03(QWidget* parent, Qt::WindowFlags fl):
  QWidget(parent, fl),
  ui_(new Ui::Kn03),
  gen_(new AlphanumGen(TEMPLATE_AC_FILE))
{
  ui_->setupUi(this);
  ui_->dt->setDisplayFormat(meteo::dtHumanFormatTimeShort);
  QObject::connect(ui_->sendInMsg, SIGNAL(clicked()), SLOT(slotSendTlg()));
  QObject::connect(ui_->accept, SIGNAL(clicked()), SLOT(slotSaveTlg()));

  initWidgets();
  setConnects();
  setDefault();
  for (int i = 0, sz = ui_->decodeView->columnCount(); i < sz; ++i) {
    ui_->decodeView->resizeColumnToContents(i);
  }
}

Kn03::~Kn03()
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

void Kn03::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    close();
  }
}

void Kn03::initWidgets()
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
  
  slotAddLevelLine(kStandart);

  ui_->levels->resizeColumnsToContents();

  ui_->shearMode2->setEnabled(false);
  // for (int i = 0, sz = _ui->levels->columnCount(); i < sz; ++i) {
  //   _ui->levels->resizeColumnToContents(i);
  // }
}

void Kn03::setConnects()
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

  QObject::connect(ui_->shearMode, SIGNAL(toggled(bool)), SLOT(slotShearChanged(bool)));
  QObject::connect(ui_->vbvb, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->vava, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->shearMode2, SIGNAL(toggled(bool)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->vbvb2, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));
  QObject::connect(ui_->vava2, SIGNAL(valueChanged(int)), SLOT(slotLevelsChanged()));

  QObject::connect(ui_->a4, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));

  QObject::connect(ui_->levels, SIGNAL(cellChanged(int, int)), SLOT(slotLevelsChanged()));

  QObject::connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotAddLevelLine()));
  QObject::connect(ui_->removeBtn, SIGNAL(clicked()), SLOT(slotRmLevelLine()));
}

void Kn03::setDefault()
{
  ui_->dt->setDateTime(QDateTime::currentDateTimeUtc());
  ui_->IIiii->setText(QString::number(meteo::global::currentStationIndex()));
  ui_->ii->setText("0");
  ui_->CCCC->setText(meteo::global::wmoId());

  slotPartChanged(0);
}

//! Ячейка с целочисленными значениями
void Kn03::slotSetIntCell(int row, int col, int min, int max, bool readOnly /*= false*/, const int* value /* = 0*/)
{
  Q_UNUSED(readOnly);

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
void Kn03::slotSetDoubleCell(int row, int col, double min, double max, int decimals)
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
void Kn03::slotSetRegexpCell(int row, int col, const QRegExp& rx)
{
  QLineEdit *le = new QLineEdit(ui_->levels);
  le->setFrame(false);
  QRegExpValidator* leValid = new QRegExpValidator(rx);
  le->setValidator(leValid);
  connect(le, SIGNAL(textChanged(const QString&)), SLOT(slotLevelsChanged()));
  ui_->levels->setCellWidget(row, col, le);
}

void Kn03::slotAddLevelLine(int levType /* = -1*/, const int* level /*= 0*/, const int* height /*= 0*/)
{
  int row = ui_->levels->rowCount();
  ui_->levels->setRowCount(row + 1);

  QList<QPair<QString, LevelType> > type;
  int partIdx = ui_->part->currentIndex();

  if (levType == kStation) {
    type << QPair<QString, LevelType>(QObject::tr("Станция, гПа"), kStation);
    type << QPair<QString, LevelType>(QObject::tr("Станция, м"),  kHStation);
  } else if (levType == kStandart) {
    type << QPair<QString, LevelType>(QObject::tr("Стандартный, гПа"), kStandart);
    type << QPair<QString, LevelType>(QObject::tr("Стандартный, м"), kHStandart);
  } else if (partIdx == 0 || partIdx == 2) {
    type << QPair<QString, LevelType>(QObject::tr("Макс. ветер, гПа"), kMaxWind);
    type << QPair<QString, LevelType>(QObject::tr("Макс. ветер, м"), kHMaxWind);
  } else {
    type << QPair<QString, LevelType>(QObject::tr("Особая точка, гПа"), kSign);
    type << QPair<QString, LevelType>(QObject::tr("Особая точка, м"), kHSign);
  }

  QComboBox* wtype = new QComboBox(ui_->levels);
  wtype->setStyleSheet("QComboBox {\
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
      icon-size: 8px;\
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

  bool readOnly = false;
  if (levType == kHStation || levType == kHStandart || levType == kStandart ||
      levType == kHMaxWind || levType == kHSign) {
    readOnly = true;
  }

  if (ui_->part->currentIndex() == 0 || ui_->part->currentIndex() == 2) {
    slotSetIntCell(row, kLevel, 0, 1999, level);
  } else {
    slotSetDoubleCell(row, kLevel, 0, 1999, 1);
  }

  readOnly = false;
  if (levType == kStation || levType == kStandart || levType == kMaxWind ||
      levType == kSign) {
    readOnly = true;
  }
  slotSetIntCell(row, kHeight, 0, 99999, readOnly, height);
  slotSetRegexpCell(row, kdd, QRegExp("~|//|[0-2]?[\\d]{1,2}|(?!360)3[0-5][0-9]"));
  slotSetRegexpCell(row, kff, QRegExp("///|[1-3/]?[\\d]{1,2}"));

  slotLevelsChanged();
}

void Kn03::slotRmLevelLine()
{
  int row = ui_->levels->rowCount() - 1; //_ui->levels->currentRow();
  var(row);
  if (row < 0) return;

  QComboBox* cb = qobject_cast<QComboBox*>(ui_->levels->cellWidget(row, kType));
  var(cb);
  if (0 == cb) return;
  LevelType type = LevelType(cb->itemData(cb->currentIndex()).toInt());

  if (type == kMaxWind || type == kHMaxWind  || type == kHSign || type == kSign) {
    ui_->levels->clearSelection();
    ui_->levels->removeRow(row);
    slotLevelsChanged();
  }
}

void Kn03::slotGenerate()
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


void Kn03::slotSetValue(const QString& tval)
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

void Kn03::slotSetValue(int tval)
{
  if (0 == gen_ || 0 == QObject::sender()) {
    return;
  }

  gen_->setValue(QObject::sender()->objectName(), tval);
  
  slotGenerate();
}

void Kn03::slotSetValue(double tval)
{
  if (0 == gen_ || 0 == QObject::sender()) {
    return;
  }

  gen_->setValue(QObject::sender()->objectName(), tval);
  
  slotGenerate();
}

bool Kn03::checkParams(const QLineEdit& le, const QString& error)
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

void Kn03::slotDateTimeChanged(const QDateTime& dt)
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


void Kn03::slotPartChanged(int part)
{
  //TODO message, что данные не сохранены/отправлены
  if (0 == gen_) return;
  
  ui_->levels->clearContents();
  ui_->levels->setRowCount(0);

  switch (part) {
  case 0:
    gen_->readTemplate(TEMPLATE_AC_FILE);
    ui_->shearMode->setEnabled(true);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->T1T2->setText("UP");
    gen_->setCode("MMMM", "PPAA");
    break;
  case 1:
    gen_->readTemplate(TEMPLATE_B_FILE);
    ui_->shearMode->setEnabled(false);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->T1T2->setText("UG");
    gen_->setCode("MMMM", "PPBB");
    break;
  case 2:
    gen_->readTemplate(TEMPLATE_AC_FILE);
    ui_->shearMode->setEnabled(true);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->T1T2->setText("UH");
    gen_->setCode("MMMM", "PPCC");
    break;
  case 3:
    gen_->readTemplate(TEMPLATE_D_FILE);
    ui_->shearMode->setEnabled(false);
    ui_->shearMode->setChecked(false);
    ui_->shearMode2->setEnabled(false);
    ui_->shearMode2->setChecked(false);
    ui_->T1T2->setText("UQ");
    gen_->setCode("MMMM", "PPDD");
    break;
  }

  slotDateTimeChanged(ui_->dt->dateTime());
  // _gen->setValue("CCCC", meteo::global::wmoId());
  gen_->setValue("IIiii", ui_->IIiii->text());
  // _gen->setValue("ii", _ui->ii->text());
  gen_->setValue("a4", 0);

  fillPartDefault(part);

  slotGenerate();
}

void Kn03::fillPartDefault(int part)
{
  if (part == 1) {
    slotAddLevelLine(kStation);
  }
  
  if (part == 1 || part == 3) {
    slotAddLevelLine(kSign);
  }

  QList<int> levels;
  QList<int> height;
  if (part == 0) {
    levels << 850 << 700 << 500 << 400 << 300 << 250 << 200 << 150 << 100;
    height << 1500 << 3000 << 5500 << 7000 << 9000 << 10500 << 12000 << 13500 << 16000;
  } else if (part == 2) {
    levels << 70 << 50 << 30 << 20 << 10;
    height << 18500 << 20500 << 23500 << 26500 << 31000;
  }

  int lidx = ui_->levels->rowCount() - 1;
  for (int idx = 0; idx < levels.count(); idx++) {
    slotAddLevelLine(kStandart, &levels.at(idx), &height.at(idx));
    ++lidx;

    // QTableWidgetItem* item = new QTableWidgetItem();
    // if (0 == item) return;
    // item->setText(QString::number(levels.at(idx)));
    // _ui->levels->setItem(lidx, kLevel, item);
  }
  
}

void Kn03::slotShearChanged(bool toggl)
{
  if (toggl && ui_->part->currentIndex() == 0) {
    ui_->shearMode2->setEnabled(true);
  } else {
    ui_->shearMode2->setEnabled(false);
  }

  slotLevelsChanged();
}

void Kn03::slotSetLevelValue(const QString& name, int row, int col)
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

void Kn03::slotSetWind(const QString& ddname, const QString& ffname, int row)
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

void Kn03::setStandartLevel(const QList<int>& mark, const QList<QString>& level, 
                            const QList<QString>& dd, const QList<QString>& ff)
{
  int idx = 0;
  int lastIdx = -1;
  for (int k = level.size() - 1; k >= 0; k--) {
    if (!dd.at(k).isEmpty() || !ff.at(k).isEmpty()) {
      lastIdx = k;
      break;
    }
  }

  if (lastIdx == -1) return;
  int size = lastIdx + 1;

  for (int k = 0; k < size; k++) {
    if (!dd.at(k).isEmpty() || !ff.at(k).isEmpty()) {
      idx = k;
      break;
    }
  }

  while (idx < size) {
    int n = 3;
    if (idx + n >= size) {
      n = size - idx;
    }
    for (int k = 1; k < n; k++) { //уровень смены давления на высоту
      if (mark.at(idx) != mark.at(idx + k)) {
        n = k;
        break;
      }
    }
    if (n == 1 && dd.at(idx).isEmpty() && ff.at(idx).isEmpty()) {
      idx++;
      continue;
    }

    int num = gen_->addGroupSetValue("standart", "Pmark", mark.at(idx));
    gen_->setGroupSetValue("standart", num, "n", n);
    gen_->setGroupSetValue("standart", num, "P1P1", level.at(idx).toInt());
    for (int k = 0; k < n; k++) {
      QString ddval, ffval;
      anc::func_ddfff(dd.at(idx), ff.at(idx), &ddval, &ffval);
      if (ddval.isEmpty()) {
        gen_->setGroupSetRequire("standart", num, QString("dd_") + QString::number(k), true);
        gen_->setGroupSetValue("standart", num, QString("dd_") + QString::number(k), "//");
      } else {
        gen_->setGroupSetValue("standart", num, QString("dd_") + QString::number(k), ddval);
      }
      if (ffval.isEmpty()) {
        gen_->setGroupSetRequire("standart", num, QString("ff_") + QString::number(k), true);
        gen_->setGroupSetValue("standart", num, QString("ff_") + QString::number(k), "///");
      } else {
        gen_->setGroupSetValue("standart", num, QString("ff_") + QString::number(k), ffval);
      }
      idx++;
    }
  }
}

void Kn03::setSignLevel(const QList<int>& height, const QList<QString>& dd, const QList<QString>& ff, bool signStation)
{
  QList<int> mark;
  int size = height.size();
  for (int idx = 0; idx < height.size(); idx++) {
    if (height.at(idx) % 500 == 0) {
      mark << 8;
    } else if (height.at(idx) % 300 == 0) {
      mark << 9;
    } else {
      size = idx;
      break;
    }
  }

  if (mark.size() == 0) return;

  //  if (mark == 0) {
  // QMessageBox::information(this, QObject::tr("Не корректное значение высоты"),
  // 			     QObject::tr("Значения высот должны быть кратны 300 или 500"),
  // 			     QMessageBox::Ok);
  //return;
  // }

  int idx = 0;
  while (idx < size) {
    int n = 3;
    if (idx + n >= size) {
      n = size - idx;
    }
    for (int k = 0; k < n; k++) {
      if (mark.at(idx) != mark.at(idx + k)) {
        n = k;
        break;
      }
    }
    if (n == 1 && dd.at(idx).isEmpty() && ff.at(idx).isEmpty()) {
      idx++;
      continue;
    }

    int num;
    if (mark.at(idx) == 9 && height.at(idx) > 29700) {
      num = gen_->addGroupSetValue("wind", "uMark", 1);
    } else {
      num = gen_->addGroupSetValue("wind", "uMark", mark.at(idx));
    }

    QString u;
    for (int k = 0; k < n; k++) {
      //debug_log << size << idx << k << n;
      int tn = height.at(idx);
      if (mark.at(idx) == 8) {
        tn /= 500;
      } else if (mark.at(idx) == 9) {
        tn /= 300;
      } else {
        tn = (tn - 30000) / 300;
      }
      if (k == 0) {
        gen_->setGroupSetValue("wind", num, "tn", tn / 10);
      }

      if (idx == 0 && k == 0 && signStation) {
        u += "/";
      } else {
        u += QString::number(tn % 10);
      }

      QString ddval, ffval;
      anc::func_ddfff(dd.at(idx), ff.at(idx), &ddval, &ffval);
      if (ddval.isEmpty()) {
        gen_->setGroupSetRequire("wind", num, QString("dd_") + QString::number(k), true);
        gen_->setGroupSetValue("wind", num, QString("dd_") + QString::number(k), "//");
      } else {
        gen_->setGroupSetValue("wind", num, QString("dd_") + QString::number(k), ddval);
      }
      if (ffval.isEmpty()) {
        gen_->setGroupSetRequire("wind", num, QString("ff_") + QString::number(k), true);
        gen_->setGroupSetValue("wind", num, QString("ff_") + QString::number(k), "///");
      } else {
        gen_->setGroupSetValue("wind", num, QString("ff_") + QString::number(k), ffval);
      }
      idx++;
    }
    gen_->setGroupSetValue("wind", num, "u1u2u3", u.leftJustified(3, '/'));
  }

}

void Kn03::slotLevelsChanged()
{
  if (gen_ == 0) return;

  gen_->clearGroupSet("standart");
  gen_->clearGroupSet("wind");

  if (ui_->part->currentIndex() == 1 || ui_->part->currentIndex() == 3) {
    gen_->clearValue("P0P0P0");
    gen_->clearValue("d0d0");
    gen_->clearValue("f0f0f0");
    gen_->clearGroupSet("signWind");
  }

  // _gen->clearValue("");
  int nnwind = 1;
  int maxWindCnt = 0;

  QList<QString> level, dd, ff;//для стандартных
  QList<int> mark;
  QList<QString> sdd, sff;//для особых
  QList<int> sheight;
  bool signStation = false;

  for (int row = 0; row < ui_->levels->rowCount(); row++) {
    QComboBox* cb = qobject_cast<QComboBox*>(ui_->levels->cellWidget(row, kType));
    if (0 == cb) { continue; }
    LevelType type = LevelType(cb->itemData(cb->currentIndex()).toInt());
    switch (type) {
    case kStation:
      slotSetLevelValue("P0P0P0", row, kLevel);
      slotSetWind("d0d0", "f0f0f0", row);
      break;
    case kStandart:
    case kHStandart: {
      QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
      if (0 == leLevel || leLevel->text().isEmpty() ||  0 == ledd || 0 == leff) {
        break;
      }
      if (type == kStandart) {
        mark << 44;
      } else {
        mark << 55;
      }
      level << leLevel->text();
      dd << ledd->text();
      ff << leff->text();
    }
      break;
    case kMaxWind:
    case kHMaxWind: {
      QLineEdit* leLevel  = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
      QLineEdit* leHeight = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kHeight));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
      if ( 0 == leLevel || 0 == leHeight || (type == kMaxWind && leLevel->text().isEmpty() )
           || (type == kHMaxWind && leHeight->text().isEmpty())
           || 0 == ledd || 0 == leff) {
        break;
      }
      
      int idx;
      QString set;
      if (type == kMaxWind) {
        set = "wind";
        idx = gen_->addGroupSetValue(set, "PmPmPm", leLevel->text().toInt());
        gen_->setGroupSetValue(set, idx, "PmMark", 77);
      } else {
        set = "wind";
        idx = gen_->addGroupSetValue(set, "HmHmHmHm", leHeight->text().toInt() / 10);
        gen_->setGroupSetValue(set, idx, "HmMark", 7);
      }
      QString dd, ff;
      anc::func_ddfff(ledd->text(), leff->text(), &dd, &ff);
      if (!dd.isEmpty()) {gen_->setGroupSetValue(set, idx, "dmdm", dd); }
      if (!ff.isEmpty()) {gen_->setGroupSetValue(set, idx, "fmfmfm", ff); }
      if (ui_->shearMode->isEnabled() && ui_->shearMode->isChecked() && maxWindCnt == 0) {
        gen_->setGroupSetValue(set, idx, "vava", ui_->vava->value());
        gen_->setGroupSetValue(set, idx, "vbvb", ui_->vbvb->value());
      }
      if (ui_->shearMode2->isEnabled() && ui_->shearMode2->isChecked() && maxWindCnt == 1) {
        gen_->setGroupSetValue(set, idx, "vava", ui_->vava2->value());
        gen_->setGroupSetValue(set, idx, "vbvb", ui_->vbvb2->value());
      }
      maxWindCnt++;
    }
      break;
    case kHStation:
    case kHSign: {
      QLineEdit* leHeight = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kHeight));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));

      if (0 == leHeight || leHeight->text().isEmpty() || 0 == ledd || 0 == leff) {
        break;
      }

      if (type == kHStation) {
        signStation = true;
      }
      
      sheight << leHeight->text().toInt();
      sdd << ledd->text();
      sff << leff->text();
    }
      break;
    case kSign: {
      QLineEdit* leLevel = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kLevel));
      QLineEdit* ledd = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kdd));
      QLineEdit* leff = qobject_cast<QLineEdit*>(ui_->levels->cellWidget(row, kff));
      
      if (0 == leLevel || leLevel->text().isEmpty() ||
          0 == ledd || 0 == leff) break;

      int idx = gen_->addGroupSetValue("signWind", "P1P1P1", leLevel->text().toFloat());
      gen_->setGroupSetValue("signWind", idx, "n1n1", QString::number(nnwind) + QString::number(nnwind));
      QString dd, ff;
      anc::func_ddfff(ledd->text(), leff->text(), &dd, &ff);
      if (!dd.isEmpty()) {
        gen_->setGroupSetValue("signWind", idx, "d1d1", dd);
      }
      if (!ff.isEmpty()) {
        gen_->setGroupSetValue("signWind", idx, "f1f1f1", ff);
      }
      ++nnwind;
    }
      break;
      
    default: {}
    }
    
  }

  if (!level.isEmpty()) {
    setStandartLevel(mark, level, dd, ff);
  }
  
  if (!sheight.isEmpty()) {
    setSignLevel(sheight, sdd, sff, signStation);
  }

  slotGenerate();
}

bool Kn03::checkCorrectInput()
{
  if (!checkParams(*ui_->IIiii, QObject::tr("индекса станции"))) return false;
  if (!checkParams(*ui_->T1T2, QObject::tr("T1T2"))) return false;
  if (!checkParams(*ui_->A1A2, QObject::tr("A1A2"))) return false;
  if (!checkParams(*ui_->ii, QObject::tr("ii"))) return false;
  if (!checkParams(*ui_->CCCC, QObject::tr("CCCC"))) return false;
  if (!checkParams(*ui_->BBB, QObject::tr("BBB"))) return false;
  return true;
}

void Kn03::slotSaveTlg()
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

void Kn03::slotSendTlg()
{
  if (!checkCorrectInput()) return;
  QString defaultName =  ui_->T1T2->text() + ui_->A1A2->text() +
      ui_->ii->text().rightJustified(2, '0') + ui_->dt->dateTime().toString("ddhhmm") + ".tlg";
  saveInnerDoc(defaultName);
}

bool Kn03::saveInnerDoc(const QString& name)
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
    if (nullptr == ctrl) {
      QApplication::restoreOverrideCursor();
      QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
                            QObject::tr("Ошибка сохранения в БД. Нет подключения к БД"));
      return false;
    }
    reply = ctrl->remoteCall(&msgcenter::MsgCenterService::CreateTelegram, req, 10000 );
    delete ctrl;
  }
  if (nullptr == reply){
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
                          QObject::tr("Ошибка сохранения в БД. Нет ответа от ЦКС"));
    return false;
  }
  bool isOk = true;
  if (! reply->result() )   {
    isOk = false;
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
                          QObject::tr("Ошибка сохранения в БД '%1'")
                          .arg(QString::fromStdString(reply->comment())));
  }
  delete reply;
  QApplication::restoreOverrideCursor();
  return isOk;
}

void Kn03::slotHelp(bool check)
{
  if (check == false) {
    if (0 != helpUi_) {
      helpUi_->hide();
    }
    return;
  }

  if (0 == helpUi_) {
    helpUi_ = new QDialog(this);
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
