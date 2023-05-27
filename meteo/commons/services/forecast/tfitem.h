#ifndef METEO_COMMONS_FORECAST_TFITEM_H
#define METEO_COMMONS_FORECAST_TFITEM_H

#include <QList>
#include <QVariant>
#include <QColor>
#include <qicon.h>
#include <qscriptvalue.h>
#include <qscriptengine.h>
#include <commons/geobasis/geopoint.h>
#include "tdataprovider.h"

class TFModel;
class QIcon;
class TDataProvider;

const QColor BAD_DATA    = Qt::red;
const QColor GOOD_DATA   = Qt::green;
const QColor MANUAL_DATA = Qt::yellow;
const QString BAD_VALUE  = "-9999";
class TFItem: public QObject
{
  Q_OBJECT

public:
  TFItem(const QString & text, const QString & descr, TFModel *parent);
   virtual ~TFItem();


    TFItem *child(int row);
    int childCount() const;
    QVariant data(int column) ;
    int row() const;
    TFModel *model();

    meteo::forecast::ForecastMethodItemRole role();

    TDataProvider* dataProvider();
    TFItem *getPoDescriptor(const QString& );
   // QString getValuePoName(const QString &);
    QVariant getValuePoName(const QString &);
    TFItem *setDependPoName(const QString &);

    TFItem* getItem(const QString &args);
    QString getDescr(){return itemDescriptor_;}
    QString getText(){return itemText_;}
    void setDescr(const QString & ad){itemDescriptor_=ad;}
    QColor& marker() ;

    void resetScript();
    void resetChilds();

    // получаем список подчиненных айтемов
    QList<TFItem*> getChilds(){ return this->childItems_; }

    virtual void resetData();
    void resetAll();

    virtual QVariant getItemData() ;
    virtual QVariant getItemDataM() ;

    virtual QVariant getAddItemData() ;
    virtual QStringList listData( );
    virtual int currentIndex( );
    virtual void reset();
    virtual void setData(const QVariant& adata);
    virtual void setItemData(const QVariant& adata);

    virtual void setMarker(const QColor&);
    virtual QString getItemInfo();
    virtual void appendDeps(TFItem *);
    virtual void setNoData();
    virtual void setErrInScript();
    virtual const meteo::GeoPoint & curCoord()const ;
    virtual void appendChild(TFItem *item);
    virtual int columnCount() const;
    QString getComboText(const FMetaData & m);
    //void setAddData(const QVariant& adata);

    // здесь мы возвращаем значение, которое зашито в экземпляре класса
    QRegExp  getRegexpByParam();
    int  checkRoundDigits();

    void setRole(const meteo::forecast::ForecastMethodItemRole& arole);
    void setLevel(const QString & alvl);
    void setTypeLevel(const QString & alvl);
    void setSrok(const QString & );
    void setCurCoord(const meteo::GeoPoint & ac);
    void setFunction(const  QScriptValue& afunc,const  QStringList& args);
    void setParrent(const QString &);
    void setDataSourceType(dataSource sdt);


    TFItem* parent();
    TFItem* constParent() const;
    int resultDescriptor();

    void setResultDescriptor(const QString &adescr);
    void needReset(bool r){need_reset_ = r;}
    void needResetAll(bool r);

    // все ли маркеры введены
    bool isAllGood(){ return good_data_; }
    bool isInAdvection() const;
    void setInAdvection(bool );
    const QMap<int, float > &getValuesMap()const {return values_list_;}
    const QMap<int, FMetaData> &getMetaMap()const {return meta_list_;}

    // получаем список айтемов, которые должны быть в дроп-боксе
    QStringList getItems(){ return items_; }


    // запрещаем и разрешаем автообновление и автоперерасчет прогнозов
    void disableAutoReset(){ disable_autoreset=true; };
    void enableAutoReset() { disable_autoreset=false; };

    bool resetFirstChildItemForScript();
  
signals:
    // сигнал об изменении маркера
    void markerChanged();

private:
  QList<TFItem*> childItems_; // для отображения
  QList<TFItem*> depItems_;   //для зависимости значений
  TFModel *model_;
  bool getValue() ;

protected:
  QStringList *itemScriptArgs_;
  QScriptValue *itemFunc_;
  QStringList items_;

  meteo::forecast::ForecastMethodItemRole itemRole_;
  QString itemText_;
  QString itemDescriptor_;
  QString parentItem_;
  //QVariant adata_;

  bool is_in_advection_ =false;
  bool good_data_ =false;
  QColor marker_;
  QVariant itemData_;
  FMetaData meta_data_;


  meteo::GeoPoint cur_coord_;

  bool need_reset_;
  QMultiMap<int, FMetaData> meta_list_;
  QMap<int, float> values_list_;
  int cur_value_index_;
  QString srok_;
  QString time_;
  QString type_level_ ;
  QString level_;

  // запрещаем автопереасчет при изменении данных
  // если включено, то расчет прогнозов производится исключительно вручную, при вызове функции reset()
  // необходимо для того, чтобы присвоить пачку параметров, и рассчитать прогноз в конце один раз
  bool disable_autoreset = false;

};
//! [0]

#endif
