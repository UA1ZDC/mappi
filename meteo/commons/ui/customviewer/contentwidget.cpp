#include "contentwidget.h"
#include "ui_contentwidget.h"

#include <qapplication.h>
#include <qdesktopservices.h>
#include <qcursor.h>
#include <qlist.h>
#include <qmap.h>
#include <qtemporaryfile.h>
#include <qfiledialog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/funcs/tlgtextobject.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/map/view/custom/faxviewer.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/ui/custom/filedialogrus.h>
#include <meteo/commons/ui/tlgeditor/tlgeditorwindow.h>
#include <meteo/commons/faxes/datachain.h>
#include <meteo/commons/proto/customviewer.pb.h>
#include <meteo/commons/proto/document_service.pb.h>

#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>


//static const QString dbTelegrams = "telegramsdb";
namespace meteo {

static const TMap<QString,meteo::TypeTelegram> tlgtypes = TMap<QString,meteo::TypeTelegram>()
<< QPair<QString,meteo::TypeTelegram>("grib", meteo::GRIB)
<< QPair<QString,meteo::TypeTelegram>("bufr", meteo::BUFR)
<< QPair<QString,meteo::TypeTelegram>("alphanum", meteo::ALPHANUM);

static QString getNameCyrByNameSMB(const QString& nameSmb)
{
  static QHash<QString, QString> areasCache_;

  if ( true == areasCache_.contains(nameSmb) ){
    return areasCache_[nameSmb];
  }

  auto serviceCode = settings::proto::kCustomViewerService;
  auto serviceTitle = global::serviceTitle(serviceCode);
  std::unique_ptr<rpc::Channel> channel(global::serviceChannel(serviceCode));
  if (nullptr == channel ){
    error_log << msglog::kServiceConnectFailed.arg(serviceTitle);
    return nameSmb;
  }

  proto::Dummy request;
  std::unique_ptr<proto::GetTypecAreasResponce> responce(channel->remoteCall(&proto::CustomViewerService::GetTypecAreas, request, 10000));
  if ( nullptr == responce ){
    return nameSmb;
  }

  for ( auto area: responce->area() ){
    auto nameSmb = QString::fromStdString(area.name_smb());
    auto nameCyr = QString::fromStdString(area.name_cyr());

    if (true == nameCyr.isEmpty() ){
      continue;
    }
    areasCache_.insert(nameSmb, nameCyr);
  }

  if ( false == areasCache_.contains(nameSmb) ){
    return nameSmb;
  }

  return areasCache_[nameSmb];
}

ContentWidget::ContentWidget(QWidget *parent, const QString& id_column )
  : QDialog(parent),
    ui_(new Ui::ContentWidget),
    imagePreview_(new meteo::ViewImagePreview(this)),
    decodeview_(new meteo::DecodeViewer(this)),
    message_(nullptr),
    id_column_(id_column)
{
  ui_->setupUi( this );
  QObject::connect( ui_->toolButton, SIGNAL(clicked()), SLOT(accept()));
  QObject::connect( ui_->checkBox, SIGNAL(clicked()), SLOT(slotLoadMsg()) );

  ui_->contentPage->layout()->addWidget(decodeview_);
  ui_->faxViewPage->layout()->addWidget(imagePreview_);

  QObject::connect( ui_->editBtn, SIGNAL(clicked(bool)), SLOT(slotEditMsg()) );
  QObject::connect(decodeview_, SIGNAL(reload()), SLOT(slotLoadMsg()));
  QObject::connect(ui_->open_file_btn, SIGNAL(clicked(bool)), SLOT(slotOpenFile()));
  QObject::connect(ui_->save_file_btn, SIGNAL(clicked(bool)), SLOT(slotSaveFile()));
  QObject::connect(ui_->saveBtn,SIGNAL(clicked(bool)), SLOT(slotSaveMsg()));
  ui_->editBtn->setIcon(QIcon(":/meteo/icons/edit.png"));
  ui_->saveBtn->setIcon(QIcon(":/meteo/icons/save.png"));
}

ContentWidget::~ContentWidget()
{
  // delete ui_;
  //  ui_ = nullptr;
  // delete message_;
  //message_ = nullptr;
}

static QString getNameByT1T2(const QString& t1Arg, const QString& t2Arg)
{
  auto t1 = meteo::global::kTranslitFunc(t1Arg).toUpper();
  auto t2 = meteo::global::kTranslitFunc(t2Arg).toUpper();

  static QHash<QPair<QString, QString>, QString> nameHash_;

  if ( true == nameHash_.contains(qMakePair(t1, t2)) ){
    return nameHash_[qMakePair(t1, t2)];
  }

  auto serviceCode = settings::proto::kCustomViewerService;
  auto serviceTitle = global::serviceTitle(serviceCode);
  std::unique_ptr<rpc::Channel> channel(global::serviceChannel(serviceCode));
  if ( nullptr == channel ){
    error_log << msglog::kServiceConnectFailed.arg(serviceTitle);
    return t1 + t2;
  }

  proto::Dummy request;
  std::unique_ptr<proto::GetAvailableTTResponce> responce(channel->remoteCall(&proto::CustomViewerService::GetAvailableTT, request, 10000));
  if ( nullptr == responce ){
    error_log << msglog::kServiceAnswerFailed.arg(serviceTitle);
    return t1 + t2;
  }

  for ( auto t1Proto : responce->t1() ){
    auto t1 = QString::fromStdString(t1Proto.t1());
    QString t2;
    auto name = QString::fromStdString(t1Proto.name());
    nameHash_.insert(qMakePair(t1, t2), name);
  }

  for ( auto t2Proto: responce->t2() ){
    auto t1 = QString::fromStdString(t2Proto.t1());
    auto t2 = QString::fromStdString(t2Proto.t2());
    auto name = QString::fromStdString(t2Proto.name());

    nameHash_.insert(qMakePair(t1, t2), name);
  }

  if ( false == nameHash_.contains(qMakePair(t1, t2))){
    return t1 + t2;
  }
  return nameHash_[qMakePair(t1, t2)];
}

QString parseRouteString(const QString& route)
{
  QJsonDocument doc = QJsonDocument::fromJson(route.toUtf8());
  if ( true == doc.isNull() ){
    return route;
  }
  auto obj = doc.object();
  QStringList routes;
  for (auto index: obj.keys()) {
    auto routeObject = obj[index].toObject();
    static auto kSrc = QObject::tr("src");

    if ( true == routeObject.keys().contains(kSrc) ) {
      routes << routeObject[kSrc].toString();
    }
  }
  return routes.join(";");
}

bool ContentWidget::init(meteo::tlg::MessageNew* tlg, const QMap<QString, QString>& info)
{
  if( nullptr == tlg ){
    return false;
  }
  delete message_;
  message_ = tlg;
  info_ = info;
  QString header;
  header.append(QString::fromStdString(message_->header().t1()));
  header.append(QString::fromStdString(message_->header().t2()));
  header.append(QString::fromStdString(message_->header().a1()));
  header.append(QString::fromStdString(message_->header().a2()));
  header.append(QString::number(message_->header().ii()));
  header.append(QString::fromStdString(message_->header().cccc()));
  header.append(QString::fromStdString(message_->header().yygggg()));
  header.append(QString::fromStdString(message_->header().bbb()));

  ui_->headerLbl->setText(header);
  ui_->channelLbl->setText( parseRouteString(info_["route"]));

  QStringList specification;

  QString groupName = getNameByT1T2( QString::fromStdString(message_->header().t1()),
                                     QString() );
  if ( false == groupName.isEmpty() ){
    specification << groupName;
  }

  QString name = getNameByT1T2( QString::fromStdString(message_->header().t1()),
                                QString::fromStdString(message_->header().t2())
                                );
  if ( false == name.isEmpty() ) {
    specification << name;
  }

  QString nameSMB =
      QString::fromStdString(message_->header().a1()) +
      QString::fromStdString(message_->header().a2());
  QString nameCyr = getNameCyrByNameSMB(nameSMB);
  if ( false == nameCyr.isEmpty() ) {
    specification << nameCyr;
  }

  if( specification.isEmpty() ){
    ui_->specificationLbl->setText(QObject::tr("Описание недоступно"));
  }
  else{
    ui_->specificationLbl->setText(specification.join(", "));
  }
  ui_->hourLbl->setText(info_["msg_dt"]);
  ui_->centerLbl->setText(QString::fromStdString(message_->header().cccc()));
  ui_->id_label->setText(info_[id_column_]);
  QPixmap pix(QString(":/meteo/icons/%1.png").arg(info_["type"]));
  ui_->label_icon->setPixmap(pix.scaled(QSize(48,48), Qt::KeepAspectRatio));
  ui_->label_icon->setToolTip(info_["type"]);
  slotLoadMsg();
  return true;
}

bool ContentWidget::loadImage(const QByteArray& arr)
{
  if( nullptr != ui_->decodedTab  && nullptr == ui_->tabWidget->widget(1)){
    ui_->tabWidget->addTab(ui_->decodedTab, QObject::tr("Сводки"));
  }
  ui_->stackedWidget->setCurrentWidget(ui_->faxViewPage);
  QImage image = QImage::fromData(meteo::tlg::tlg2image(arr));
  imagePreview_->setImage(image);
  return true;
}


bool ContentWidget::loadBinMessage()
{
  if( nullptr == message_ ){
    return false;
  }
  QByteArray startline = QByteArray(message_->startline().data().data(), message_->startline().data().size());
  QByteArray header = QByteArray(message_->header().data().data(), message_->header().data().size());
  QByteArray msg = QByteArray(message_->msg().data(), message_->msg().size());
  QByteArray end = QByteArray(message_->end().data(), message_->end().size());
  QByteArray startdata =  startline + header;
  QString content = replaceContent(startdata, ui_->checkBox->isChecked());
  QString end_content = replaceContent(end, ui_->checkBox->isChecked());
  ui_->contentEdit->setText(content + QObject::tr("\n *** Бинарные данные ***\n") + end_content);

  if ( 0 == info_["type"].compare("doc") ){
    loadFile();
    return true;
  }
  if ( 0 == info_["type"].compare("fax") ){
    loadFaxImage();
    return true;
  }
  if( "sputnik" == info_["type"] ||
      "map" == info_["type"] ||
      "fax" == info_["type"] ||
      "image" == info_["type"]){
    loadImage(msg);
    return true;
  }
  if( "sxf" == info_["type"] ||
      "rsc" == info_["type"] ||
      "map_ptkpp" == info_["type"] ||
      "aero_ptkpp" == info_["type"] ||
      "ocean_ptkpp" == info_["type"] ||
      "fax" == info_["type"] ||
      "cut_ptkpp" == info_["type"]){
    ui_->tabWidget->removeTab(1);
    return true;
  }
  loadDecodedContent(startdata + msg + end, tlgtypes[info_.value("type")], info_.value("converted_dt"));
  return true;

}

bool ContentWidget::loadTextMessage()
{
  if( nullptr == message_ ){
    return false;
  }
  QByteArray startline = QByteArray(message_->startline().data().data(), message_->startline().data().size());
  QByteArray header = QByteArray(message_->header().data().data(), message_->header().data().size());
  QByteArray msg = QByteArray(message_->msg().data(), message_->msg().size());
  QByteArray end = QByteArray(message_->end().data(), message_->end().size());

  QByteArray data =  startline + header + msg + end;
  QString content = replaceContent(data, ui_->checkBox->isChecked());
  ui_->contentEdit->setText(content);
  QString dt = QDateTime::fromString(info_.value("msg_dt"), "yyyy-MM-dd hh:mm:ss").toString(Qt::ISODate);

  if( msg.contains("IONEX") ){
    loadDecodedContent(data, meteo::ALPHANUMGPH, dt);
  }
  else{
    loadDecodedContent(data, tlgtypes[info_.value("type")], dt);
  }
  return true;
}

void ContentWidget::loadDecodedContent(const QByteArray& tlg, meteo::TypeTelegram type, const QString& dt)
{
  if( nullptr != ui_->decodedTab && nullptr == ui_->tabWidget->widget(1)){
    ui_->tabWidget->addTab(ui_->decodedTab, QObject::tr("Сводки"));
  }
  decodeview_->init(tlg, type, dt);
  ui_->stackedWidget->setCurrentWidget(ui_->contentPage);
}

QString ContentWidget::replaceContent(const QByteArray& arr, bool flag)
{
  QString msg = decodeMsg(arr);

  QString text;
  for ( int i = 0, isz = msg.size(); i < isz; ++i ) {
    QChar c = msg.at(i);
    if ( TlgTextObject::isSpecChar(c) ) {
      if ( true == flag ) {
        text += TlgTextObject::kSpecChars.key(c);
      }

      if ( QChar(012) == c ) { text += "\n"; }
    }
    else {
      text += c;
    }
  }
  return text;
}

QString ContentWidget::decodeMsg(const QByteArray& raw) const
{
  QStringList checkList = QStringList()
                          << "МОСКВА"
                          << "РОС"
                          << "TEC"
                          << "КРА"
                          << "ГИДРО"
                          << "МЕТ"
                          << "ГИДРОМЕТ"
                          << "ЦЕНТР"
                          << "ПОГОДА"
                          << "ПРОГНОЗ"
                          << "ДНЕМ"
                          << "ТЕМПЕРАТУРА"
                          << "ВОЗДУХА"
                          << "НОЧЬ";

  QList<QByteArray> codecs = QList<QByteArray>()
                             << "KOI8-R"
                             << "WINDOWS-1251"
                             << "UTF-8"
                                ;

  QString msg;
  foreach ( const QByteArray& n, codecs ) {
    QTextCodec* codec = QTextCodec::codecForName(n);
    msg = codec->toUnicode(raw);
    foreach ( const QString& s, checkList ) {
      if ( msg.contains(s, Qt::CaseInsensitive) ) {
        return msg;
      }
    }
  }
  return msg;
}

void ContentWidget::loadFile()
{
  //TODO если уже раскодировано - прочитать из базы
  meteo::msgcenter::GetTelegramRequest req;
  req.set_onlyheader(false);
  req.set_global_file_id(info_["globalfile_id"].toInt());
  //req.add_msgtype(info_["type"].toStdString());
  QDateTime ddt = QDateTime::fromString(info_["msg_dt"],Qt::ISODate);
  req.set_msg_beg_dt(ddt.toString(Qt::ISODate ).toStdString());
  req.set_msg_end_dt(ddt.addSecs(1).toString(Qt::ISODate ).toStdString());
  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kMsgCenter);
  if ( nullptr == ch ) {
    QApplication::restoreOverrideCursor();
    error_log.msgBox() << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));
    return ;
  }
  meteo::msgcenter::GetTelegramResponse* reply = ch->remoteCall( &meteo::msgcenter::MsgCenterService::GetTelegram, req, 20000 );
  delete ch;
  if ( nullptr == reply ) {
    QApplication::restoreOverrideCursor();
    error_log.msgBox() << meteo::msglog::kServerAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));
    return ;
  }
  extractFiles(reply);
  //TODO запись в базу
  delete reply;
}


bool ContentWidget::extractFiles(meteo::msgcenter::GetTelegramResponse* reply)
{
  ui_->fileOpenPage->setHidden(true);
  QMultiHash<int, meteo::ChainSegment> allSegments_;
  for (int i = 0; i < reply->msg_size(); ++i){
    auto message = reply->msg(i);
    QByteArray content(message.msg().data(), message.msg().size());
    if (content.isEmpty() == true) {
      error_log << QString::fromUtf8("Получено пустое тело телеграммы id = %1")
                   .arg(message.metainfo().id());
      continue;
    }
    meteo::ChainSegment segment(content);
    if ( false == segment.isValid() ){
      error_log << QObject::tr("Ошибка при разборе содержимого сегмента. Возможно сегмент поврежден");
      return false;
    }
    //meteo::faxes::FaxSegment segment = meteo::faxes::Extractor::extractSegment(content);
    allSegments_.insert(segment.formatString().magic(), segment);
  }

  while (false == allSegments_.isEmpty()) {
    int nextFaxId = allSegments_.keys().first();
    auto segments = allSegments_.values(nextFaxId);

    meteo::Chain chain(allSegments_.values());
    const QByteArray& extractedData = chain.tryExtract();
    if ( true == extractedData.isEmpty() ) {
      error_log << QObject::tr("Ошибка при извлечении данных из полученных сегментов");
      return false;
    }
    meteo::FileBuffer buffer(extractedData);
    if ( false == buffer.isValid() ){
      error_log << QObject::tr("Ошибка при разборе буфера, содержащего файл.");
      return false;
    }
    file_name_ = buffer.fileName();
    QTemporaryFile fl(QDir::tempPath() + "/XXXXXX_" + file_name_);
    fl.open();
    file_name_ = fl.fileName();
    ui_->file_open_label->setText(file_name_);
    fl.write(buffer.data());
    fl.setAutoRemove(false);
    fl.close();
    ui_->fileOpenPage->setHidden(false);
    ui_->stackedWidget->setCurrentWidget(ui_->fileOpenPage);
    allSegments_.remove(nextFaxId);
  }
  return true;
}

void ContentWidget::slotSaveFile(){
  QFileInfo fi(file_name_);
  QString name = fi.fileName();
  QString fileName = meteo::FileDialog::getSaveFileName(this,QString::fromUtf8("Выбрать директорию"),QDir::homePath()+"/"+name);
  if ( fileName.isEmpty() ) { return; }

  QFile file(file_name_);
  file.copy(fileName);
}

void ContentWidget::slotOpenFile(){
  QDesktopServices::openUrl(QUrl("file:///"+file_name_, QUrl::TolerantMode));
}

void ContentWidget::slotEditMsg()
{
  if( nullptr == message_ ){
    return;
  }
  QByteArray startline = QByteArray(message_->startline().data().data(), message_->startline().data().size());
  QByteArray header = QByteArray(message_->header().data().data(), message_->header().data().size());
  QByteArray msg = QByteArray(message_->msg().data(), message_->msg().size());
  QByteArray end = QByteArray(message_->end().data(), message_->end().size());
  QByteArray allmsg = startline + header + msg + end;
  meteo::TlgEditorWindow* w = new meteo::TlgEditorWindow();
  w->setRawTlg(allmsg);
  w->show();
}

void ContentWidget::slotSaveMsg()
{
  if( nullptr == message_ ){
    return;
  }
  QString fileName;
  if ( fileName.isEmpty() ) {
    fileName = meteo::FileDialog::getSaveFileName();
  }

  if ( fileName.isEmpty() ) { return; }

  QFile file(fileName);
  if ( !file.open(QFile::WriteOnly | QFile::Truncate) ) {
    error_log << tr("Ошибка при сохранении телеграммы в файл %1").arg(fileName);
    return;
  }
  QByteArray startline = QByteArray(message_->startline().data().data(), message_->startline().data().size());
  QByteArray header = QByteArray(message_->header().data().data(), message_->header().data().size());
  QByteArray msg = QByteArray(message_->msg().data(), message_->msg().size());
  QByteArray end = QByteArray(message_->end().data(), message_->end().size());
  QByteArray raw = startline + header + msg + end;
  //  QByteArray raw = getRawTlg();

  QDataStream out(&file);
  int wsz = out.writeRawData(raw.data(), raw.size());
  if ( wsz != raw.size() ) {
    error_log << tr("Ошибка при записи данных в файл %1").arg(fileName);
  }
}

QImage loadFax( const tlg::Header& header, const QDateTime& hour)
{
  auto serviceCode = settings::proto::kMap;
  auto serviceTitle = global::serviceTitle(serviceCode);
  std::unique_ptr<rpc::Channel> channel(global::serviceChannel(serviceCode));
  if ( nullptr == channel ){
    error_log.msgBox() << msglog::kServiceConnectFailed.arg(serviceTitle);
    return QImage();
  }

  meteo::map::proto::FaxRequest request;
  request.set_header_only(false);
  auto faxGetRequestParam = request.add_faxes();
  faxGetRequestParam->set_t1(header.t1());
  faxGetRequestParam->set_t2(header.t2());
  faxGetRequestParam->set_a1(header.a1());
  faxGetRequestParam->set_a2(header.a2());
  faxGetRequestParam->set_ii(header.ii());
  faxGetRequestParam->set_cccc(header.cccc());
  faxGetRequestParam->set_yygggg(header.yygggg());
  faxGetRequestParam->set_dt_start(hour.toString(Qt::ISODate).toStdString());
  faxGetRequestParam->set_dt_end(hour.toString(Qt::ISODate).toStdString());


  std::unique_ptr<meteo::map::proto::FaxReply>  faxResponce( channel->remoteCall(&meteo::map::proto::DocumentService::GetFaxes, request, 30000 ) );
  if ( nullptr == faxResponce ) {
    error_log.msgBox() << msglog::kServiceAnsverFailed.arg(serviceTitle);
    return QImage();
  }

  QByteArray data_;
  if ( false == faxResponce->result() ){
    error_log.msgBox() << faxResponce->comment();
    return QImage();
  }

  for ( auto fax: faxResponce->faxes() ){
    map::proto::FaxRecord rec = fax;
    rec.clear_msg();

    debug_log << rec.Utf8DebugString();
  }

  if ( 1 != faxResponce->faxes_size() ){
    error_log.msgBox() << QObject::tr("Невозможно отобразить данные. Сообщение получено не полностью.")
                          .arg(faxResponce->faxes_size());
    return QImage();
  }
  auto singleFax = faxResponce->faxes(0);
  if ( false == singleFax.has_msg() ){
    error_log << QObject::tr("Невозможно отобразить данные");
    return QImage();
  }



  QByteArray data = QByteArray::fromStdString(singleFax.msg());

  return QImage::fromData(data,"TIFF");
}


void ContentWidget::loadFaxImage()
{
  if ( 0 == info_["decoded"].compare("false") ){
    return;
  }

  QString dt = QString::fromStdString(message_->metainfo().converted_dt());
  QImage image = loadFax(message_->header(), QDateTime::fromString(dt, Qt::ISODate));
  ui_->stackedWidget->setCurrentWidget(ui_->faxViewPage);

  imagePreview_->setImage(image);
  QTimer::singleShot(0, imagePreview_, &meteo::ViewImagePreview::scaleOptimal);

}

void ContentWidget::slotLoadMsg()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui_->contentEdit->clear();
  bool res;
  if("true" == info_.value("bin") || true == message_->isbinary() || "sputnik"==info_.value("type")){
    res = loadBinMessage();
  }
  else{
    res = loadTextMessage();
  }
  if( false == res ){
    ui_->contentEdit->setText( QObject::tr("Нет данных.") );
  }
  QApplication::restoreOverrideCursor();
}

}
