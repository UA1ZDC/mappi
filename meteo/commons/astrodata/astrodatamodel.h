#ifndef _ASTRO_DATA_MODEL_H_
#define _ASTRO_DATA_MODEL_H_

#include "astrodata.h"
#include <QAbstractTableModel>


namespace astro
{

/** @brief Модель астрономических данных */
class AstroDataModel :
    public QAbstractTableModel
{
    Q_OBJECT
public :
    explicit AstroDataModel(AstroDataCalendar* calendar, QObject* parent = 0);
    virtual ~AstroDataModel();

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    void update();

private :
    AstroDataCalendar* m_calendar;
};

}

#endif // _ASTRO_DATA_MODELH_
