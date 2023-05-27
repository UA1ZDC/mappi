#ifndef CUSTOMVIEWERDATAMODEL_H
#define CUSTOMVIEWERDATAMODEL_H

#include <qhash.h>
#include <qlist.h>
#include <qicon.h>
#include <meteo/commons/global/global.h>
#include <sql/nosql/document.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/proto/customviewer.pb.h>

namespace meteo {

class CustomViewerDataModel : public QObject
{
  Q_OBJECT
public:
  CustomViewerDataModel(int pageSize);
  virtual ~CustomViewerDataModel();

  virtual void initDataModel();
  int getCurrentPage() const { return this->currentPage_; }
  virtual proto::CustomViewerId tableId() const;
  bool hasCondition(const QString& col) { return hasCondition( columnIndexByName(col) ); }
  bool hasCondition(int col) { return conditions_.contains(col); }

  int columnSize();
  QString columnName(int col);
  //Управление колонками  
  bool hasColumn(const QString& columnName);

  proto::ViewerType columnType(const QString& col) const;
  proto::ViewerType columnType(int col) const;

  int columnIndexByName(const QString& name) const;
  QString columnDisplayName(const QString& name) const;
  QString columnDisplayName(int index) const;

  Qt::SortOrder getSortOrder() const { return this->sortOrder_; }
  QString getSortColumn() const { return this->sortColumn_; }

  //Переход по страницам
  int getPageSize() const { return pageSize_; }
  void setPageSize(int pageSize);

  void setPage(int page);
  void setNextPage();
  void setPrewPage();
  void setFirstPage();

  void threadDataLoad();

  //Сортировка
  void setSort(const QString& sortColumn, Qt::SortOrder sortOrder);
  void setSort(int sortColumn, Qt::SortOrder sortOrder);

  //Условия отбора  
  QStringList filterColumns();
  bool hasConditions();
  void clearConditions();
  void conditionEnable(const QString& column, const bool enabled);
  void conditionEnable(const int column, const bool enabled);

  void conditionRemove(const QString& column);
  void conditionRemove(int column);
  void conditionRemove(const QStringList& columns);

  void addCondition(int column, const Condition& condition, bool enabled = true);
  void addCondition(const QString& column, const Condition& condition, bool enabled = true);
  void addConditionByText(int col, const QString& value, const QString& displayValue);

  void insertManyConditions( const QHash<int, Condition> &conditions );
  void insertManyConditions( const QHash<QString, Condition> &conditions );

  QHash<int, Condition> getConditions() const;
  QHash<int,  Condition> getConditionsEnabled() const;

  QList<proto::ViewerColumnMenu> menu(const QString& column) const;

  //Рекомендованные для отбора поля
  bool ifWillRemoveLastRecomendedField(int index);
  QStringList getRecomendedFields();

  //Дополнительная информация о колонках
  QString columnHeaderTooltip(const QString& column) const;
  bool columnCanSort(int index) const;
  bool columnCanSort(const QString& columnName) const;

  //Строка в заголовке окна - шаблон для количества записей в бд
  virtual const QString& stringTemplateTotalRecords() const = 0;
  //Заголовок окна
  virtual const QString& windowTitle() const = 0;

  QList<proto::CustomViewerField> getFieldsForColumn(const QString& column) const;

  bool hasDocuments();
  int countDocuments();

  //Возвращает виджет, который нужно установить внутри панели слева.
  virtual QWidget* getLeftPanelWidget() const;

protected:
  class ConditionData {
  public:
    Condition condition_;
    bool isEnabled;
  };

  virtual QList<proto::CustomViewerTableRecord> load(QList<Condition> conditions) const;
  qint64 countTotal();

  //Разбор строки
  QHash<int, QString> dataFromDocument(const DbiEntry &doc);  

  void clearDocuments() { this->allDocuments_.clear(); }

  ConditionData* conditionForColumn(int col);

private:
  void threadCountDataInDatabase();

  void addConditionRaw(int index, const Condition& condition,  bool isEnabled);
  bool conditionRemoveRaw( int index );

  void emitSignalChangeConditions();
private slots:
  void slotCountDataInDatabase();
  void slotTimerStop(bool status);


signals:
  void signalUpdate();
  void signalLock(bool lockStatus);
  void signalNewData(qint64 newDataCount);
  void signalDataLoaded(const QList<proto::CustomViewerTableRecord>* data, const int &page);
  void signalConditionsChanged();

protected:

  const proto::ViewerColumn* columnConfig(const QString& column) const;
  const proto::ViewerColumn* columnConfig(int column) const;
private:
  //Фильтры
  QHash<int, ConditionData*> conditions_;

  //Страницы
  int currentPage_ = 0;
  int pageSize_ = 0;

  //Сортировка
  QString sortColumn_;
  Qt::SortOrder sortOrder_;

  QTimer databaseCounterTimer_;

  QList<proto::CustomViewerTableRecord> allDocuments_;

  proto::ViewerConfig config_;

  friend class CustomViewer;
};


}

#endif
