#include "doctypefilter.h"
#include "ui_doctypefilter.h"

#include <cross-commons/debug/tlog.h>
#include <qevent.h>

#include <QtConcurrent/qtconcurrentrun.h>
#include <meteo/commons/global/global.h>

DocTypeFilter::DocTypeFilter(QWidget *parent) :
  QWidget(parent),
  mapTopLevelItem(new QTreeWidgetItem()),
  mapTypes(QList<QTreeWidgetItem*>()),
  documentTopLevelItem(new QTreeWidgetItem()),
  documentTypes(QList<QTreeWidgetItem*>()),
  ui_(new Ui::DocTypeFilter)
{
  ui_->setupUi(this);
  this->mapTopLevelItem->setText(0, "Карты");
  this->documentTopLevelItem->setText(0, "Документы");

  this->ui_->filterTree->addTopLevelItem(mapTopLevelItem);
  this->ui_->filterTree->addTopLevelItem(documentTopLevelItem);

  for (int i = 0; i < ui_->filterTree->topLevelItemCount(); ++i){
    auto topItem = ui_->filterTree->topLevelItem(i);
    topItem->setFlags(Qt::ItemIsEnabled);
    topItem->setExpanded(true);
    topItem->setChildIndicatorPolicy(QTreeWidgetItem::ChildIndicatorPolicy::ShowIndicator);
  }

  QObject::connect(this, &DocTypeFilter::signalDocumentTypesReceived, this, &DocTypeFilter::slotDocumentTypesReceived);
  QObject::connect(this, &DocTypeFilter::signalMapTypesReceived, this, &DocTypeFilter::slotMapTypesReceived);
  QObject::connect(ui_->filterTree, &QTreeWidget::itemSelectionChanged, this, &DocTypeFilter::slotSelectionChanged);

  QtConcurrent::run(this, &DocTypeFilter::getMapTypes);
  QtConcurrent::run(this, &DocTypeFilter::getDocumentTypes);
}

QStringList DocTypeFilter::getSelectedDocumentTypes()
{
  QStringList selectedDocuments;
  for (int i = 0; i < documentTopLevelItem->childCount(); ++i){
    auto documentItem = documentTopLevelItem->child(i);
    if (documentItem->isSelected()){
      selectedDocuments << documentItem->text(0);
    }
  }
  return selectedDocuments;
}

QStringList DocTypeFilter::getSelectedMapTypes()
{
  QStringList selectedMaps;
  for (int i = 0; i < mapTopLevelItem->childCount(); ++i){
    auto mapItem = mapTopLevelItem->child(i);
    if (mapItem->isSelected()){
      selectedMaps << mapItem->text(0);
    }
  }
  return selectedMaps;
}

void DocTypeFilter::slotSelectionChanged()
{
  auto selectedMaps = getSelectedMapTypes();
  auto selectedDocuments = getSelectedDocumentTypes();

  mapTopLevelItem->setText(0, QString("Карты: %1/%2")
                           .arg(selectedMaps.count())
                           .arg(mapTopLevelItem->childCount()));
  documentTopLevelItem->setText(0, QString("Документы: %1/%2")
                                .arg(selectedDocuments.count())
                                .arg(documentTopLevelItem->childCount()));
}

DocTypeFilter::~DocTypeFilter()
{
  if (nullptr != ui_) {
    delete ui_; ui_ = nullptr;
  }
}

void DocTypeFilter::getMapTypes()
{
  //TODO - using service
  QString query = "{ \"distinct\" : \"weather_images\", \"key\" : \"map_template\" }";
  auto result = nosql::NoSql::execQuery(::meteo::global::mongodbConfDocument().host(),
                                        ::meteo::global::mongodbConfDocument().port().toInt(),
                                        "documentdb", query);
  auto distinct = result->result();
  auto allFields = distinct.valueArray("values");
  QStringList allTypes;
  while (allFields.next()){
    allTypes << allFields.valueString();
  }
  emit signalMapTypesReceived(allTypes);
}

void DocTypeFilter::getDocumentTypes()
{
  //TODO - using service
  QString query = "{ \"distinct\" : \"weather_document\", \"key\" : \"name\" }";

  auto result = nosql::NoSql::execQuery(::meteo::global::mongodbConfDocument().host(),
                                        ::meteo::global::mongodbConfDocument().port().toInt(),
                                        "documentdb", query);
  auto distinct = result->result();
  auto allFields = distinct.valueArray("values");
  QStringList allTypes;
  while (allFields.next()){
    allTypes << allFields.valueString();
  }
  emit signalDocumentTypesReceived(allTypes);
}

void DocTypeFilter::slotDocumentTypesReceived(QStringList allDocumentTypes)
{
  for (auto doctype : allDocumentTypes ){
    addItem(documentTopLevelItem, doctype);
  }
}

void DocTypeFilter::slotMapTypesReceived(QStringList allMapTypes)
{
  for (auto doctype : allMapTypes ){
    addItem(mapTopLevelItem, doctype);
  }
}

void DocTypeFilter::addItem(QTreeWidgetItem* parentItem, const QString& title)
{
  QTreeWidgetItem* item = new QTreeWidgetItem();
  item->setText(0, title);
  parentItem->addChild(item);
  item->setSelected(true);
}
