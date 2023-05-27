
#include "oceantablewidget.h"
#include "ui_oceantablewidget.h"
#include "ui_taboceantablewidget.h"

#include <cross-commons/debug/tlog.h>

//#include <meteo/commons/ui/map/view/maphandler.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/filedialogrus.h>
#include <meteo/commons/global/log.h>
#include <commons/textproto/tprototext.h>

#define CONF_FILE QDir::homePath() + "/.meteo/" + "saveoceandiagtable.conf"


namespace meteo
{
namespace odiag
{


TabOceanTableWidget::TabOceanTableWidget(QWidget *parent)
  : QDialog(parent),
  ui_(new Ui::TabOceanTableWidget)
{
  ui_->setupUi(this);
}

TabOceanTableWidget::~TabOceanTableWidget()
{
  //delete ui_;
  //ui_ = 0;
}

//void TabOceanTableWidget::addOceanTable(const QList<zond::PlaceData> & pd ){
//  ui_->tabWidget->clear();
//  for(int i = 0; i <  pd.size(); ++i){
//    odiag::OceanTableWidget * oceanTable = new odiag::OceanTableWidget(this);
//    //mainwindow_->toMdi(aeroTable);
//    //oceanTable->fillOceanTable(pd.at(i));
//    ui_->tabWidget->addTab(oceanTable,pd.at(i).docName());
//  }
//}


OceanTableWidget::OceanTableWidget(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::OceanTableWidget)
{
    ui_->setupUi(this);
    connect(ui_->saveButton, SIGNAL(clicked()), this, SLOT(slotSaveOceanTable()));
    ui_->tableWidget->horizontalHeader()->setSortIndicator(0,  Qt::AscendingOrder);
    readConfig();
}


OceanTableWidget::~OceanTableWidget()
{
}

void OceanTableWidget::fillOceanTable(const QString name, const QList<TMeteoData> &apd)
{
  dname_  = name + "_" + QDateTime::currentDateTimeUtc().toString("dd.MM.yyyy");
  setWindowTitle(dname_);
  QList<TMeteoData> pd = apd;

  int width = 0;
  int allColumnWidth = 0;
  QTableWidget *table = ui_->tableWidget;
  int sortCol = table->horizontalHeader()->sortIndicatorSection();
  Qt::SortOrder sortOrder = table->horizontalHeader()->sortIndicatorOrder();

  table->setSortingEnabled(false);
  table->setColumnCount(4);
  table->setRowCount(pd.size());
  table->setHorizontalHeaderLabels(QStringList() << tr("Глубина, м")
                                    << tr("Скорость звука, м/с")
                                    << tr("Солёность,%")
                                    << tr("Температура,\xC2\xB0""C"));
  QStringList descrname;
  descrname << "zn" << "ff" <<"Sn"<<"Tw";
  QList<TMeteoData>::const_iterator itb = pd.begin();
  QList<TMeteoData>::const_iterator ite = pd.end();
  int curRow = 0;
  double value = -999.;
  QString str;
  QColor backColor = Qt::white;
  for (; itb != ite; ++itb) {
      for (int i = 0; i < 4; ++i) {
          backColor = Qt::white;
          TMeteoData  mdata = *itb;
          descr_t descr = TMeteoDescriptor::instance()->descriptor(descrname[i]);
          value = mdata.getParam(descr).value();
          if (mdata.getParam(descr).quality() == control::AUTO_CORRECTED) {
            backColor = qRgb(190, 230, 200);
          }
          QTableWidgetItem *item = new TableWidgetNumberItem();
          if (value == -9999.) {
            str = "-";
          } else {
            str = QString::number(value, 'f', 1);
          }
          item->setText(str);
          item->setBackground(backColor);
          table->setItem(curRow, i, item);

      }
      ++curRow;
  }

  table->setSortingEnabled(true);
  table->sortByColumn(sortCol, sortOrder);
  ui_->tableWidget->horizontalHeader()->setSortIndicatorShown(true);

  table->resizeColumnsToContents();
  for (int i = 0; i < 4; ++i) {
      allColumnWidth = allColumnWidth + table->columnWidth(i);
  }
  width = allColumnWidth + 80;
//// debug_log << "!!!!!!!!!!!!!! width = " << width;
  resize(width, frameSize().height());
  setMaximumWidth(width);
}



void OceanTableWidget::slotSaveOceanTable()
{
  QString fileName = FileDialog::getSaveFileName(this, tr("Сохранение таблицы данных батиметрии"),
                                                 QString::fromStdString(conf_.path()) + QObject::tr("/Данные батиметрии_") +dname_ + ".txt", 
						 tr("Текст (*.txt)"));

  if (fileName.isEmpty()) {
    return;
  }

  QString defaultDir = fileName.left(fileName.lastIndexOf("/") + 1);
  if (fileName.endsWith(".txt") == false) {
    fileName += ".txt";
  }

  QFile file(fileName);
  if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
    QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
			  msglog::kFileOpenError.arg(fileName).arg(file.errorString()));
    return;
  }
  
  QTextStream stream(&file);
  QTableWidget *table = ui_->tableWidget;
  QString value;
  stream << tr("Глубина, м") << "\t"
   << tr("Скорость звука, м/с") << "\t"
   << tr("Солёность,%") << "\t"
   << tr("Температура,\xC2\xB0""C") << "\t" <<"\n";
  
  for (int i = 0; i < table->rowCount(); ++i) {
    for (int k = 0; k < table->columnCount(); ++k) {
      if (k != 0) stream << "\t";
      value = table->item(i, k)->text();
      stream << value;
      //debug_log << "item " << i << ", " << k << " = " << value;
    }
    stream << "\n";
  }
  
  file.close();
  
  saveConfig(defaultDir);
}


void OceanTableWidget::saveConfig(const QString& dir)
{
  conf_.set_path(dir.toStdString());
  conf_.set_save_fs(true);
  conf_.set_save_db(false);

  QString text = TProtoText::toText(conf_);
  if (text.isEmpty()) return;


  QFile file(CONF_FILE);
  if ( !file.open(QIODevice::WriteOnly) ) {
    return;
  }
  file.write(text.toUtf8());
  file.close();
}

void OceanTableWidget::readConfig()
{
  conf_.set_path(QDir::homePath().toStdString());
  
  QFile file(CONF_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    return;
  }
  
  QString text = QString::fromUtf8(file.readAll());
  file.close();
  
  meteo::map::exportmap::ExportPath p;
  if ( !TProtoText::fillProto(text, &p) ) {
    file.remove(CONF_FILE);
  } else {
    conf_ = p;
  }
}


}
}
