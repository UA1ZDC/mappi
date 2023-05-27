#ifndef _ASTRO_DATA_COLOR_PROXY_MODEL_H_
#define _ASTRO_DATA_COLOR_PROXY_MODEL_H_

#include <QIdentityProxyModel>


namespace astro
{

/** @brief Прокси модель - изменяет цвета колонок */
class AstroDataColorProxyModel :
    public QIdentityProxyModel
{
    Q_OBJECT
public :
    explicit AstroDataColorProxyModel(QObject* parent = 0);
    virtual ~AstroDataColorProxyModel();

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
};

}

#endif //_ASTRO_DATA_COLOR_PROXY_MODEL_H_
