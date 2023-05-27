#include "kn01.h"
#include "ui_kn01.h"
#include "ui_selectweather.h"
#include "alphanumgen.h"

#include <qdir.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qfiledialog.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <cross-commons/debug/tmap.h>
#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>

#define TEMPLATE_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn01.templ")
#define HELP_FILE  QString(MnCommon::projectPath() + "/share/meteo/anc-templates/kn01.html")
#define CONF_FILE QDir::homePath() + "/.meteo/" + "kn01generate.conf"

using namespace meteo;

enum CloudMode {
  kSkyClear = 0,
  kSkyLessClouds = 1,
  kSkyInvisible = 2,
  kSkyInvisOther = 3,
  kSkyNoMeas = 4,
  kSkyCustom = 5
};

//верхняя граница в градусах и соответствующий румб
static const TMap<float, QString> ddConvert = TMap<float, QString>()
<< QPair<float, QString>(  11.25, QObject::tr("  (С)"))
<< QPair<float, QString>(  33.75, QObject::tr("  (ССВ)"))
<< QPair<float, QString>(  56.25, QObject::tr("  (СВ)"))
<< QPair<float, QString>(  78.75, QObject::tr("  (ВСВ)"))
<< QPair<float, QString>( 101.25, QObject::tr("  (В)"))
<< QPair<float, QString>( 123.75, QObject::tr("  (ВЮВ)"))
<< QPair<float, QString>( 146.25, QObject::tr("  (ЮВ)"))
<< QPair<float, QString>( 168.75, QObject::tr("  (ЮЮВ)"))
<< QPair<float, QString>( 191.25, QObject::tr("  (Ю)"))
<< QPair<float, QString>( 213.75, QObject::tr("  (ЮЮЗ)"))
<< QPair<float, QString>( 236.25, QObject::tr("  (ЮЗ)"))
<< QPair<float, QString>( 258.75, QObject::tr("  (ЗЮЗ)"))
<< QPair<float, QString>( 281.25, QObject::tr("  (З)"))
<< QPair<float, QString>( 303.75, QObject::tr("  (ЗСЗ)"))
<< QPair<float, QString>( 326.25, QObject::tr("  (СЗ)"))
<< QPair<float, QString>( 348.75, QObject::tr("  (ССЗ)"))
<< QPair<float, QString>( 359.99, QObject::tr("  (С)"));

Kn01::Kn01(QWidget* parent, Qt::WindowFlags fl):
  QWidget(parent, fl),
  ui_(new Ui::Kn01),
  gen_(new AlphanumGen(TEMPLATE_FILE))
{
  ui_->setupUi(this);
  ui_->dt->setDisplayFormat(meteo::dtHumanFormatTimeShort);
  QObject::connect(ui_->accept, SIGNAL(clicked()), SLOT(slotSaveTlg()));
  QObject::connect(ui_->sendInMsg, SIGNAL(clicked()), SLOT(slotSendTlg()));
  initWidgets();
  setConnects();
  setDefault();
  for (int i = 0, sz = ui_->decodeView->columnCount(); i < sz; ++i) {
    ui_->decodeView->resizeColumnToContents(i);
  }
}

Kn01::~Kn01()
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

void Kn01::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    close();
  }
}

//! Заполнение виджетов
void Kn01::initWidgets()
{
  ui_->decodeView->hideColumn(1);

  QIntValidator* v = new QIntValidator(ui_->IIiii);
  v->setRange(1, 99999);
  ui_->IIiii->setValidator(v);

  QRegExpValidator* va1a2 = new QRegExpValidator(QRegExp("[A-Za-z][A-Za-z]"));
  ui_->A1A2->setValidator(va1a2);

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

  for (int idx = 0; idx < ui_->E->count() - 1; idx ++) {
    ui_->E->setItemData(idx, QString::number(idx));
  }
  ui_->E->setItemData(ui_->E->count() - 1, "/");
  ui_->E->setCurrentIndex(10);

  for (int idx = 0; idx < ui_->E->count(); idx ++) {
    ui_->E5->insertItem(idx, ui_->E->itemText(idx), ui_->E->itemData(idx, Qt::UserRole));
  }
  ui_->E5->setCurrentIndex(10);

  for (int idx = 0; idx < ui_->E->count(); idx += 2) {
    ui_->E->setItemData(idx, QColor(235, 235, 235), Qt::BackgroundRole);
    ui_->E5->setItemData(idx, QColor(235, 235, 235), Qt::BackgroundRole);
  }

  for (int idx = 0; idx < ui_->E_->count() - 1; idx ++) {
    ui_->E_->setItemData(idx, QString::number(idx));
  }
  ui_->E_->setItemData(ui_->E_->count() - 1, "/");

  for (int idx = 0; idx < ui_->E_->count(); idx += 2) {
    ui_->E_->setItemData(idx, QColor(235, 235, 235), Qt::BackgroundRole);
  }
  ui_->E_->setCurrentIndex(10);

  ui_->a3->insertItem(0, QObject::tr("1000 гПа"), "1");
  ui_->a3->insertItem(1, QObject::tr("925 гПа"),  "2");
  ui_->a3->insertItem(2, QObject::tr("500 гПа"),  "5");
  ui_->a3->insertItem(3, QObject::tr("700 гПа"),  "7");
  ui_->a3->insertItem(4, QObject::tr("850 гПа"),  "8");
  
  QStringList form;
  form << "Ci" << "Cc" << "Cs" << "Ac" << "As" << "Ns"
       << "Sc" << "St" << "Cu" << "Cb" << "/";
  for (int idx = 0; idx < ui_->clouds->rowCount(); idx++) {
    QComboBox* cb = new QComboBox;
    for (int fidx = 0; fidx < form.size(); fidx++) {
      if (form.at(fidx) != "/") {
        cb->insertItem(fidx, form.at(fidx), QString::number(fidx));
      } else {
        cb->insertItem(fidx, form.at(fidx), form.at(fidx));
      }
    }

    connect(cb, SIGNAL(currentIndexChanged(int)), SLOT(slotCloudsLayChanged()));
    
    QSpinBox* sbN = new QSpinBox;
    sbN->setMaximum(10);
    connect(sbN, SIGNAL(valueChanged(int)), SLOT(slotCloudsLayChanged()));
    QSpinBox* sbh = new QSpinBox;
    sbh->setMaximum(99999);
    connect(sbh, SIGNAL(valueChanged(int)), SLOT(slotCloudsLayChanged()));
    QCheckBox* sbht = new QCheckBox;
    sbht->setCheckState(Qt::Checked);
    connect(sbht, SIGNAL(stateChanged(int)), SLOT(slotCloudsLayChanged()));

    ui_->clouds->setCellWidget(idx, 0, sbN);
    ui_->clouds->setCellWidget(idx, 1, cb);
    ui_->clouds->setCellWidget(idx, 2, sbh);
    ui_->clouds->setCellWidget(idx, 3, sbht);
  }
  //  _ui->clouds->horizontalHeaderItem(0)->setText("Количество,\nбалл");
  ui_->clouds->resizeColumnsToContents();

  for (int idx = 0; idx < ui_->SpNm->count(); idx += 2) {
    ui_->SpNm->setItemData(idx, QColor(235, 235, 235), Qt::BackgroundRole);
  }

  for (int idx = 0; idx < ui_->W1->count() - 1; idx ++) {
    ui_->W1->setItemData(idx, QString::number(idx));
  }
  ui_->W1->setItemData(ui_->CL->count() - 1, "/");
  for (int idx = 0; idx < ui_->W2->count() - 1; idx ++) {
    ui_->W2->setItemData(idx, QString::number(idx));
  }
  ui_->W2->setItemData(ui_->CL->count() - 1, "/");
}


void Kn01::setConnects()
{
  QObject::connect(ui_->help, SIGNAL(toggled(bool)), SLOT(slotHelp(bool)));
  QObject::connect(ui_->cancel, SIGNAL(clicked()), SLOT(close()));

  QObject::connect(ui_->IIiii, SIGNAL(textChanged(const QString&)), SLOT(slotSetValue(const QString&)));
  QObject::connect(ui_->dt, SIGNAL(dateTimeChanged(const QDateTime&)), SLOT(slotDateTimeChanged(const QDateTime&)));

  // QObject::connect(_ui->T1T2, SIGNAL(currentIndexChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->A1A2, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->ii, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->CCCC, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));
  // QObject::connect(_ui->BBB, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)));

  QObject::connect(ui_->cloudMode, SIGNAL(currentIndexChanged(int)), SLOT(slotCloudsChanged()));
  QObject::connect(ui_->N, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->Nh, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->h, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->CL, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->CM, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->CH, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));

  QObject::connect(ui_->ix, SIGNAL(currentIndexChanged(int)), SLOT(slotWeatherChanged()));
  QObject::connect(ui_->ww, SIGNAL(valueChanged(const QString&)), SLOT(slotSetValue(const QString&)));
  QObject::connect(ui_->W1, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->W2, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->wwCode, SIGNAL(clicked()), SLOT(slotSetww()));

  QObject::connect(ui_->VV, SIGNAL(valueChanged(double)), SLOT(slotVVChanged()));
  QObject::connect(ui_->VVmode, SIGNAL(currentIndexChanged(int)), SLOT(slotVVChanged()));

  QObject::connect(ui_->SSSmode, SIGNAL(toggled(bool)), SLOT(slotSSSChanged()));
  QObject::connect(ui_->SSS, SIGNAL(valueChanged(double)), SLOT(slotSSSChanged()));

  QObject::connect(ui_->iw, SIGNAL(stateChanged(int)), SLOT(slotWindChanged()));
  QObject::connect(ui_->windMode, SIGNAL(currentIndexChanged(int)), SLOT(slotWindChanged()));
  QObject::connect(ui_->ff, SIGNAL(valueChanged(int)), SLOT(slotWindChanged()));
  QObject::connect(ui_->dd, SIGNAL(valueChanged(int)), SLOT(slotWindChanged()));
  QObject::connect(ui_->dd, SIGNAL(valueChanged(int)), SLOT(slotSetddSuffix(int)));

  QObject::connect(ui_->iR, SIGNAL(currentIndexChanged(int)), SLOT(slotPrecpChanged()));
  QObject::connect(ui_->tR, SIGNAL(currentIndexChanged(int)), SLOT(slotPrecpChanged()));
  QObject::connect(ui_->RRR, SIGNAL(valueChanged(double)), SLOT(slotPrecpChanged()));

  QObject::connect(ui_->is_PPPP, SIGNAL(toggled(bool)), SLOT(slotIsPPPPChanged(bool)));
  QObject::connect(ui_->a3, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->hhh, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->PPPP, SIGNAL(valueChanged(double)), SLOT(slotSetValue(double)));
  QObject::connect(ui_->P0P0P0P0, SIGNAL(valueChanged(double)), SLOT(slotSetValue(double)));
  QObject::connect(ui_->a, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->ppp, SIGNAL(valueChanged(double)), SLOT(slotSetValue(double)));

  QObject::connect(ui_->snTTT, SIGNAL(valueChanged(double)), SLOT(slotSetValue(double)));
  QObject::connect(ui_->snTdTdTd, SIGNAL(valueChanged(double)), SLOT(slotSetValue(double)));
  QObject::connect(ui_->is_snTxTxTx, SIGNAL(stateChanged(int)), SLOT(slotIsTxChanged(int)));
  QObject::connect(ui_->is_snTnTnTn, SIGNAL(stateChanged(int)), SLOT(slotIsTnChanged(int)));
  QObject::connect(ui_->snTxTxTx, SIGNAL(valueChanged(double)), SLOT(slotSetValue(double)));
  QObject::connect(ui_->snTnTnTn, SIGNAL(valueChanged(double)), SLOT(slotSetValue(double)));
  
  QObject::connect(ui_->is_snTgTg, SIGNAL(stateChanged(int)), SLOT(slotIsTgChanged(int)));
  QObject::connect(ui_->snTgTg, SIGNAL(valueChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->E, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->E_, SIGNAL(currentIndexChanged(int)), SLOT(slotSetValue(int)));
  QObject::connect(ui_->sss, SIGNAL(valueChanged(int)), SLOT(slotSssChanged()));
  QObject::connect(ui_->sssMode, SIGNAL(currentIndexChanged(int)), SLOT(slotSssChanged()));

  QObject::connect(ui_->clouds, SIGNAL(cellChanged(int, int)), SLOT(slotCloudsLayChanged()));

  //QObject::connect(_ui->E, SIGNAL(currentIndexChanged(int)), _ui->E5, SLOT(setCurrentIndex(int)));
  
  QObject::connect(ui_->snTg_Tg_, SIGNAL(valueChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->E5,       SIGNAL(currentIndexChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->snT24T24T24, SIGNAL(valueChanged(double)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->snT2T2,   SIGNAL(valueChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->f12f12,   SIGNAL(valueChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->R24R24R24, SIGNAL(valueChanged(double)), SLOT(slotNationalChanged()));

  QObject::connect(ui_->is_snTg_Tg_, SIGNAL(stateChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->is_snT24T24T24, SIGNAL(stateChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->is_snT2T2,   SIGNAL(stateChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->is_f12f12,   SIGNAL(stateChanged(int)), SLOT(slotNationalChanged()));
  QObject::connect(ui_->is_R24R24R24, SIGNAL(stateChanged(int)), SLOT(slotNationalChanged()));


  QObject::connect(ui_->is_SpTw,    SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Spff,    SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Spgust,  SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Spfog,   SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Sphail,  SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Sphaze,  SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Spprecp, SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Spss,    SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Spsstorm,  SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_Sptornado, SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_SpNm,    SIGNAL(toggled(bool)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_SpNv,    SIGNAL(toggled(bool)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_SpNm_Eh, SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_SpNm_vp, SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_SpNv_Eh, SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->is_SpNv_vp, SIGNAL(stateChanged(int)), SLOT(slotAddChanged()));

  QObject::connect(ui_->Sp_tornado, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_Mw, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_sq, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_ff_hour, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_sstorm, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_precp, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_fog, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_haze_hour, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->SpNm, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_n3, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_Nv, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_n4, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));

  QObject::connect(ui_->Sp_Da, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_Dp, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_ff, SIGNAL(valueChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_S8_, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_hailRR, SIGNAL(valueChanged(double)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_hailRRmode, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_RR, SIGNAL(valueChanged(double)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_RRmode, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_Tw, SIGNAL(valueChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_ss, SIGNAL(valueChanged(double)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_ssmode, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_NmEh, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_NmDa, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_Nmvp, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_NmDp, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_NvEh, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_NvDa, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_Nvvp, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
  QObject::connect(ui_->Sp_NvDp, SIGNAL(currentIndexChanged(int)), SLOT(slotAddChanged()));
}

//! Параметры по умолчанию
void Kn01::setDefault()
{
  ui_->dt->setDateTime(QDateTime::currentDateTimeUtc());
  ui_->IIiii->setText(QString::number(meteo::global::currentStationIndex()));
  ui_->ii->setText("0");
  ui_->T1T2->setCurrentIndex(1);
  ui_->CCCC->setText(meteo::global::wmoId());

  slotCloudsChanged();

  slotWeatherChanged();

  slotVVChanged();
  slotSSSChanged();
  slotWindChanged();
  slotPrecpChanged();

  //---

  ui_->PPPP->setValue(1000);
  ui_->P0P0P0P0->setValue(1000);
  ui_->a->setCurrentIndex(4);
  ui_->ppp->setValue(1);

  ui_->snTTT->setValue(5);
  ui_->snTdTdTd->setValue(5);
  // _ui->snTxTxTx->setValue(5);
  // _ui->snTnTnTn->setValue(5);

  // _ui->snTgTg->setValue(5);
  slotSssChanged();
}

//! Значение ветра в румбах
void Kn01::slotSetddSuffix(int value)
{
  QList<float> keys = ddConvert.keys();

  for (int idx = 0; idx < keys.count(); idx++) {
    if (value <= keys.at(idx)) {
      ui_->dd->setSuffix(ddConvert.value(keys.at(idx)));
      break;
    }
  }
}

void Kn01::slotSetww()
{
  SelectWeather* dlg = new SelectWeather;
  if ( QDialog::Accepted == dlg->exec() ) {
    int w = dlg->weather();
    if (w != -1) {
      ui_->ww->setValue(w);
    }
  }

  delete dlg;
}

//! Формирование телеграммы	   
void Kn01::slotGenerate()
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

//! Установка значение
void Kn01::slotSetValue(const QString& tval)
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

//! Установка значение
void Kn01::slotSetValue(int tval)
{
  if (0 == gen_ || 0 == QObject::sender()) {
    return;
  }

  QComboBox* cb = qobject_cast<QComboBox*>(QObject::sender());
  if (0 != cb && QVariant::Invalid != cb->itemData(cb->currentIndex(), Qt::UserRole)) {
    gen_->setCode(cb->objectName(), cb->itemData(cb->currentIndex(), Qt::UserRole).toString());
  } else {
    gen_->setValue(QObject::sender()->objectName(), tval);
  }
  
  slotGenerate();
}

//! Установка значение
void Kn01::slotSetValue(double tval)
{
  if (0 == gen_ || 0 == QObject::sender()) {
    return;
  }

  gen_->setValue(QObject::sender()->objectName(), tval);
  
  slotGenerate();
}

//-----

//! Сокращенный заголовок
bool Kn01::checkParams(const QLineEdit& le, const QString& error)
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

//! Облачность
void Kn01::slotCloudsChanged()
{
  if (0 == gen_) {
    return;
  }

  if (ui_->cloudMode->currentIndex() == kSkyCustom) {
    ui_->N->setEnabled(true);
    ui_->Nh->setEnabled(true);
    ui_->h->setEnabled(true);
    ui_->CL->setEnabled(true);
    ui_->CM->setEnabled(true);
    ui_->CH->setEnabled(true);
    gen_->setValue("h", ui_->h->value());
    gen_->setValue("N", ui_->N->value());
    gen_->setValue("Nh", ui_->Nh->value());
    gen_->setCode("CL",  ui_->CL->itemData(ui_->CL->currentIndex(), Qt::UserRole).toString());
    gen_->setCode("CM",  ui_->CM->itemData(ui_->CM->currentIndex(), Qt::UserRole).toString());
    gen_->setCode("CH",  ui_->CH->itemData(ui_->CH->currentIndex(), Qt::UserRole).toString());
    ui_->clouds->setEnabled(true);
    slotCloudsLayChanged();
    slotGenerate();
    return;
  }

  ui_->N->setEnabled(false);
  ui_->Nh->setEnabled(false);
  ui_->h->setEnabled(false);
  ui_->CL->setEnabled(false);
  ui_->CM->setEnabled(false);
  ui_->CH->setEnabled(false);

  switch (ui_->cloudMode->currentIndex()) {
  case kSkyClear:
    gen_->setCode("h", "9");
    gen_->setCode("N", "0");
    gen_->setCode("Nh", "/");
    break;
  case kSkyLessClouds:
    ui_->h->setEnabled(true);
    gen_->setValue("h", ui_->h->value());
    gen_->setCode("N", "1");
    gen_->setCode("Nh", "/");
    break;
  case kSkyInvisible:
    gen_->setCode("h", "/");
    gen_->setCode("N", "9");
    gen_->setCode("Nh", "/");
    break;
  case kSkyInvisOther: case kSkyNoMeas:
    gen_->clearValue("h");
    gen_->clearValue("N");
    gen_->clearValue("Nh");
    break;
  }

  gen_->clearValue("CL");
  gen_->clearValue("CM");
  gen_->clearValue("CH");

  ui_->clouds->setEnabled(false);
  slotCloudsLayChanged();

  slotGenerate();
}

//! Отдельные облака
void Kn01::slotCloudsLayChanged()
{
  if (0 == gen_) {
    return;
  }

  gen_->clearGroupList("clouds");

  if (!ui_->clouds->isEnabled()) {
    slotGenerate();
    return;
  }

  for (int row = 0; row < ui_->clouds->rowCount(); row++) {
    QSpinBox* sb = qobject_cast<QSpinBox*>(ui_->clouds->cellWidget(row, 0));
    if (0 != sb && 0 != sb->value()) {
      int idx = gen_->addListValue("clouds", "Ns", sb->value());
      QComboBox* cb = qobject_cast<QComboBox*>(ui_->clouds->cellWidget(row, 1));
      if (0 != cb) {
        gen_->setListValue("clouds", idx, "C", cb->itemData(cb->currentIndex()).toString());
      }
      sb = qobject_cast<QSpinBox*>(ui_->clouds->cellWidget(row, 2));
      QCheckBox* sbht = qobject_cast<QCheckBox*>(ui_->clouds->cellWidget(row, 3));
      if (0 != sb && 0 != sbht) {
        if (sbht->checkState() == Qt::Checked) {
          gen_->setListValue("clouds", idx, "hshs", anc::func_hshsInstr(sb->value()));
        } else {
          gen_->setListValue("clouds", idx, "hshs", anc::func_hshsVis(sb->value()));
        }
      }

      idx++;
    }
  }

  slotGenerate();
}

//! Дата и срок наблюдения
void Kn01::slotDateTimeChanged(const QDateTime& dt)
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

//! Ветер
void Kn01::slotWindChanged()
{
  if (0 == gen_) {
    return;
  }

  if (ui_->iw->checkState() == Qt::Checked) {
    gen_->setCode(ui_->iw->objectName(), "1");
  } else {
    gen_->setCode(ui_->iw->objectName(), "0");
  }

  switch (ui_->windMode->currentIndex()) {
  case 0:
    ui_->iw->setEnabled(true);
    ui_->dd->setEnabled(true);
    ui_->ff->setEnabled(true);
    gen_->setValue("dd", ui_->dd->value());
    gen_->setValue("ff", ui_->ff->value());
    break;
  case 1: //штиль
    ui_->iw->setEnabled(true);
    ui_->dd->setEnabled(false);
    ui_->ff->setEnabled(false);
    gen_->setCode("dd", "0");
    gen_->setCode("ff", "0");
    break;
  case 2: //переменный
    ui_->iw->setEnabled(true);
    ui_->dd->setEnabled(false);
    ui_->ff->setEnabled(true);
    gen_->setCode("dd", "99");
    gen_->setValue("ff", ui_->ff->value());
    break;
  case 3: //нет данных
    ui_->iw->setEnabled(false);
    ui_->dd->setEnabled(false);
    ui_->ff->setEnabled(false);
    gen_->clearValue("iw");
    gen_->clearValue("ff");
    gen_->clearValue("dd");
    break;
  }

  slotGenerate();
}

//! Видимость
void Kn01::slotVVChanged()
{
  if (0 == gen_) {
    return;
  }

  switch (ui_->VVmode->currentIndex()) {
  case 0:
    gen_->setCode("VV", anc::func_VVinstr(ui_->VV->value()));
    ui_->VV->setEnabled(true);
    break;
  case 1:
    gen_->setCode("VV", anc::func_VVvis(ui_->VV->value()));
    ui_->VV->setEnabled(true);
    break;
  case 2:
    gen_->clearValue("VV");
    ui_->VV->setEnabled(false);
    break;
  }

  slotGenerate();
}

//! Солнечное сияние
void Kn01::slotSSSChanged()
{
  if (0 == gen_) {
    return;
  }

  if (ui_->SSSmode->isChecked()) {
    gen_->setValue("SSS", ui_->SSS->value());
  } else {
    gen_->clearValue("SSS");
  }

  slotGenerate();
}

//! Осадки
void Kn01::slotPrecpChanged()
{
  if (0 == gen_) {
    return;
  }
  gen_->setValue("iR", ui_->iR->currentIndex());

  ui_->tR->setEnabled(true);
  ui_->RRR->setEnabled(true);

  switch(ui_->iR->currentIndex()) {
  case 0:
    gen_->setValue("tR", ui_->tR->currentIndex());
    gen_->setValue("RRR", ui_->RRR->value());
    gen_->setValue("tR6", ui_->tR->currentIndex());
    gen_->setValue("RRR6", ui_->RRR->value());
    break;
  case 1:
    gen_->setValue("tR", ui_->tR->currentIndex());
    gen_->setValue("RRR", ui_->RRR->value());
    gen_->clearValue("tR6");
    gen_->clearValue("RRR6");
    break;
  case 2:
    gen_->clearValue("tR");
    gen_->clearValue("RRR");
    gen_->setValue("tR6", ui_->tR->currentIndex());
    gen_->setValue("RRR6", ui_->RRR->value());
    break;
  case 3: case 4:
    gen_->clearValue("tR");
    gen_->clearValue("RRR");
    gen_->clearValue("tR6");
    gen_->clearValue("RRR6");
    ui_->tR->setEnabled(false);
    ui_->RRR->setEnabled(false);
    break;
  }

  slotGenerate();
}

//! Погода
void Kn01::slotWeatherChanged()
{
  if (0 == gen_) {
    return;
  }
  
  gen_->setValue("ix", ui_->ix->currentIndex() + 1);
  
  if (ui_->ix->currentIndex() == 0) {
    ui_->ww->setEnabled(true);
    ui_->W1->setEnabled(true);
    ui_->W2->setEnabled(true);
    gen_->setValue("ww", ui_->ww->value());
    gen_->setCode("W1",  ui_->W1->itemData(ui_->W1->currentIndex(), Qt::UserRole).toString());
    gen_->setCode("W2",  ui_->W2->itemData(ui_->W1->currentIndex(), Qt::UserRole).toString());
  } else {
    ui_->ww->setEnabled(false);
    ui_->W1->setEnabled(false);
    ui_->W2->setEnabled(false);
    gen_->clearValue("ww");
    gen_->clearValue("W1");
    gen_->clearValue("W2");
  }

  slotGenerate();
}

//! Максимальная температура
void Kn01::slotIsTxChanged(int state)
{
  if (0 == gen_) {
    return;
  }

  if (state == Qt::Checked) {
    ui_->snTxTxTx->setEnabled(true);
    gen_->setValue("snTxTxTx", ui_->snTxTxTx->value());
  } else {
    ui_->snTxTxTx->setEnabled(false);
    gen_->clearValue("snTxTxTx");
  }

  slotGenerate();
}

//! Минимальная температура
void Kn01::slotIsTnChanged(int state)
{
  if (0 == gen_) {
    return;
  }

  if (state == Qt::Checked) {
    ui_->snTnTnTn->setEnabled(true);
    gen_->setValue("snTnTnTn", ui_->snTnTnTn->value());
  } else {
    ui_->snTnTnTn->setEnabled(false);
    gen_->clearValue("snTnTnTn");
  }

  slotGenerate();
}

//! Давление/геопотенциал
void Kn01::slotIsPPPPChanged(bool checked)
{
  if (0 == gen_) {
    return;
  }

  if (checked) {
    ui_->PPPP->setEnabled(true);
    ui_->a3->setEnabled(false);
    ui_->hhh->setEnabled(false);
    gen_->setValue("PPPP", ui_->PPPP->value());
    gen_->clearValue("a3");
    gen_->clearValue("hhh");
  } else {
    ui_->PPPP->setEnabled(false);
    ui_->a3->setEnabled(true);
    ui_->hhh->setEnabled(true);
    gen_->clearValue("PPPP");
    gen_->setValue("a3", ui_->a3->itemData(ui_->a3->currentIndex(), Qt::UserRole).toString());
    gen_->setValue("hhh", ui_->hhh->value());
  }
  slotGenerate();
}

//! Температура почвы
void Kn01::slotIsTgChanged(int state)
{
  if (0 == gen_) {
    return;
  }

  if (state == Qt::Checked) {
    ui_->snTgTg->setEnabled(true);
    gen_->setValue("snTgTg", ui_->snTgTg->value());
  } else {
    ui_->snTgTg->setEnabled(false);
    gen_->clearValue("snTgTg");
  }

  slotGenerate();
}

//! Поверхность, снег
void Kn01::slotSssChanged()
{
  if (0 == gen_) {
    return;
  }

  if (ui_->sssMode->currentIndex() == 0) {
    ui_->E->setEnabled(true);
    ui_->E_->setEnabled(false);
    gen_->setCode("E", ui_->E->itemData(ui_->E->currentIndex(), Qt::UserRole).toString());
    gen_->clearValue("E_");
  } else {
    ui_->E->setEnabled(false);
    ui_->E_->setEnabled(true);
    gen_->setCode("E_", ui_->E_->itemData(ui_->E_->currentIndex(), Qt::UserRole).toString());
    gen_->clearValue("E");
  }

  if (ui_->sssMode->currentIndex() == 1) {
    ui_->sss->setEnabled(true);
  } else {
    ui_->sss->setEnabled(false);
  }
  
  switch (ui_->sssMode->currentIndex()) {
  case 0:
    gen_->clearValue("sss");
    break;
  case 1:
    gen_->setValue("sss", ui_->sss->value());
    break;
  case 2:
    gen_->setCode("sss", "997");
    break;
  case 3:
    gen_->setCode("sss", "998");
    break;
  case 4: case 5:
    gen_->setCode("sss", "999");
    break;
  }

  slotGenerate();
}

//! Национальные данные
void Kn01::slotNationalChanged()
{
  if (0 == gen_) {
    return;
  }

  gen_->clearValue("snTg_Tg_");
  gen_->clearValue("E5");
  gen_->clearValue("snT24T24T24");
  gen_->clearValue("snT2T2");
  gen_->clearValue("f12f12");
  gen_->clearValue("R24R24R24");
  gen_->clearValue("R24R24R24_check");
  

  if (ui_->is_snTg_Tg_->checkState() == Qt::Checked) {
    gen_->setValue("snTg_Tg_", ui_->snTg_Tg_->value());
    ui_->snTg_Tg_->setEnabled(true);
  } else {
    ui_->snTg_Tg_->setEnabled(false);
  }

  gen_->setCode("E5", ui_->E5->itemData(ui_->E5->currentIndex(), Qt::UserRole).toString());
  if (ui_->is_snT24T24T24->checkState() == Qt::Checked) {
    gen_->setValue("snT24T24T24", ui_->snT24T24T24->value());
    ui_->snT24T24T24->setEnabled(true);
  } else {
    ui_->snT24T24T24->setEnabled(false);
  }

  if (ui_->is_snT2T2->checkState() == Qt::Checked) {
    gen_->setValue("snT2T2", ui_->snT2T2->value());
    ui_->snT2T2->setEnabled(true);
  } else {
    ui_->snT2T2->setEnabled(false);
  }

  if (ui_->is_f12f12->checkState() == Qt::Checked) {
    gen_->setValue("f12f12", ui_->f12f12->value());
    ui_->f12f12->setEnabled(true);
  } else {
    ui_->f12f12->setEnabled(false);
  }

  if (ui_->is_R24R24R24->checkState() == Qt::Checked) {
    gen_->setValue("R24R24R24", ui_->R24R24R24->value());
    ui_->R24R24R24->setEnabled(true);
    if (ui_->R24R24R24->value() >= 30) {
      gen_->setValue("R24R24R24_check", ui_->R24R24R24->value());
    } else {
      gen_->clearValue("R24R24R24_check");
    }
  } else {
    ui_->R24R24R24->setEnabled(false);
  }

  slotGenerate();
}


void Kn01::slotAddChanged()
{
  gen_->clearGroupList("9Sp");
  // int Sp1, Sp2;
  // int sp1, sp2;

  QWidget* wfocus = QApplication::focusWidget();

  if (ui_->is_Sptornado->checkState() == Qt::Checked) {
    ui_->Sp_tornado->setEnabled(true);
    if (ui_->Sp_tornado->currentIndex() == 0) {
      int idx = gen_->addListValue("9Sp", "SpSp", 61);
      gen_->setListValue("9Sp", idx, "spsp", 19);
      ui_->Sp_Mw->setEnabled(false);
      ui_->Sp_Da->setEnabled(false);
    } else {
      ui_->Sp_Mw->setEnabled(true);
      ui_->Sp_Da->setEnabled(true);
      int idx = gen_->addListValue("9Sp", "SpSp", 19);
      gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_Mw->currentIndex()*10 +
                         ui_->Sp_Da->currentIndex());
    }
  } else {
    ui_->Sp_tornado->setEnabled(false);
    ui_->Sp_Mw->setEnabled(false);
    ui_->Sp_Da->setEnabled(false);
  }

  if (ui_->is_Spgust->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 18);
    gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_sq->currentIndex()*10 +
                       ui_->Sp_Dp->currentIndex());
    ui_->Sp_sq->setEnabled(true);
    ui_->Sp_Dp->setEnabled(true);
  } else {
    ui_->Sp_sq->setEnabled(false);
    ui_->Sp_Dp->setEnabled(false);
  }
  
  if (ui_->is_Spff->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 10 + ui_->Sp_ff_hour->currentIndex());
    gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_ff->value());
    ui_->Sp_ff_hour->setEnabled(true);
    ui_->Sp_ff->setEnabled(true);
  } else {
    ui_->Sp_ff_hour->setEnabled(false);
    ui_->Sp_ff->setEnabled(false);
  }


  if (ui_->is_Spsstorm->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 29);
    gen_->setListValue("9Sp", idx, "spsp", (8 +ui_->Sp_sstorm->currentIndex())*10 +
                       ui_->Sp_S8_->currentIndex());
    ui_->Sp_sstorm->setEnabled(true);
    ui_->Sp_S8_->setEnabled(true);
  } else {
    ui_->Sp_sstorm->setEnabled(false);
    ui_->Sp_S8_->setEnabled(false);
  }
  
  if (ui_->is_Sphail->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 32);
    ui_->Sp_hailRRmode->setEnabled(true);
    if (ui_->Sp_hailRRmode->currentIndex() == 0) {
      ui_->Sp_hailRR->setEnabled(true);
      gen_->setListValue("9Sp", idx, "spsp", anc::func_RR(ui_->Sp_hailRR->value()).toInt());
    } else {
      ui_->Sp_hailRR->setEnabled(false);
      gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_hailRRmode->currentIndex() + 96);
    }
  } else {
    ui_->Sp_hailRR->setEnabled(false);
    ui_->Sp_hailRRmode->setEnabled(false);
  }
  
  if (ui_->is_Spprecp->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 34 + ui_->Sp_precp->currentIndex());
    ui_->Sp_precp->setEnabled(true);
    ui_->Sp_RRmode->setEnabled(true);
    if (ui_->Sp_RRmode->currentIndex() == 0) {
      ui_->Sp_RR->setEnabled(true);
      gen_->setListValue("9Sp", idx, "spsp", anc::func_RR(ui_->Sp_RR->value()).toInt());
    } else {
      ui_->Sp_RR->setEnabled(false);
      gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_RRmode->currentIndex() + 96);
    }
  } else {
    ui_->Sp_RR->setEnabled(false);
    ui_->Sp_precp->setEnabled(false);
    ui_->Sp_RRmode->setEnabled(false);
  }
  
  if (ui_->is_Spfog->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 60);
    gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_fog->currentIndex() + 41);
    ui_->Sp_fog->setEnabled(true);
  } else {
    ui_->Sp_fog->setEnabled(false);
  }
  
  if (ui_->is_SpTw->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 25);
    gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_Tw->value());
    ui_->Sp_Tw->setEnabled(true);
  } else {
    ui_->Sp_Tw->setEnabled(false);
  }
  
  if (ui_->is_Spss->checkState() == Qt::Checked) {
    int idx = gen_->addListValue("9Sp", "SpSp", 31);
    ui_->Sp_ssmode->setEnabled(true);
    if (ui_->Sp_ssmode->currentIndex() == 0) {
      ui_->Sp_ss->setEnabled(true);
      gen_->setListValue("9Sp", idx, "spsp", anc::func_ss(ui_->Sp_ss->value()).toInt());
    } else {
      ui_->Sp_ss->setEnabled(false);
      gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_ssmode->currentIndex() + 96);
    }
  } else {
    ui_->Sp_ss->setEnabled(false);
    ui_->Sp_ssmode->setEnabled(false);
  }
  
  if (ui_->is_Sphaze->checkState() == Qt::Checked) {
    int idx = 0;
    if (ui_->Sp_haze_hour->currentIndex() == 0) {
      idx = gen_->addListValue("9Sp", "SpSp", 61);
    } else {
      idx = gen_->addListValue("9Sp", "SpSp", 65);
    }
    gen_->setListValue("9Sp", idx, "spsp", 10);
    ui_->Sp_haze_hour->setEnabled(true);
  } else {
    ui_->Sp_haze_hour->setEnabled(false);
  }
  
  slotAddMountainClouds();

  slotGenerate();

  if (0 != wfocus) {
    wfocus->setFocus();
  }
}

void Kn01::slotAddMountainClouds()
{
  ui_->Sp_NmEh->setEnabled(false);
  ui_->Sp_NmDa->setEnabled(false);
  ui_->Sp_Nmvp->setEnabled(false);
  ui_->Sp_NmDp->setEnabled(false);
  ui_->Sp_NvEh->setEnabled(false);
  ui_->Sp_NvDa->setEnabled(false);
  ui_->Sp_Nvvp->setEnabled(false);
  ui_->Sp_NvDp->setEnabled(false);

  if (ui_->is_SpNm->isChecked()) {
    int idx = gen_->addListValue("9Sp", "SpSp", 50);
    gen_->setListValue("9Sp", idx, "spsp", ui_->SpNm->currentIndex()*10 +
                       ui_->Sp_n3->currentIndex());

    if (ui_->is_SpNm_Eh->checkState() == Qt::Checked) {
      int idx = gen_->addListValue("9Sp", "SpSp", 58);
      gen_->setListValue("9Sp", idx, "spsp",   ui_->Sp_NmEh->currentIndex()*10 +
                         ui_->Sp_NmDa->currentIndex());
      ui_->Sp_NmEh->setEnabled(true);
      ui_->Sp_NmDa->setEnabled(true);
    }
    if (ui_->is_SpNm_vp->checkState() == Qt::Checked) {
      int idx = gen_->addListValue("9Sp", "SpSp", 59);
      gen_->setListValue("9Sp", idx, "spsp",  ui_->Sp_Nmvp->currentIndex()*10 +
                         ui_->Sp_NmDp->currentIndex());
      ui_->Sp_Nmvp->setEnabled(true);
      ui_->Sp_NmDp->setEnabled(true);
    }
  }

  if (ui_->is_SpNv->isChecked()) {
    int idx = gen_->addListValue("9Sp", "SpSp", 51);
    gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_Nv->currentIndex()*10 +
                       ui_->Sp_n4->currentIndex());

    if (ui_->is_SpNv_Eh->checkState() == Qt::Checked) {
      int idx = gen_->addListValue("9Sp", "SpSp", 58);
      gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_NvEh->currentIndex()*10 +
                         ui_->Sp_NvDa->currentIndex());
      ui_->Sp_NvEh->setEnabled(true);
      ui_->Sp_NvDa->setEnabled(true);
    }
    if (ui_->is_SpNv_vp->checkState() == Qt::Checked) {
      int idx = gen_->addListValue("9Sp", "SpSp", 59);
      gen_->setListValue("9Sp", idx, "spsp", ui_->Sp_Nvvp->currentIndex()*10 +
                         ui_->Sp_NvDp->currentIndex());
      ui_->Sp_Nvvp->setEnabled(true);
      ui_->Sp_NvDp->setEnabled(true);
    }
  }

}

bool Kn01::checkCorrectInput()
{
  if (!checkParams(*ui_->IIiii, QObject::tr("индекса станции"))) return false;
  if (!checkParams(*ui_->A1A2, QObject::tr("A1A2"))) return false;
  if (!checkParams(*ui_->ii, QObject::tr("ii"))) return false;
  if (!checkParams(*ui_->BBB, QObject::tr("BBB"))) return false;
  return true;
}

void Kn01::slotSaveTlg()
{
  if (!checkCorrectInput()) return;
  QFileDialog diag;
  diag.setDirectory(QDir::home());
  diag.setFileMode( QFileDialog::AnyFile );
  diag.setAcceptMode( QFileDialog::AcceptSave );
  QString defaultName = ui_->T1T2->currentText() + ui_->A1A2->text() + ui_->ii->text().rightJustified(2, '0');
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

void Kn01::slotSendTlg()
{
  if (!checkCorrectInput()) return;
  QString defaultName = ui_->T1T2->currentText() + ui_->A1A2->text() +
      ui_->ii->text().rightJustified(2, '0') + ui_->dt->dateTime().toString("ddhhmm") + ".tlg";;
  saveInnerDoc(defaultName);
}

bool Kn01::saveInnerDoc(const QString& name)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  tlg::MessageNew req;
  tlg::Header* header = req.mutable_header();

  header->set_t1(ui_->T1T2->currentText().left(1).toStdString());
  header->set_t2(ui_->T1T2->currentText().right(1).toStdString());
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

  meteo::msgcenter::CreateTelegramReply* reply = nullptr;
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

void Kn01::slotHelp(bool check)
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
    helpUi_->setWindowTitle("Схема кода КН-01");
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

//---

SelectWeather::SelectWeather():
  w_(-1)
{
  ui_ = new Ui::SelectWeather;
  ui_->setupUi(this);

  //connect(_ui->w, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(apply()));
  connect(ui_->w, SIGNAL(itemSelectionChanged()), SLOT(slotSelected()));
  connect(ui_->accept, SIGNAL(clicked()), SLOT(slotApply()));
  connect(ui_->cancel, SIGNAL(clicked()), SLOT(reject()));
  connect(ui_->w, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotApplyItem(QTreeWidgetItem*)));

  for (int i = 0; i < ui_->w->topLevelItemCount(); i++) {
    ui_->w->topLevelItem(i)->setExpanded(true);
  }

  fillww();
}

SelectWeather::~SelectWeather()
{
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
}

void SelectWeather::slotApply()
{
  QList<QTreeWidgetItem *> items = ui_->w->selectedItems();
  if (items.size() !=0 && items.at(0) != 0) {
    bool ok;
    w_ = items.at(0)->text(0).toInt(&ok);
    if (!ok) w_ = -1;
  }

  accept();
}

void SelectWeather::slotSelected()
{
  QList<QTreeWidgetItem *>  items =  ui_->w->selectedItems();
  if (items.size() !=0 && items.at(0) != 0) {
    ui_->accept->setEnabled(true);
  } else {
    ui_->accept->setEnabled(false);
  }
}

void SelectWeather::slotApplyItem(QTreeWidgetItem* item)
{
  if (0 != item && 0 == item->childCount()) {
    slotApply();
  }
  
}

void SelectWeather::fillww()
{
  //00-19
  QTreeWidgetItem* w00 = ui_->w->topLevelItem(0)->child(0);
  w00->setText(1, QObject::tr("Без осадков, тумана, ледяного тумана (за исключением 11 и 12), пыльной или\n"
                                  "песчаной бури, снежной низовой метели или поземка на станции"));
  
  w00->child(0)->setText(1, QObject::tr("За прошедший час развитие облачности не наблюдалось или не наблюдается"));
  w00->child(1)->setText(1, QObject::tr("За прошедший час облака в целом рассеиваются или становятся менее развитыми"));
  w00->child(2)->setText(1, QObject::tr("За прошедший час состояние неба в общем не изменилось"));
  w00->child(3)->setText(1, QObject::tr("За прошедший час облака в целом образовывались или развивались"));

  w00->child(4)->setText(1, QObject::tr("В срок наблюдения видимость ухудшена дымом"));
  w00->child(5)->setText(1, QObject::tr("В срок наблюдения в мгла"));
  w00->child(6)->setText(1, QObject::tr("В срок наблюдения в пыль, взвешенная в воздухе на обширном пространстве,\n"
                                            "но не поднятая ветром"));
  w00->child(7)->setText(1, QObject::tr("В срок наблюдения в пыль или песок, поднятые ветром на или вблизи станции\n"
                                            "в срок наблюдения, но нет хорошо развитого пыльного или песчаного вихря (вихрей),\n"
                                            "и никакой пыльной или песчаной бури не видно"));
  w00->child(8)->setText(1, QObject::tr("Хорошо развитый пыльный или песчаный вихрь (вихри) на или вблизи станции\n"
                                            "в течение последнего часа или в срок наблюдения, но пыльной или песчаной бури нет"));
  w00->child(9)->setText(1, QObject::tr("Пыльная или песчаная буря в поле зрения в срок наблюдения или на станции\n"
                                            "в течение последнего часа"));
  w00->child(10)->setText(1, QObject::tr("Дымка"));
  w00->child(11)->setText(1, QObject::tr("Клочья приземного или ледяного тумана на станции, на море или на суше, высотой\n"
                                             "не более 2 м над сушей или не более 10 м над морем"));
  w00->child(12)->setText(1, QObject::tr("Более или менее сплошной слой приземного или ледяного тумана на станции, на море\n"
                                             "или на суше, высотой не более 2 м над сушей или не более 10 м над морем"));
  w00->child(13)->setText(1, QObject::tr("Видна молния, грома не слышно"));
  w00->child(14)->setText(1, QObject::tr("Осадки в поле зрения, достигающие поверхности земли или моря поблизости, но не на самой станции"));
  w00->child(15)->setText(1, QObject::tr("Осадки в поле зрения, достигающие поверхности земли или моря на расстоянии более 5 км от станции"));
  w00->child(16)->setText(1, QObject::tr("Осадки в поле зрения, достигающие поверхности земли или моря поблизости, но не на самой станции"));
  w00->child(17)->setText(1, QObject::tr("Гроза, но без осадков, в срок наблюдения"));
  w00->child(18)->setText(1, QObject::tr("Шквалы на станции или в поле зрения за последний час или в срок наблюдения"));
  w00->child(19)->setText(1, QObject::tr("Воронкообразное(ые) облако(а) на станции или в поле зрения за последний час или в срок наблюдения"));

  QTreeWidgetItem *w20 = ui_->w->topLevelItem(0)->child(1);
  w20->setText(1, QObject::tr("Осадки, туман, гроза в последний час, но не в срок наблюдения"));
  w20->child(0)->setText(1, QObject::tr("Морось (незамерзающая) или снежные зерна"));
  w20->child(1)->setText(1, QObject::tr("Дождь (незамерзающий)"));
  w20->child(2)->setText(1, QObject::tr("Снег"));
  w20->child(3)->setText(1, QObject::tr("Дождь со снегом или ледяной дождь"));
  w20->child(4)->setText(1, QObject::tr("Морось (дождь) замерзающие, образующие гололед"));
  w20->child(5)->setText(1, QObject::tr("Ливневой дождь"));
  w20->child(6)->setText(1, QObject::tr("Ливневой дождь со снегом или ливневой снег"));
  w20->child(7)->setText(1, QObject::tr("Град, крупа (ледяная или снежная) – с дождем или без дождя"));
  w20->child(8)->setText(1, QObject::tr("Туман или ледяной туман, видимость менее 1 км"));
  w20->child(9)->setText(1, QObject::tr("Гроза (с осадками или без них)"));

  QTreeWidgetItem* w30 = ui_->w->topLevelItem(0)->child(2);
  w30->setText(1, QObject::tr("Пыльная буря, песчаная буря, поземок или снежная низовая метель"));
  w30->child(0)->setText(1, QObject::tr("Слабая или умеренная пыльная или песчаная буря ослабела за последний час"));
  w30->child(1)->setText(1, QObject::tr("Слабая или умеренная пыльная или песчаная буря без заметного изменения в течение последнего часа"));
  w30->child(2)->setText(1, QObject::tr("Слабая или умеренная пыльная или песчаная буря началась или усилилась в течение последнего часа"));
  w30->child(3)->setText(1, QObject::tr("Сильная пыльная или песчаная буря ослабела за последний час"));
  w30->child(4)->setText(1, QObject::tr("Сильная пыльная или песчаная буря без заметного изменения в течение последнего часа"));
  w30->child(5)->setText(1, QObject::tr("Сильная пыльная или песчаная буря началась или усилилась в течение последнего часа"));
  w30->child(6)->setText(1, QObject::tr("Слабый или умеренный поземок в целом низкий (ниже уровня глаз наблюдателя)"));
  w30->child(7)->setText(1, QObject::tr("Сильный поземок в целом низкий (ниже уровня глаз наблюдателя)"));
  w30->child(8)->setText(1, QObject::tr("Слабая или умеренная снежная низовая метель в целом высокая (выше уровня глаз наблюдателя)"));
  w30->child(9)->setText(1, QObject::tr("Сильная снежная низовая метель в целом высокая (выше уровня глаз наблюдателя)"));

  QTreeWidgetItem* w40 = ui_->w->topLevelItem(0)->child(3);
  w40->setText(1, QObject::tr("Туман или ледяной туман в срок наблюдения"));
  w40->child(0)->setText(1, QObject::tr("Туман или ледяной туман на расстоянии в срок наблюдения, простирающийся выше уровня\n"
                                            "глаз наблюдателя. В течение последнего часа туман на станции не наблюдался"));
  w40->child(1)->setText(1, QObject::tr("Туман или обрывки ледяного тумана"));
  w40->child(2)->setText(1, QObject::tr("Туман или ледяной туман, небо видно, ослабел за последний час"));
  w40->child(3)->setText(1, QObject::tr("Туман или ледяной туман, неба не видно, ослабел за последний час"));
  w40->child(4)->setText(1, QObject::tr("Туман или ледяной туман, небо видно, без заметного изменения интенсивности в течение последнего часа"));
  w40->child(5)->setText(1, QObject::tr("Туман или ледяной туман, неба не видно, начался или усилился в течение последнего часа"));
  w40->child(6)->setText(1, QObject::tr("Туман или ледяной туман, небо видно, начался или усилился в течение последнего часа"));
  w40->child(7)->setText(1, QObject::tr("Туман или ледяной туман, неба не видно"));
  w40->child(8)->setText(1, QObject::tr("Туман с отложением изморози, небо видно"));
  w40->child(9)->setText(1, QObject::tr("Туман с отложением изморози, неба не видно"));

  //--

  QTreeWidgetItem* w50 = ui_->w->topLevelItem(1)->child(0);
  w50->setText(1, QObject::tr("Морось"));
  w50->child(0)->setText(1, QObject::tr("Морось незамерзающая с перерывами слабая в срок наблюдения"));
  w50->child(1)->setText(1, QObject::tr("Морось незамерзающая непрерывная слабая в срок наблюдения"));
  w50->child(2)->setText(1, QObject::tr("Морось незамерзающая с перерывами умеренная в срок наблюдения"));
  w50->child(3)->setText(1, QObject::tr("Морось незамерзающая непрерывная умеренная в срок наблюдения"));
  w50->child(4)->setText(1, QObject::tr("Морось незамерзающая с перерывами сильная в срок наблюдения"));
  w50->child(5)->setText(1, QObject::tr("Морось незамерзающая непрерывная сильная в срок наблюдения"));
  w50->child(6)->setText(1, QObject::tr("Морось замерзающая слабая"));
  w50->child(7)->setText(1, QObject::tr("Морось замерзающая умеренная или сильная"));
  w50->child(8)->setText(1, QObject::tr("Морось и дождь слабые"));
  w50->child(9)->setText(1, QObject::tr("Морось и дождь умеренные или сильные"));

  QTreeWidgetItem* w60 = ui_->w->topLevelItem(1)->child(1);
  w60->setText(1, QObject::tr("Дождь"));
  w60->child(0)->setText(1, QObject::tr("Дождь незамерзающий с перерывами слабый в срок наблюдения"));
  w60->child(1)->setText(1, QObject::tr("Дождь незамерзающий непрерывный слабый в срок наблюдения"));
  w60->child(2)->setText(1, QObject::tr("Дождь незамерзающий с перерывами умеренный в срок наблюдения"));
  w60->child(3)->setText(1, QObject::tr("Дождь незамерзающий непрерывный умеренный в срок наблюдения"));
  w60->child(4)->setText(1, QObject::tr("Дождь незамерзающий с перерывами сильный в срок наблюдения"));
  w60->child(5)->setText(1, QObject::tr("Дождь незамерзающий непрерывный сильный в срок наблюдения"));
  w60->child(6)->setText(1, QObject::tr("Дождь замерзающий слабый"));
  w60->child(7)->setText(1, QObject::tr("Дождь замерзающий умеренный или сильный"));
  w60->child(8)->setText(1, QObject::tr("Дождь или морось со снегом слабые"));
  w60->child(9)->setText(1, QObject::tr("Дождь или морось со снегом умеренные или сильные"));

  QTreeWidgetItem* w70 = ui_->w->topLevelItem(1)->child(2);
  w70->setText(1, QObject::tr("Твердые осадки, неливневые"));
  w70->child(0)->setText(1, QObject::tr("Снег с перерывами слабый в срок наблюдения"));
  w70->child(1)->setText(1, QObject::tr("Снег непрерывный слабый в срок наблюдения"));
  w70->child(2)->setText(1, QObject::tr("Снег с перерывами умеренный в срок наблюдения"));
  w70->child(3)->setText(1, QObject::tr("Снег непрерывный умеренный в срок наблюдения"));
  w70->child(4)->setText(1, QObject::tr("Снег с перерывами сильный в срок наблюдения"));
  w70->child(5)->setText(1, QObject::tr("Снег непрерывный сильный в срок наблюдения"));
  w70->child(6)->setText(1, QObject::tr("Алмазная пыль (с туманом или без него)"));
  w70->child(7)->setText(1, QObject::tr("Снежные зерна (с туманом или без него)"));
  w70->child(8)->setText(1, QObject::tr("Отдельные кристаллы снега в виде звездочек (с туманом или без него)"));
  w70->child(9)->setText(1, QObject::tr("Ледяная крупа"));

  QTreeWidgetItem* w80 = ui_->w->topLevelItem(1)->child(3);
  w80->setText(1, QObject::tr("Ливневые осадки или осадки с грозой в срок наблюдения или за последний час"));
  w80->child(0)->setText(1, QObject::tr("Ливневый(ые) дождь(и) слабый(ые)"));
  w80->child(1)->setText(1, QObject::tr("Ливневый(ые) дождь(и) умеренный(ые) или сильный(ые)"));
  w80->child(2)->setText(1, QObject::tr("Ливневый(ые) дождь(и) очень сильный(ые)"));
  w80->child(3)->setText(1, QObject::tr("Ливневый(ые) дождь(и) со снегом слабый(ые)"));
  w80->child(4)->setText(1, QObject::tr("Ливневый(ые) дождь(и) со снегом умеренный(ые) или сильный(ые)"));
  w80->child(5)->setText(1, QObject::tr("Ливневый снег слабый"));
  w80->child(6)->setText(1, QObject::tr("Ливневый снег умеренный или сильный"));
  w80->child(7)->setText(1, QObject::tr("Ливневая снежная крупа или небольшой град с дождем или без него,\n"
                                            "или дождь со снегом, слабая"));
  w80->child(8)->setText(1, QObject::tr("Ливневая снежная крупа или небольшой град с дождем или без него,\n"
                                            "или дождь со снегом, умеренный или сильный"));
  w80->child(9)->setText(1, QObject::tr("Ливневый град с дождем или без него, или дождь со снегом без грома, слабый"));
  w80->child(10)->setText(1, QObject::tr("Ливневый град с дождем или без него, или дождь со снегом без грома, умеренный или сильный"));

  QTreeWidgetItem* w91 = ui_->w->topLevelItem(1)->child(4);
  w91->setText(1, QObject::tr("Гроза в течение последнего часа, но не в срок наблюдения"));
  w91->child(0)->setText(1, QObject::tr("Слабый дождь в срок наблюдения, гроза в течение последнего часа,\n"
                                            "но не в срок наблюдения"));
  w91->child(1)->setText(1, QObject::tr("Умеренный или сильный дождь в срок наблюдения, гроза в течение последнего часа,\n"
                                            "но не в срок наблюдения"));
  w91->child(2)->setText(1, QObject::tr("Слабый снег или дождь со снегом или град в срок наблюдения, гроза в течение последнего часа,\n"
                                            "но не в срок наблюдения"));
  w91->child(3)->setText(1, QObject::tr("Умеренный или сильный снег или дождь со снегом, или град в срок наблюдения, гроза в течение\n"
                                            "последнего часа, но не в срок наблюдения"));

  QTreeWidgetItem* w95 = ui_->w->topLevelItem(1)->child(5);
  w95->setText(1, QObject::tr("Гроза в срок наблюдения"));
  w95->child(0)->setText(1, QObject::tr("Гроза слабая или умеренная без града, но с дождем и/или снегом в срок наблюдения,\n"
                                            "гроза в срок наблюдения"));
  w95->child(1)->setText(1, QObject::tr("Гроза слабая или умеренная с градом в срок наблюдения, гроза в срок наблюдения"));
  w95->child(2)->setText(1, QObject::tr("Гроза сильная без града, но с дождем и/или снегом в срок наблюдения, гроза в срок наблюдения"));
  w95->child(3)->setText(1, QObject::tr("Гроза вместе с пыльной или песчаной бурей в срок наблюдения, гроза в срок наблюдения"));
  w95->child(4)->setText(1, QObject::tr("Гроза сильная с градом в срок наблюдения, гроза в срок наблюдения"));
}
