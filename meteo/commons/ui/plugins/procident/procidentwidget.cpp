
#include "procidentwidget.h"
#include "ui_procidentwidget.h"
#include "fielddescrwidget.h"

#include <cross-commons/debug/tlog.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/global/gradientparams.h>



namespace meteo {
namespace map {

  enum {
    Num            = 0,
    Hour           = 1,
    Season         = 2,
    Korr           = 3
  };


  ProcIdentWidget::ProcIdentWidget(MapView* view,obanal::TField *field, int type) :
  QDialog(view),
  ui_(new Ui::ProcIdentWidget),
  view_(view),
  field_(field),
  document_(nullptr),
  type_(type)
{
  ui_->setupUi(this);
  //ui_->timeBox->setCurrentIndex(8);
  connect(ui_->delBtn, SIGNAL(clicked()), SLOT(slotDelField()));
  connect(ui_->editBtn, SIGNAL(clicked()), SLOT(slotEditField()));
  connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotAddField()));
  connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(slotClose()));
  connect(ui_->searchBtn, SIGNAL(clicked()), SLOT(slotSearch()));
  connect(ui_->selectRegBtn, SIGNAL(toggled(bool)), this, SLOT(slotSelectRegion(bool)));
  connect(ui_->fieldsTree, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this, SLOT(slotEditField()));
  connect(ui_->fieldsTree, SIGNAL(itemSelectionChanged (  )), this, SLOT(slotTreeChanged()));

  QStringList lst;
  lst << "Номер" << "Время" << "Сезон";
  if(1 == type_ ){
    lst << "Корреляция";
  }
  ui_->fieldsTree->setHeaderLabels(lst);
  ui_->fieldsTree->setRootIsDecorated(false);
  ui_->fieldsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  fillTree();
}

ProcIdentWidget::~ProcIdentWidget()
{
  SelectActionIface::removeAction();
  QApplication::restoreOverrideCursor();

}

void ProcIdentWidget::slotSearch(){
  if(  nullptr == field_ ) { return ; }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  meteo::rpc::Channel* ctrl_field_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field_) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return;
  }
  meteo::field::SimpleDataRequest request;
  meteo::field::ProcessFieldsData* reply = ctrl_field_->remoteCall( &meteo::field::FieldService::GetProcessFields, request, 30000);
  delete ctrl_field_; ctrl_field_=nullptr;
  QApplication::restoreOverrideCursor();
  if ( nullptr == reply ) {
    error_log.msgBox() << msglog::kServiceInfoAnswerFailed.arg("данные").arg("полей");
    delete reply;
    return ;
  }
  int ds = reply->data_size();
  if (1 > ds) {
    delete reply;
    return ;
  }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  for (int i = 0; i < ds; ++i){
    const meteo::field::ProcessFieldData & fd = reply->data(i);
    if (!fd.descr().has_process_id() || !fd.descr().has_season()|| !fd.descr().has_hour()
      || !fd.descr().has_fnumber() || !fd.descr().has_process_name()) continue;

  //  int field_id = fd.descr().field_id();

    obanal::TField* field = new obanal::TField;

    QByteArray arr( fd.fielddata().data(), fd.fielddata().size() );
    if(false == field->fromBuffer(&arr)){
      delete reply; reply = nullptr;
      QApplication::restoreOverrideCursor();
      warning_log.msgBox() <<  msglog::kServiceRequestInvalid.arg(meteo::global::serviceTitle(settings::proto::kField));
      return;
    }
  }
  delete reply;
  return;
}
/*
fftw_complex * ProcIdentWidget::prepField(const obanal::TField *im, int ny, int nx){

  int ny_part = im->kolFi();//im.kolfi();
  int nx_part = im->kolLa();//im.kolla();

  int koldata = ny*nx;
  fftw_complex *fftw = new fftw_complex[koldata];

  double mo = 0.;
  //im->oprMatOz(&mo);

  for (int i = 0; i < koldata; i++){
    fftw[i].re = 0.;
    fftw[i].im = 0.;
  }

  double her = 0;
  for   (int y = 0; y < ny_part; ++y){
    for (int x = 0; x < nx_part; ++x){
      int ii_fftw = y*nx+x;
      if(!im->getMask(y,x)) continue;
      fftw[ii_fftw].re = (im->getData(y,x))-mo;
      her +=fftw[ii_fftw].re;
      fftw[ii_fftw].im = 0.;
    }
  }

  qDebug()<<"her"<<her;

  return fftw;
}

void ProcIdentWidget::findMaxCor(obanal::TField *srcfield,obanal::TField *afield){
  qDebug()<<"start!";
  int df = afield->kolFi() - 1;
  int dl = afield->kolLa() - 1;
  int kolfi = srcfield->kolFi()+df;// image.kolfi();
  int kolla = srcfield->kolLa()+dl;//image.kolla();

  qDebug()<<"w1"<<kolfi<<kolla;
  fftw_complex *pattern_in = prepField(afield,kolfi,kolla);
  fftw_complex *src_in = prepField(srcfield,kolfi,kolla);
  visualizeFFT(pattern_in, kolla, kolfi, "/home/gotur/tmp/test/small.bmp");
  visualizeFFT(src_in, kolla, kolfi, "/home/gotur/tmp/test/big.bmp");

  qDebug()<<"w3"<<kolfi<<kolla;
  int koldata = kolfi*kolla;
  fftw_complex *her = new fftw_complex[koldata];
  fftw_complex *her1 = new fftw_complex[koldata];
  fftwnd_plan p_f;
  p_f = fftw2d_create_plan( kolfi,kolla, FFTW_FORWARD, FFTW_ESTIMATE|FFTW_OUT_OF_PLACE);
  fftwnd_one(p_f, src_in, her);
  fftwnd_one(p_f, pattern_in, her1);
  visualizeFFT(her,  kolla, kolfi, "/home/gotur/tmp/test/her.bmp");
  visualizeFFT(her1, kolla, kolfi, "/home/gotur/tmp/test/her1.bmp");


  delete []src_in;
  delete []pattern_in;
  fftwnd_destroy_plan(p_f);
  fftw_complex *XY = new fftw_complex[koldata];

  for (int i = 0; i < koldata; ++i){
    //скалярное произведение
    XY[i].re =     her[i].re*her1[i].re + her[i].im*her1[i].im;
    XY[i].im =     her[i].re*her1[i].im + -1.*her[i].im*her1[i].re;
    float tmp =    sqrtf( XY[i].re*XY[i].re + XY[i].im*XY[i].im);
    if( MnMath::isZero(tmp)){
      tmp =1.;
     // debug_log << "ALARM!" << i;
    }
    XY[i].re = XY[i].re/ tmp/float(koldata);
    XY[i].im = XY[i].im/tmp/float(koldata);
  }
  delete []her;

  fftwnd_plan p_b;
  p_b = fftw2d_create_plan(kolfi,kolla, FFTW_BACKWARD, FFTW_ESTIMATE|FFTW_OUT_OF_PLACE);
  //p_b = fftw2d_create_plan(height,width, FFTW_BACKWARD, FFTW_ESTIMATE|FFTW_OUT_OF_PLACE);
  fftwnd_one(p_b, XY, her1);
  visualizeFFT(her1, kolla, kolfi, "/home/gotur/tmp/test/sp.bmp");

  int maxi=-1;
  int maxf=-1;
  int maxl=-1;
  float max = -1e10;
  float maxV = -1e10;
  float min = 1e20;
  float minV = 1e20;

  for(int i =1; i < kolfi-1-df;++i){
    for(int j =1; j <  kolla-1-dl; ++j){
      int ii_fftw = i*kolla+j;
      float M = sqrt(pow(her1[ii_fftw].re, 2.) + pow(her1[ii_fftw].im, 2.));///132537.*256.*256./6.11914e+12;
      ii_fftw = (i-1)*kolla+j;
      M += sqrt(pow(her1[ii_fftw].re, 2.) + pow(her1[ii_fftw].im, 2.));///132537.*256.*256./6.11914e+12;
      ii_fftw = (i+1)*kolla+j;
      M += sqrt(pow(her1[ii_fftw].re, 2.) + pow(her1[ii_fftw].im, 2.));///132537.*256.*256./6.11914e+12;
      ii_fftw = (i)*kolla+j+1;
      M += sqrt(pow(her1[ii_fftw].re, 2.) + pow(her1[ii_fftw].im, 2.));///132537.*256.*256./6.11914e+12;
      ii_fftw = (i)*kolla+j-1;
      M += sqrt(pow(her1[ii_fftw].re, 2.) + pow(her1[ii_fftw].im, 2.));///132537.*256.*256./6.11914e+12;
      if(max < M ){
        max = M;
        maxV = M;
        maxi= ii_fftw;
        maxf= i;
        maxl= j;
      }
      if(min > M ){
        min = M;
        minV = M;
      }
    }
  }
  delete []her1;
  fftwnd_destroy_plan(p_b);

  qDebug()<< maxi<<maxf<<srcfield->getFi(maxf)*180./M_PI <<maxl<<srcfield->getLa(maxl)*180./M_PI<<maxV<<minV;
  qDebug()<<"end!";
}



  void ProcIdentWidget::visualizeFFT(fftw_complex* fft,  int WIDTH,  int HEIGHT, const QString & path) {
    QImage visualImage(WIDTH, HEIGHT, QImage::Format_RGB32);

    // Find maximum
    float maxAmpl = 0;
    float curAmpl = 0;
    for (int i=0; i<WIDTH*HEIGHT; i++) {
      // Extract Amplitude
      curAmpl = sqrt(pow(fft[i].re, 2) + pow(fft[i].im, 2));
      curAmpl = log(1 + curAmpl);
      if (curAmpl > maxAmpl) {
        maxAmpl = curAmpl;
      }
    }
    float maxAmpl_vs = -1.e10;
    int mi=0,mj=0;
    // Build image
    for(int y=0; y<HEIGHT; y++) {
      for(int x=0; x<WIDTH; x++) {
        // Normalize
        curAmpl = sqrt(pow(fft[y*WIDTH+x].re, 2) + pow(fft[y*WIDTH+x].im, 2));
        // Log scale
        curAmpl = 255 * log(1 + curAmpl) / maxAmpl;

        if (curAmpl > maxAmpl_vs) {
          maxAmpl_vs = curAmpl;
          mi=y;mj=x;

        }

        visualImage.setPixel(x, y, qRgb(curAmpl, curAmpl, curAmpl));
      }
    }

    qDebug()<<mi<<mj<<maxAmpl_vs;
    visualImage.save(path);
  }

  */


void ProcIdentWidget::fillTree(){
  return;
  procList_.clear();
  if( nullptr == ui_ || nullptr == ui_->fieldsTree) return ;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  meteo::field::SimpleDataRequest request;
  request.add_id(field_->dbId().toStdString().c_str(), field_->dbId().length());
  meteo::rpc::Channel* ctrl_field_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field_) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return;
  }
  meteo::field::ProcessFieldsData* reply = ctrl_field_->remoteCall( &meteo::field::FieldService::GetProcessFieldsKorr, request, 30000);
  delete ctrl_field_; ctrl_field_=nullptr;
  if ( nullptr == reply ) {
    error_log.msgBox() << msglog::kServiceInfoAnswerFailed.arg("данные").arg("полей");
    delete reply;
    QApplication::restoreOverrideCursor();
    return ;
  }
  int ds = reply->data_size();
  if (1 > ds) {
    delete reply;
    QApplication::restoreOverrideCursor();
    return ;
  }

  ui_->fieldsTree->clear();

  for (int i = 0; i < ds; ++i){
    const meteo::field::ProcessFieldData & fd = reply->data(i);
    if (!fd.descr().has_process_id() || !fd.descr().has_season()|| !fd.descr().has_hour()
      || !fd.descr().has_fnumber() || !fd.descr().has_process_name()) continue;
   // int proc_num = fd.descr().process_id();
    QString field_id = QString::fromStdString( fd.descr().field_id() );
    QString season =QString::fromStdString( fd.descr().season());
    int fnumber = fd.descr().fnumber();
    QString hour = "+"+QString::number(fd.descr().hour())+"ч.";
    QString procname = QString::fromStdString(fd.descr().process_name());
    QString korr = QString();
    if(fd.has_koef_kor()){
      korr = QString::number(fd.koef_kor(),'f',2);
    }
    QTreeWidgetItem * cur_top_item = nullptr;
    for(int j =0; j < ui_->fieldsTree->topLevelItemCount(); ++j){
      cur_top_item = ui_->fieldsTree->topLevelItem(j);
      if(nullptr != cur_top_item){
        if(0 == cur_top_item->text(0).compare(procname,Qt::CaseSensitive)){

          break;

        }
      }
      cur_top_item = nullptr;
    }
    QTreeWidgetItem * cur_item = nullptr;

    if(nullptr == cur_top_item){
      cur_top_item = new QTreeWidgetItem((QTreeWidget*)nullptr);
      cur_top_item->setText(0,procname);
      ui_->fieldsTree->addTopLevelItem(cur_top_item);
      procList_.append(procname);
     // debug_log << procname;
    }
    cur_item = new QTreeWidgetItem((QTreeWidget*)nullptr);
    cur_item->setText(Num,QString::number(fnumber));
    cur_item->setText(Hour,hour);
    cur_item->setData(Hour,Qt::UserRole,fd.descr().hour());
    cur_item->setText(Season,season);
    cur_item->setData(Num,Qt::UserRole, field_id);
    if(!korr.isNull()) cur_item->setText(Korr,korr);

    cur_top_item->addChild(cur_item);
  }

  ui_->fieldsTree->setSelectionMode(QAbstractItemView::SingleSelection);
  ui_->fieldsTree->setAllColumnsShowFocus(true);

  if(0 < ui_->fieldsTree->topLevelItemCount()){
      ui_->fieldsTree->topLevelItem(0)->setSelected(true);
  }

  delete reply;
  ui_->fieldsTree->expandAll();
  QApplication::restoreOverrideCursor();

}

void ProcIdentWidget::setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect)
{
  Q_UNUSED(nw);
  Q_UNUSED(ne);
  Q_UNUSED(sw);
  Q_UNUSED(se);
  Q_UNUSED(rect);
  reg_par_.start = sw;
  reg_par_.end = ne;
  mapBand_ = rect;
}


bool ProcIdentWidget::getProcField(obanal::TField *field){
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  int kolfi = field->kolFi();
  int kolla = field->kolLa();

  GeoPoint gpnt;
  for(int i =0; i < kolfi;++i){
    for(int j =0; j < kolla;++j){
      if(field->getMask(i,j)) {
        gpnt.setFi(field->getFi(i));
        gpnt.setLa(field->getLa(j));
        if(!mapBand_.contains(document_->coord2screen(gpnt))){
          field->setMasks(field->num(i,j),false);
        }
      }
    }
  }

  QApplication::restoreOverrideCursor();
  return true;
}

void ProcIdentWidget::test(obanal::TField *field){
  if (nullptr==field) return ;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  LayerIso* ll = nullptr;
  ll = new LayerIso( document_, "Процесс" );
  ll->setField(field);
  int iso_count = ll->addIsoLines();
  if(0 < iso_count){
    ll->addExtremums();
    //ll->setShotName(QString("<b>%1, %2</b>").arg(field->name() ).arg(field->unit())); //item->text(Level)
    ll->setName(QString("<b>%1, %2</b>").arg(field->name() ).arg(field->unit())); //item->text(Level)
  }
  QApplication::restoreOverrideCursor();
}


void ProcIdentWidget::slotAddField(){
  emit addRect(mapBand_ );

  QTreeWidgetItem * curitem = ui_->fieldsTree->currentItem();
  QString procname = "Процесс";
  QString season = "Лето";
  QString hour = "0";
  QString fnumber = "0";

  if(nullptr != curitem){
    QTreeWidgetItem * topitem =   curitem->parent();
    if(topitem){
      procname = topitem->text(Num);
      fnumber = QString::number(curitem->text(Num).toInt()+1);
      season = curitem->text(Season);
      hour = QString::number(curitem->data(Hour,Qt::UserRole).toInt()+3);
    }
  }

  FieldDescrWidget *dlg = new FieldDescrWidget(view_);
  dlg->fillProcessList(procList_);
  dlg->setProcName(procname);
  dlg->setFnum(fnumber);
  dlg->setSeason(season);
  dlg->setHour(hour);
  dlg->exec();

  if(false == dlg->is_ok_){
    delete dlg;
    return;
  }

  QString proc_name;
  int num_proc = 0;
  if(-1 == (num_proc = dlg->getProc(&proc_name))){
    debug_log << "не выбран тип процесса";
    delete dlg;
    return;
  }

  if(  nullptr == field_
    || nullptr == document_ ) {
    return ;
  }
  obanal::TField *field = field_->getCopy();
  if(!getProcField(field)) return ;
 /// test(field);
 // findMaxCor(field_,field);
 // return;
 // test(field);
  ::meteo::field::ProcessFieldData request;
  ::meteo::field::ProcessFieldDesc *fdesc = request.mutable_descr();

  fdesc->set_process_id(num_proc);//!< номер процесса
  fdesc->set_process_name(proc_name.toStdString());//!<  процесса
  fdesc->set_season(dlg->season().toStdString());//!< сезон 0 - зима, 1 - весна, 2 - лето, 3 - осень
  fdesc->set_fnumber(dlg->numField());//!< номер поля для процесса
  fdesc->set_net_type(field->typeNet());
  fdesc->set_level(field->getLevel());
  fdesc->set_type_level(field->getLevelType());
  fdesc->set_fparam(field->getDescr());
  fdesc->set_hour(dlg->hour());

  QByteArray ba;
  field->getBuffer(&ba);
  request.set_fielddata(ba.data(), ba.size());
 // fdesc->set_field_id();
  saveProcessField(request);
 // delete field; field = 0;
  fillTree();
}


void ProcIdentWidget::slotEditField(){

  QTreeWidgetItem * curitem = ui_->fieldsTree->currentItem();
  QString procname = "Процесс";
  QString season = "Лето";
  QString hour = "0";
  QString fnumber = "0";
  QString field_id = "0";

  if(nullptr == curitem){return;}
  QTreeWidgetItem * topitem =   curitem->parent();
  if(nullptr == topitem){return;}

  procname = topitem->text(Num);
  fnumber = QString::number(curitem->text(Num).toInt());
  season = curitem->text(Season);
  hour = QString::number(curitem->data(Hour,Qt::UserRole).toInt());
  field_id = curitem->data(Num, Qt::UserRole).toString();

  FieldDescrWidget *dlg = new FieldDescrWidget(view_);
  dlg->fillProcessList(procList_);
  dlg->setProcName(procname);
  dlg->setFnum(fnumber);
  dlg->setSeason(season);
  dlg->setHour(hour);
  dlg->exec();
  if(false == dlg->is_ok_){
    delete dlg;
    return;
  }

  int num_proc = 0;
  if(-1 == (num_proc = dlg->getProc(&procname))){
    debug_log << "не выбран тип процесса";
    delete dlg;
    return;
  }

  ::meteo::field::ProcessFieldData request;
  ::meteo::field::ProcessFieldDesc *fdesc = request.mutable_descr();
  fdesc->set_field_id(field_id.toStdString().c_str(), field_id.toStdString().length());//!< номер процесса
  fdesc->set_process_name(procname.toStdString());//!<  процесса
  fdesc->set_season(dlg->season().toStdString());//!< сезон 0 - зима, 1 - весна, 2 - лето, 3 - осень
  fdesc->set_fnumber(dlg->numField());//!< номер поля для процесса
  fdesc->set_hour(dlg->hour());

  editProcessField(request);
  fillTree();
  delete dlg;
}

void ProcIdentWidget::slotDelField(){

  QTreeWidgetItem * curitem = ui_->fieldsTree->currentItem();
  QString field_id = "0";

  if(nullptr == curitem){return;}
  field_id = curitem->data(Num, Qt::UserRole).toString();
  int ret = QMessageBox::warning(this, QObject::tr("Внимание!"),
                                 QObject::tr("Вы уверены, что хотите удалить выбранное поле?"),
                                 QMessageBox::Ok | QMessageBox::Cancel);
  if(QMessageBox::Ok ==  ret ){
    deleteProcessField(field_id.toInt());
    fillTree();
  }
}

void ProcIdentWidget::slotTreeChanged(){
  QTreeWidgetItem * curitem = ui_->fieldsTree->currentItem();
  if(nullptr == curitem){
    ui_->editBtn->setDisabled(true);
    ui_->delBtn->setDisabled(true);
    ui_->addBtn->setDisabled(false);
    return;
  }
  if(nullptr != curitem->parent()){
    ui_->editBtn->setDisabled(false);
    ui_->delBtn->setDisabled(false);
    ui_->addBtn->setDisabled(false);
  } else {
    if(0 == curitem->childCount()){
      ui_->editBtn->setDisabled(true);
      ui_->delBtn->setDisabled(false);
      ui_->addBtn->setDisabled(false);
    }else {
      ui_->editBtn->setDisabled(true);
      ui_->delBtn->setDisabled(true);
      ui_->addBtn->setDisabled(false);
    }
  }

}

void ProcIdentWidget::slotClose(){
  close();
}


void ProcIdentWidget::closeEvent(QCloseEvent *event)
{
  SelectActionIface::removeAction();
  QApplication::restoreOverrideCursor();

    event->accept();
 //   event->ignore();
}

void ProcIdentWidget::slotSelectRegion(bool state){

  if (state == true) {
    SelectActionIface::removeAction(false);
    SelectActionIface::createAction(view_->mapscene(), QRect());

  } else {
    SelectActionIface::removeAction();
    //fillCoordsFromDocument();
    QApplication::restoreOverrideCursor();
  }
}

bool ProcIdentWidget::saveProcessField(const ::meteo::field::ProcessFieldData& request){

  meteo::rpc::Channel* ctrl_field_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field_) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return false;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  meteo::field::SimpleDataReply* reply = ctrl_field_->remoteCall( &meteo::field::FieldService::AddProcessField, request, 30000);
  delete ctrl_field_; ctrl_field_=nullptr;
  QApplication::restoreOverrideCursor();
  if ( nullptr == reply ) {
    error_log.msgBox() << msglog::kServiceInfoAnswerFailed.arg("данные").arg("полей");
    delete reply;
    return -1;
  }
  if (!reply->has_id()) {
    delete reply;
    return false;
  }

  delete reply;

  return true;
}

bool ProcIdentWidget::editProcessField(const ::meteo::field::ProcessFieldData& request){

  meteo::rpc::Channel* ctrl_field_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field_) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return false;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  meteo::field::SimpleDataReply* reply = ctrl_field_->remoteCall( &meteo::field::FieldService::EditProcessField, request, 30000);
  delete ctrl_field_; ctrl_field_= nullptr;
  QApplication::restoreOverrideCursor();
  if ( nullptr == reply ) {
    error_log.msgBox() << msglog::kServiceInfoAnswerFailed.arg("данные").arg("полей");
    delete reply;
    return false;
  }
  delete reply;

  return true;
}

bool ProcIdentWidget::deleteProcessField(int fid){

  meteo::rpc::Channel* ctrl_field_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field_) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return false;
  }

  ::meteo::field::SimpleDataRequest request;
  request.add_id(QString::number(fid).toStdString().c_str(), QString::number(fid).length());
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  meteo::field::SimpleDataReply* reply = ctrl_field_->remoteCall( &meteo::field::FieldService::DeleteProcessField, request, 30000);
  delete ctrl_field_; ctrl_field_=nullptr;
  QApplication::restoreOverrideCursor();

  if ( nullptr == reply ) {
    error_log.msgBox() << msglog::kServiceInfoAnswerFailed.arg("данные").arg("полей");
    delete reply;
    return false;
  }

  if (!reply->has_id()) {
    delete reply;
    return false;
  }

  delete reply;

  return true;
}



}
}
