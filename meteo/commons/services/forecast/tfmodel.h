#ifndef METEO_COMMONS_FORECAST_TFMODEL_H
#define METEO_COMMONS_FORECAST_TFMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>


#include <qscriptvalue.h>

class TForecast;
class TDataProvider;
class TFItem;
struct StationData;

//! [0]
class TFModel : public QAbstractItemModel
{
    Q_OBJECT

public:
  TFModel(TDataProvider *adataProvider=0, TForecast *parent = 0,const QString & aname ="");
    ~TFModel();

    QString methodFullName();
    QString methodName();


    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void resetModel();
    void addSimpleItem(const QString & text,const QString&descrName, const QString &parentName, const QString &);
    TFItem *addDataItem(const QString&, const QString&, const QString & parentDescr, const QString & alvl, const QString & , const QString & asrok=0 , const QString &aname="");
    void addCalcItem(const QString&text,  const QString&descrName, const QString& funcName, const QString & parentDescr/*, int alvl*/);
//    void addManualItem(const QString&, const QString&, const QString & parentDescr);
    TFItem* addComboFuncItem(const QScriptValue& func, const  QScriptValue& args,const QString& parentName);

    void addFieldItem(const QString& parentName, const QString& itemText,const QString& aname,
                      const QString & sparam, const QString &asrok,const QString &alevel,const QString &atypelevel, const QString &atime);
    TFItem* addFuncItem(const QScriptValue& func,const  QScriptValue& list,const QString& parentName);
    void addFieldValueItem(const QString& parentName, const QString& text,
                           const QString & sparam, const QString &asrok,const QString &alevel, const QString &atypelevel,const QString& aName);
    void addFieldAdd(const QString& parentName,const QString& addn, const QString& text,const QString& aname,
                     const QString & sparam, const QString &asrok,const QString &alevel, const QString &atypelevel, const QString &atime);
    void addComboItem(const QString& text, const QString& descr, const QStringList& items, const QString& parentName);
    TFItem* addDTItem(const QScriptValue& func,
                   const  QScriptValue& args,
                   const QString& parentName);

    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    void setLevel(const QString &alevel);
    TFItem *rootItem();
    TDataProvider* dataProvider(){
      return dataProvider_;
    }
    TForecast *mainMethod(){
        return main_method_;
    }
    TFItem* getItem(const QString& parentName);

    void repaint();
    void beforeRepaint();
    StationData station()const ;


    /**
     * получаем флаг того, что все данные введены и айтемы зелененькие
     * @return [description]
     */
    bool isAllGood(){  return all_good;  }

    // подключаем новый айтем к сигналу
    void connectNewItem(TFItem *item);

public slots:
  // слот который проверяем все ли маркеры установлены в зелененький
  void slotCheckMarkers();

signals:
  // изменение общего состояния маркеров правильности вводимых данных
  void allGoodChanges();

private:
  TFItem *rootItem_=nullptr;
  TDataProvider* dataProvider_=nullptr;
  TForecast *main_method_=nullptr;
  // флаг того, что все айтемы зеленые или желтые
  bool all_good = false;
};
//! [0]

#endif
