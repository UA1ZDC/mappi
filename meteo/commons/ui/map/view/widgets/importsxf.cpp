#include "importsxf.h"
#include "ui_importsxf.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/dataexchange/sxftomap.h>
#include <meteo/commons/ui/map/dataexchange/metasxf.h>
#include <meteo/commons/ui/custom/filedialogrus.h>

#include <qmessagebox.h>

#define CONF_FILE QDir::homePath() + "/.meteo/" + "importmap.conf"
#define METEO_RSC_FILE QString(MnCommon::projectPath() + meteo::sxf::MetaSxf::instance()->rscfile())


enum Column
{
  CHECK = 0,
  NUMBER = 1,
  NAME = 2,
  COUNT = 3,
};

using namespace meteo;
using namespace map;


ImportSxf::ImportSxf( MapWindow* parent ):
  MapWidget(parent),
  ui_(new Ui::ImportSxf),
  sxf_(new meteo::SxfToMap)
{
  ui_->setupUi(this);
  //  ui_->warning->hide();

  //  ui_->layersList->hideColumn(NUMBER);
  // setPixmap(QPixmap(":/meteo/icons/map/save.png")); TODO

  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(cancel()));
  connect(ui_->chooseSxfBtn, SIGNAL(clicked()), SLOT(chooseSxf()));
  connect(ui_->chooseRscBtn, SIGNAL(clicked()), SLOT(chooseRsc()));
  connect(ui_->layersList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
	  SLOT(checkLayer(QTreeWidgetItem*, int)));
  connect(ui_->layersList, SIGNAL(itemExpanded(QTreeWidgetItem*)), 
	  SLOT(expandColumns(QTreeWidgetItem*)));
  connect(ui_->layersList, SIGNAL(itemCollapsed(QTreeWidgetItem*)), 
	  SLOT(expandColumns(QTreeWidgetItem*)));
  connect(sxf_, SIGNAL(importedObjects(int, int)), this, SLOT(importedObjects(int, int)));
}


ImportSxf::~ImportSxf()
{ 
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
  if (0 != sxf_) {
    delete sxf_;
    sxf_ = 0;
  }
}

void ImportSxf::setOptions(const QString& options)
{
  Q_UNUSED(options);
}

Document* ImportSxf::currentDocument() const
{
  if (mapview() != 0 &&
      mapview()->mapscene() != 0 &&
      mapview()->mapscene()->document() != 0) {
    return mapview()->mapscene()->document();
  }
  
  return 0;
}

void ImportSxf::readConf() 
{
  conf_.set_path(QDir::homePath().toStdString());
  conf_.set_loadpath(METEO_RSC_FILE.toStdString());

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


void ImportSxf::saveConf() const
{
  QString text = TProtoText::toText(conf_);
  if (text.isEmpty()) return;
  
  
  QFile file(CONF_FILE);
  if ( !file.open(QIODevice::WriteOnly) ) {
    return;
  }
  file.write(text.toUtf8());
  file.close();
}

void ImportSxf::clear()
{
  ui_->layersList->clear();
  sxf_->clear();
  ui_->sxfFile->clear();
  ui_->rscFile->clear();
  ui_->loadedBar->setValue(1);
  ui_->loadedBar->setMaximum(1);
  ui_->loadedBar->reset();
  //  ui_->warning->hide();
  ui_->mapName->clear();
  ui_->mapDt->clear();
  ui_->objectsCnt->clear();
}

void ImportSxf::cancel()
{
  clear();
  MapWidget::close();
}

void ImportSxf::keyPressEvent( QKeyEvent * e)
{
 if (e->key() == Qt::Key_Escape) {
   clear();
   MapWidget::keyPressEvent(e);
 }
}

void ImportSxf::closeEvent(QCloseEvent *e)
{
  clear();
  MapWidget::closeEvent(e);
}

//! Выбор слоя при нажатии на любое место строки
void ImportSxf::checkLayer(QTreeWidgetItem* item, int column)
{
  if (item != 0) {
    if (!item->text(COUNT).isEmpty() || 0 == item->parent()) {
      if (column != CHECK) {
	Qt::CheckState currentState = item->checkState(CHECK);
	if (currentState == Qt::Checked) {
	  item->setCheckState(CHECK, Qt::Unchecked);
	} else {
	  item->setCheckState(CHECK, Qt::Checked);
	}
      }

      QTreeWidgetItem* parent = item->parent();
      if (0 != parent) {
	if (item->checkState(CHECK) == Qt::Checked) {
	  parent->setCheckState(CHECK, Qt::Checked);
	} else {
	  bool isAnyChecked = false; //хоть один потомок отмечен
	  for (int i = 0; i < parent->childCount(); i++) {
	    if (parent->child(i)->checkState(CHECK) == Qt::Checked) {
	      isAnyChecked = true;
	      break;
	    }
	  }
	  if (!isAnyChecked) {
	    parent->setCheckState(CHECK, Qt::Unchecked);
	  }
	}
      }

      for (int i = 0; i < item->childCount(); i++) {
	if (!item->child(i)->text(COUNT).isEmpty()) {
	  item->child(i)->setCheckState(CHECK, item->checkState(CHECK));
	  if (0 == item->parent()) {
	    for (int j = 0; j < item->child(i)->childCount(); j++) {
	      if (!item->child(i)->child(j)->text(COUNT).isEmpty()) {
		item->child(i)->child(j)->setCheckState(CHECK, item->checkState(CHECK));
	      }
	    }
	  }
	}
      }
    }
  }
}


void ImportSxf::expandColumns(QTreeWidgetItem* /*item*/)
{
  for (int i = 0, sz = ui_->layersList->columnCount(); i < sz; ++i) {
    ui_->layersList->resizeColumnToContents(i);
  }
}

void ImportSxf::chooseSxf()
{
  QString caption = QString::fromUtf8("Выбор файла карты");
  if (!conf_.has_path()) {
    readConf();
  }
  QString filter = QString::fromUtf8("Файл в формате SXF (*.sxf)");

  QString fileName = FileDialog::getOpenFileName(this, caption,
						 QString::fromStdString(conf_.path()),
						 filter);
  
  if (!fileName.isEmpty()) {
    conf_.set_path(fileName.left(fileName.lastIndexOf("/") + 1).toStdString());
    ui_->sxfFile->setText(fileName);  
    if (readSxfDescription()) {
      fillLayers();
      if (ui_->rscFile->text().isEmpty()) {
	chooseRsc();
      }
    }
  } else if (ui_->sxfFile->text().isEmpty()) {
    ui_->applyButton->setEnabled(false);
  }
}

void ImportSxf::chooseRsc()
{
  QString caption = QString::fromUtf8("Выбор файла классификатора");
  if (!conf_.has_loadpath()) {
    readConf();
  }
  QString filter = QString::fromUtf8("Файл в формате RSC (*.RSC)");
  
  QString rscconf = QString::fromStdString(conf_.loadpath());
  QString fileName = FileDialog::getOpenFileName(this, caption,
						 rscconf,
						 //rscconf.left(rscconf.lastIndexOf("/") + 1),
						 filter);
  
  if (!fileName.isEmpty()) {
    // conf_.set_loadpath(fileName.left(fileName.lastIndexOf("/") + 1).toStdString());
    conf_.set_loadpath(fileName.toStdString());
    ui_->rscFile->setText(fileName);  
    QByteArray rscData;
    if (readFile(ui_->rscFile->text(), &rscData)) {
      if (sxf_->setRscData(rscData)) {
	fillLayers();
      } else {
	QMessageBox::critical(this, QObject::tr("Ошибка проверки файла"), 
			      QObject::tr("Ошибка проверки структуры RSC файла"),
			      QMessageBox::Ok);
	ui_->layersList->clear();
	ui_->applyButton->setEnabled(false);	
      }
    }
  } else if (ui_->rscFile->text().isEmpty()) {
    ui_->applyButton->setEnabled(false);
  }
}

void ImportSxf::fillLayers()
{
  if (!sxf_->isValid()) {
    return;
  }

  saveConf();

  QMap<uint8_t, sxf::LayersDescription> legend;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  bool ok = sxf_->fillLegend(&legend);
  QApplication::restoreOverrideCursor();
  if (!ok) {
    QMessageBox::critical(this, QObject::tr("Ошибка"), 
			  QObject::tr("Ошибка определения слоев документа. Убедитесь в согласованности выбранных файлов"),
			  QMessageBox::Ok);

    ui_->applyButton->setEnabled(false);
    ui_->layersList->clear();
    return;
  }

  // var(layers.count());
  // var(layers);
  ui_->layersList->clear();
  QTreeWidgetItem* rootItem = new QTreeWidgetItem(ui_->layersList);
  rootItem->setExpanded(true);
  rootItem->setCheckState(CHECK, Qt::Checked);

  QMapIterator<uint8_t, sxf::LayersDescription> it(legend);
  bool cc = true;
  while (it.hasNext()) {
     it.next();
     if (it.key() != 14) { //FIXME мат основа
       QTreeWidgetItem* item = new QTreeWidgetItem(rootItem);
       item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
       item->setCheckState(CHECK, Qt::Checked);
       item->setText(NUMBER, QString::number(it.key()));
       item->setText(NAME, it.value().layName);
       int objcnt = 0;
       QTreeWidgetItem* childFirst = 0;
       QMapIterator<sxf::ObjectKey, sxf::ObjectDescription> itobj(it.value().obj);
       QMapIterator<sxf::ObjectKey, sxf::ObjectDescription> itOld(itobj);
       cc = true;
       while (itobj.hasNext()) {
	 itOld = itobj;
	 itobj.next();
	 QTreeWidgetItem* object = 0;
	 if (!itOld.hasPrevious() || itOld.key() != itobj.key()) {
	   object = new QTreeWidgetItem(item);
	   object->setCheckState(CHECK, Qt::Checked);
	   object->setText(NUMBER, QString::number(itobj.key().second) + " " + QString::number(itobj.key().first));
	   object->setText(NAME, itobj.value().name);
	   object->setText(COUNT , QString::number(itobj.value().count));
	   objcnt += itobj.value().count;
	   childFirst = object;
	   cc = !cc;
	 } else if (childFirst != 0) {
	   object = new QTreeWidgetItem(childFirst);
	   object->setText(NUMBER, QString::number(itobj.key().second) + " " + QString::number(itobj.key().first));
	   object->setText(NAME, itobj.value().name);
	 }
	 QBrush br(QColor(253, 253, 253));
	 if (!cc) {
	   br.setColor(QColor(233, 233, 233));
	 }
	 object->setBackground(NUMBER, br);
	 object->setBackground(NAME, br);
	 object->setBackground(COUNT, br);
       }
       item->setText(COUNT, QString::number(objcnt));
     }
  }

  for (int i = 0, sz = ui_->layersList->columnCount(); i < sz; ++i) {
    ui_->layersList->resizeColumnToContents(i);
  }

  ui_->applyButton->setEnabled(true);
}

bool ImportSxf::readFile(const QString& fileName, QByteArray *ba)
{
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(fileName);
    QMessageBox::critical(this, QObject::tr("Ошибка открытия файла"), 
			  QObject::tr("Ошибка открытия файла %1").arg(fileName), 
			  QMessageBox::Ok);
    return false;
  }
  *ba = file.readAll();
  
  file.close();

  return true;
}

void ImportSxf::accept()
{
  map::Document* original = currentDocument();

  if (original == 0) {
    error_log << QString::fromUtf8("Ошибка. Документ не инициализирован");
    return;
  }
  
  if (ui_->layersList->topLevelItemCount() == 0) {
    error_log << QString::fromUtf8("Ошибка. Не выбраны объекты для импортирования");
    return;
  }

  QList<QPair<QString, QString> > layerNums;
  int count = 0;
  QList<sxf::ObjectKey> checked;
  for (int lay = ui_->layersList->topLevelItem(0)->childCount() - 1; lay >=0; --lay) {
    QTreeWidgetItem* item = ui_->layersList->topLevelItem(0)->child(lay);
    if (item != 0 && item->checkState(CHECK) == Qt::Checked) {
      for (int obj = 0; obj < item->childCount(); obj++) {
	QTreeWidgetItem* obitem = item->child(obj);
	if (obitem != 0 && obitem->checkState(CHECK) == Qt::Checked) {
	  QStringList okey = obitem->text(NUMBER).split(" ");
	  if (okey.count() >= 2) {
	    checked.append(sxf::ObjectKey(okey.at(1).toUInt(), okey.at(0).toUInt()));
	    count += obitem->text(COUNT).toInt();
	    layerNums.append(QPair<QString, QString>(item->text(NUMBER), item->text(NAME)));
	  }
	}
      }
    }
  }

  //  var(count);
  ui_->loadedBar->setMaximum(count);
  ui_->loadedBar->setValue(0);
  

  QApplication::setOverrideCursor(Qt::WaitCursor);
  bool ok = sxf_->import(layerNums, checked, currentDocument());
  QApplication::restoreOverrideCursor();
  if (!ok) {
    QMessageBox::critical(this, QObject::tr("Ошибка преобразования"), 
			  QObject::tr("Ошибка преобразования данных. Убедитесь в правильности выбранных файлов"),
			  QMessageBox::Ok);
  } else {
    QFileInfo fi( ui_->sxfFile->text() );
    original->setName( fi.fileName() );
    cancel();
  }
}

bool ImportSxf::readSxfDescription()
{
  if (currentDocument() == 0 || currentDocument()->projection() == 0) return false;

  bool ok = false;
  QByteArray sxfData;
  if (readFile(ui_->sxfFile->text(), &sxfData)) {
    SxfDescription descr;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ok = sxf_->readDescription(sxfData, &descr);
    QApplication::restoreOverrideCursor();
    if (ok) {
      ui_->mapName->setText(descr.name);
      ui_->mapDt->setText(descr.date.toString("dd.MM.yyyy"));
      ui_->objectsCnt->setText(QString::number(descr.objCnt));
      ui_->loadedBar->setMaximum(descr.objCnt);
      ui_->loadedBar->setValue(0);
      // if (currentDocument()->projection()->type() == meteo::MERCAT) {
      // 	if (!qFuzzyCompare(currentDocument()->projection()->getMapCenter().lon(), 0)) {
      // 	  ui_->warning->show();
      // 	  warning_log << QObject::tr("Центр карты не совпадает с нулевым меридианом, возможно некорректное преобразование координат") << currentDocument()->center();
      // 	}
      // }
    } else {
      QMessageBox::critical(this, QObject::tr("Ошибка проверки файла"), 
			    QObject::tr("Ошибка проверки структуры SXF файла"),
			    QMessageBox::Ok);
      ui_->applyButton->setEnabled(false);
      ui_->layersList->clear();
    }
  }

  return ok;
}

void ImportSxf::importedObjects(int loaded, int )
{
  ui_->loadedBar->setValue(loaded);
  ui_->loadedBar->update();
}
