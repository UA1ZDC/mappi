#include "tlgeditorwindow.h"
#include "ui_tlgeditorwindow.h"
#include "codecdlg.h"
#include "senddlg.h"

#include <qdir.h>
#include <qfile.h>
#include <qevent.h>
#include <qsettings.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/funcs/tlgtextobject.h>

#include <commons/meteo_data/meteo_data.h>

#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/msgparser/tlgfuncs.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/proto/decoders.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/ui/custom/filedialogrus.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/ui/customviewer/decode/decodeviewer.h>

namespace meteo {

TlgEditorWindow::TlgEditorWindow(QWidget *parent)
  : QMainWindow(parent),
    ui_(new Ui::TlgEditorWindow),
    codecName_("UTF-8")
{
  ui_->setupUi(this);

  decodeview_ = new DecodeViewer(this);
  ui_->decodeTab->layout()->addWidget(decodeview_);

  actSymbols_.insert(ui_->insertSohAct, "<SOH>");
  actSymbols_.insert(ui_->insertStxAct, "<STX>");
  actSymbols_.insert(ui_->insertEtxAct, "<ETX>");
  actSymbols_.insert(ui_->insertCrAct, "<CR>");
  actSymbols_.insert(ui_->insertLfAct, "<LF>");
  actSymbols_.insert(ui_->insertSoAct, "<SO>");
  actSymbols_.insert(ui_->insertSiAct, "<SI>");

  ui_->codecBtn->setText(codecName_);

  connect( ui_->newAct, SIGNAL(triggered(bool)), SLOT(slotNewTlg()) );
  connect( ui_->newWmoCircAct, SIGNAL(triggered(bool)), SLOT(slotNewTlg()) );
  connect( ui_->newWmoAddrAct, SIGNAL(triggered(bool)), SLOT(slotNewTlg()) );
  connect( ui_->newHmsCircAct, SIGNAL(triggered(bool)), SLOT(slotNewTlg()) );
  connect( ui_->newHmsAddrAct, SIGNAL(triggered(bool)), SLOT(slotNewTlg()) );
  connect( ui_->openAct, SIGNAL(triggered(bool)), SLOT(slotOpenFile()) );
  connect( ui_->saveAct, SIGNAL(triggered(bool)), SLOT(slotSaveFile()) );
  connect( ui_->saveAsAct, SIGNAL(triggered(bool)), SLOT(slotSaveAsFile()) );
  connect( ui_->sendAct, SIGNAL(triggered(bool)), SLOT(slotSendTlg()) );

  connect( ui_->codecBtn, SIGNAL(clicked(bool)), SLOT(slotRecodeTlg()) );

  connect( ui_->insertSohAct, SIGNAL(triggered(bool)), SLOT(slotInsertSpecSymb()) );
  connect( ui_->insertStxAct, SIGNAL(triggered(bool)), SLOT(slotInsertSpecSymb()) );
  connect( ui_->insertEtxAct, SIGNAL(triggered(bool)), SLOT(slotInsertSpecSymb()) );
  connect( ui_->insertCrAct, SIGNAL(triggered(bool)), SLOT(slotInsertSpecSymb()) );
  connect( ui_->insertLfAct, SIGNAL(triggered(bool)), SLOT(slotInsertSpecSymb()) );
  connect( ui_->insertSoAct, SIGNAL(triggered(bool)), SLOT(slotInsertSpecSymb()) );
  connect( ui_->insertSiAct, SIGNAL(triggered(bool)), SLOT(slotInsertSpecSymb()) );

  connect( ui_->tabWidget, SIGNAL(currentChanged(int)), SLOT(slotUpdateDecodeTlg()) );
}

TlgEditorWindow::~TlgEditorWindow()
{
  delete ui_;
}

bool TlgEditorWindow::setRawTlg(const QByteArray& raw)
{
  tlg::MessageNew msg;

  TlgParser parser(raw);
  if ( !parser.parseNextMessage(&msg) ) {
    error_log << tr("Структура сообщения некорректна.");
    return false;
  }

  if ( !msg.isbinary() ) {
    codecName_ = detectCodec(raw);
  }

  ui_->tlgEditor->setCodecName(codecName_);
  ui_->tlgEditor->setStartLine(parser.startline());
  ui_->tlgEditor->setHeader(parser.header());
  ui_->tlgEditor->setMessage(parser.message(), msg.isbinary() ? TlgEditor::kBinary : TlgEditor::kText);
  ui_->tlgEditor->setEnd(parser.end());
  ui_->tlgEditor->setModified(false);

  ui_->tlgEditor->setTlgFormat(msg.isbinary() ? TlgEditor::kBinary : TlgEditor::kText);

  rawData_ = raw;

  return true;
}

QByteArray TlgEditorWindow::getRawTlg() const
{
  QByteArray raw;
  raw += ui_->tlgEditor->startLine();
  raw += ui_->tlgEditor->header();
  raw += ui_->tlgEditor->message();
  raw += ui_->tlgEditor->end();

  return raw;
}

bool TlgEditorWindow::openFile(const QString& fileName)
{
  if ( fileName.isNull() ) { return false; }

  fileName_ = fileName;

  if ( false == checkModified(tr("Не сохранённые изменения будут потеряны. Продолжить?"), tr("Да")) ) {
    return false;
  }

  QFile file(fileName_);
  if ( !file.open(QFile::ReadOnly) ) {
    error_log << tr("Не удалось открыть файл %1").arg(fileName_);
    return false;
  }

  if ( file.size() > 20*1024*1024  ) {
    QString title = tr("Открыть файл?");
    QString text  = tr("Размер открываемого файла больше 20 Мбайт. Продолжить?");
    int n = QMessageBox::question(0, title, text, QMessageBox::Yes | QMessageBox::No);

    if ( 1 == n ) { return false; }
  }

  ui_->tlgEditor->clear();

  rawData_ = file.readAll();

  return setRawTlg(rawData_);
}

void TlgEditorWindow::slotNewTlg()
{
  QAction* act = qobject_cast<QAction*>(sender());

  if ( 0 == act ) { return; }

  if ( false == checkModified(tr("Несохранённые изменения будут потеряты. Продолжить?"), tr("Да")) ) {
    return;
  }

  ui_->tlgEditor->clear();

  QString name;
  if ( ui_->newWmoCircAct == act ) {name = "wmocirc"; }
  else if ( ui_->newWmoAddrAct == act ) { name = "wmoaddr"; }
  else if ( ui_->newHmsCircAct == act ) { name = "hmscirc"; }
  else if ( ui_->newHmsAddrAct == act ) { name = "hmsaddr"; }

  QMap<QByteArray,QByteArray> repl;
  repl["@WMO_CCCC@"] = gSettings()->wmoId().toUtf8();
  repl["@HMS_CCCC@"] = gSettings()->hmsId().toUtf8();
  repl["@YYGGgg@"] = QDateTime::currentDateTimeUtc().toString("ddhhmm").toUtf8();

  if ( !name.isEmpty() ) {
    QFile file(QString(":/novost/tlgeditor/%1.tlg").arg(name));
    if ( file.open(QFile::ReadOnly ) ) {
      QByteArray ba = file.readAll();
      foreach ( const QByteArray& key, repl.keys() ) {
        ba.replace(key, repl[key]);
      }
      setRawTlg(ba);
    }
    else {
      debug_log << tr("Не удалось открыть файл '%1'\n").arg(file.fileName())
                << tr("QFile: %1").arg(file.errorString());
    }
  }
}

void TlgEditorWindow::slotOpenFile()
{
  if ( false == checkModified(tr("Несохранённые изменения будут потеряты. Продолжить?"), tr("Да")) ) {
    return;
  }

  openFile(meteo::FileDialog::getOpenFileName());
}

void TlgEditorWindow::slotSaveFile()
{
  if ( fileName_.isEmpty() ) {
    fileName_ = meteo::FileDialog::getSaveFileName();
  }

  if ( fileName_.isEmpty() ) { return; }

  QFile file(fileName_);
  if ( !file.open(QFile::WriteOnly | QFile::Truncate) ) {
    error_log << tr("Ошибка при сохранении телеграммы в файл %1").arg(fileName_);
    return;
  }

  QByteArray raw = getRawTlg();

  QDataStream out(&file);
  int wsz = out.writeRawData(raw.data(), raw.size());
  if ( wsz != raw.size() ) {
    error_log << tr("Ошибка при записи данных в файл %1").arg(fileName_);
    return;
  }

  ui_->tlgEditor->setModified(false);
}

void TlgEditorWindow::slotSaveAsFile()
{
  QString fn = meteo::FileDialog::getSaveFileName();

  if ( fn.isEmpty() ) { return; }

  fileName_ = fn;
  slotSaveFile();
}

void TlgEditorWindow::slotRecodeTlg()
{
  CodecDlg dlg;
  CodecDlg::Button result = dlg.run();
  if ( CodecDlg::kCancelButton == result ) {
    return;
  }

  codecName_ = dlg.codecName();
  ui_->codecBtn->setText(codecName_);

  if ( CodecDlg::kReloadButton == result ) {
    if ( !fileName_.isEmpty() ) {
      slotReloadFile();
    }
    else {
      setRawTlg(rawData_);
    }
  }
  else if ( CodecDlg::kSaveButton == result ) {
    slotSaveFile();
  }
}

void TlgEditorWindow::slotReloadFile()
{
  openFile(fileName_);
}

void TlgEditorWindow::slotSendTlg()
{
  tlg::MessageNew msg;

  QByteArray raw = getRawTlg();

  TlgParser parser(raw);
  if ( !parser.parseNextMessage(&msg) ) {
    error_log.msgBox() << tr("Структура сообщения некорректна.");
    return;
  }

  QDateTime dt = convertedDateTime(QDateTime::currentDateTime(), QString::fromUtf8(msg.header().yygggg().c_str()));
  msg.mutable_metainfo()->set_converted_dt(dt.toString(Qt::ISODate).toUtf8().constData());

  rpc::Channel* ch = global::serviceChannel( meteo::settings::proto::kMsgCenter );
  if ( 0 == ch ) {
    error_log.msgBox() << tr("Не удалось отправить сообщение в ЦКС. Не удалось подключиться к сервису");
    return;
  }

//  rpc::Address addr = ch->address();

  qApp->setOverrideCursor(Qt::WaitCursor);

  msgcenter::CreateTelegramReply* d = ch->remoteCall(&msgcenter::MsgCenterService::CreateTelegram, msg, 30000);

  ch->disconnect();
  delete ch;

  qApp->restoreOverrideCursor();

  if ( 0 == d ) {
    error_log.msgBox() << tr("Не удалось отправить сообщение в ЦКС.");
    return;
  }
  else {
    info_log.msgBox() << tr("Отправлено сообщение");
  }

  delete d;
}

void TlgEditorWindow::slotInsertSpecSymb()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if ( !actSymbols_.contains(act) ) {
    return;
  }

  QTextCodec* codec = QTextCodec::codecForName(codecName_.isEmpty() ? "UTF-8" : codecName_.toUtf8());
  if ( 0 == codec ) { return; }

  QString symb = actSymbols_[act];
  QByteArray value = codec->fromUnicode(QString(TlgTextObject::kSpecChars[symb]));

  ui_->tlgEditor->insertData(value);
}

void TlgEditorWindow::slotUpdateDecodeTlg()
{
  if ( ui_->tabWidget->currentWidget() != ui_->decodeTab ) { return; }

  QByteArray ba = getRawTlg();

  decodeview_->init(ba, ALPHANUM, QString());
}

void TlgEditorWindow::closeEvent(QCloseEvent* event)
{
  if ( false == checkModified(tr("Сохранить изменения перед закрытием?"), tr("Не сохранять")) ) {
    event->ignore();
    return;
  }

  event->accept();
}

bool TlgEditorWindow::checkModified(const QString& text, const QString& btn)
{
  while ( ui_->tlgEditor->isModified() ) {
    int r = QMessageBox::question(0, tr("Cохранить телеграмму?"), text, btn, tr("Отмена"), tr("Сохранить"));
    if ( 0 == r ) {
      break;
    }
    if ( 1 == r ) {
      return false;
    }
    if ( 2 == r ) {
      slotSaveFile();
    }
  }

  return true;
}

QString TlgEditorWindow::detectCodec(const QByteArray& raw) const
{
  QStringList checkList = QStringList()
      << "МОСКВА"
      << "РОС"
      << "TEC"
      << "КРА"
      << "ГИДРО"
      << "МЕТ"
      << "ЦЕНТР"
      << "ПОГОДА"
      << "ПРОГНОЗ"
      << "ДНЕМ"
      << "ТЕМПЕРАТУРА"
      << "ВОЗДУХА"
      << "НОЧЬ";

  QStringList codecs = QStringList()
      << "KOI8-R"
      << "WINDOWS-1251"
      << "UTF-8"
         ;

  QString msg;
  foreach ( const QString& n, codecs ) {
    QTextCodec* codec = QTextCodec::codecForName(n.toUtf8());
    msg = codec->toUnicode(raw);

    foreach ( const QString& s, checkList ) {
      if ( msg.contains(s, Qt::CaseInsensitive) ) {
        return n;
      }
    }
  }

  return codecs.last();
}

} // meteo
