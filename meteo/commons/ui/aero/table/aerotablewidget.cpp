
#include "aerotablewidget.h"
#include "ui_aerotablewidget.h"
#include "ui_tabaerotablewidget.h"

#include <cross-commons/debug/tlog.h>

//#include <meteo/commons/ui/map/view/maphandler.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/filedialogrus.h>
#include <meteo/commons/global/log.h>
#include <commons/textproto/tprototext.h>

#define CONF_FILE QDir::homePath() + "/.meteo/" + "saveadtable.conf"


namespace meteo
{
namespace aero
{


TabAeroTableWidget::TabAeroTableWidget(QWidget *parent)
  : QDialog(parent),
  ui_(new Ui::TabAeroTableWidget)
{
  ui_->setupUi(this);
}

TabAeroTableWidget::~TabAeroTableWidget()
{
  //delete ui_;
  //ui_ = 0;
}

void TabAeroTableWidget::addAeroTable(const QList<zond::PlaceData> & pd ){
  ui_->tabWidget->clear();
  for(int i = 0; i <  pd.size(); ++i){
    aero::AeroTableWidget * aeroTable = new aero::AeroTableWidget(this);
    //mainwindow_->toMdi(aeroTable);
    aeroTable->fillAeroTable(pd.at(i));
    ui_->tabWidget->addTab(aeroTable,pd.at(i).docName());
  }
}


AeroTableWidget::AeroTableWidget(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::AeroTableWidget)
{
    ui_->setupUi(this);
    connect(ui_->saveButton, SIGNAL(clicked()), this, SLOT(slotSaveAeroTable()));
    ui_->tableWidget->horizontalHeader()->setSortIndicator(0,  Qt::AscendingOrder);
    readConfig();
}


AeroTableWidget::~AeroTableWidget()
{
}

void AeroTableWidget::fillAeroTable(const zond::PlaceData &apd)
{
  dname_  = apd.docName();
  setWindowTitle(dname_);
    //debug_log << "fillAeroTable()";
  zond::PlaceData pd = apd;

  int width = 0;
  int allColumnWidth = 0;
  QTableWidget *table = ui_->tableWidget;
  int sortCol = table->horizontalHeader()->sortIndicatorSection();
  Qt::SortOrder sortOrder = table->horizontalHeader()->sortIndicatorOrder();

  table->setSortingEnabled(false);
  table->setColumnCount(7);
  table->setRowCount(pd.zond().urovenList().size());
  table->setHorizontalHeaderLabels(QStringList() << tr("P,гПа")
                                    << tr("H,м")
                                    << tr("T,\xC2\xB0""C")
                                    << tr("Td,\xC2\xB0""C")
                                    << tr("D,\xC2\xB0""C")
                                    << tr("dd,\xC2\xB0")
                                    << tr("ff,м/с"));

  QMap<zond::Level, zond::Uroven>::const_iterator itb = pd.zond().urovenList().begin();
  QMap<zond::Level, zond::Uroven>::const_iterator ite = pd.zond().urovenList().end();
  int curRow = 0;//pd.zond.urovenList().size();
  zond::Uroven ur;
  pd.zond().getUrz(&ur);
  zond::ValueType urType;
  double value = -999.;
  QString str;
  QColor backColor = Qt::white;
  QColor typeColor = Qt::white;
  for (; itb != ite; ++itb) {
      //    --curRow;

      QString type;
      int level_type = itb.value().levelType();
      switch( level_type )       //тип уровня
      {
        case 1:
          type =  tr("Земная или водная поверхность");
          typeColor = Qt::green;
        break;
        case 101:
          type =  tr("Средний уровень моря");
          typeColor = Qt::green;
        break;
        case 103:
          type =  tr("Высота над поверхностью");
          typeColor = Qt::green;
        break;

        case 7:
          type =  tr("Уровень nропопаузы");
          typeColor = qRgb(162, 37, 0);
        break;
        case 6:
          type =  tr("Уровень максимального ветра");
          typeColor = Qt::magenta;
        break;
        default:
        case 100:
          type = tr("Изобарическая поверхность");
          typeColor = Qt::white;
        break;
      }
          /*Gr_Tempr         = 4, //!< особые точки  по температуре
          Gr_Wind           = 5, //!< особые точки  по ветру
          Gr_MaxWind_D    = 13, //!< на уровне максимального ветра выше 100
          Gr_Tempr_D         = 14, //!< особые точки  по температуре выше 100
          Gr_Wind_D           = 15, //!< особые точки  по ветру выше 100
          Gr_Standart_C     = 16, //!< на стандартных изобарических поверхностях выше 100
*/
      for (int i = 0; i < 7; ++i) {
        backColor = typeColor;
        urType = static_cast<zond::ValueType>(i);
        value = itb.value().value(urType);
        if (itb.value().isGood(urType)) {
          if (itb.value().quality(urType) == control::AUTO_CORRECTED) {
            backColor = qRgb(190, 210, 200);
          }
        }
        else {
          if (itb.value().isGood(zond::UR_P)) {
            pd.zond().getUrPoP(itb.key(), &ur);
            value = ur.value(urType);
            backColor = qRgb(190, 210, 200);
          }//  else if (itb.value().isGood(zond::UR_H)) {
          // //TODO можно заполнить почти все пропуски. но эти данные не используются при построении. И могут появится уровни с одним P и разными H
          //   pd.zond().getUrPoH(itb.value().value(zond::UR_H), &ur);
          //   value = ur.value(urType);
          //   backColor = qRgb(190, 210, 200);
          // }
        }
        QTableWidgetItem *item = new TableWidgetNumberItem();
        if ( MnMath::isEqual(value,-9999.)) {
          str = "-";
          backColor = Qt::white;
        }
        else {
          str = QString::number(value, 'f', 1);
        }
        item->setText(str);
        item->setBackground(backColor);
        item->setToolTip((type));
        item->setFlags(item->flags()^Qt::ItemIsEditable);

        table->setItem(curRow, i, item);
      }
      ++curRow;
  }
  table->setSelectionBehavior(QAbstractItemView::SelectRows);

  table->setSortingEnabled(true);
  table->sortByColumn(sortCol, sortOrder);
  ui_->tableWidget->horizontalHeader()->setSortIndicatorShown(true);

  table->resizeColumnsToContents();
  for (int i = 0; i < 7; ++i) {
      allColumnWidth = allColumnWidth + table->columnWidth(i);
  }
  width = allColumnWidth + 80;
// debug_log << "!!!!!!!!!!!!!! width = " << width;
  resize(width, frameSize().height());
  setMaximumWidth(width);
}



void AeroTableWidget::slotSaveAeroTable()
{
  QString fileName = FileDialog::getSaveFileName(this, tr("Сохранение таблицы данных зондирования"),
                                                 QString::fromStdString(conf_.path()) + QObject::tr("/Данные зондирования_") +dname_ + ".txt",
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
  stream << tr("P,гПа") << "\t"
         << tr("H,м") << "\t"
         << tr("T,\xC2\xB0""C") << "\t"
         << tr("Td,\xC2\xB0""C") << "\t"
         << tr("D,\xC2\xB0""C") << "\t"
         << tr("dd,\xC2\xB0") << "\t"
         << tr("ff,м/с") << "\n";

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


void AeroTableWidget::saveConfig(const QString& dir)
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

void AeroTableWidget::readConfig()
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
