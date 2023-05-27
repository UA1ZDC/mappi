#ifndef _ASTRO_DATA_INFO_H_
#define _ASTRO_DATA_INFO_H_

#include "astrodata.h"
#include "defines.h"
#include <QString>
#include <QDateTime>
#include <QIcon>


namespace astro
{

/** @brief Информация (кортеж) об астрономических данных для отображения на view */
class AstroDataInfo
{
public :
    static QString toString(const QDate& date);
    static QString toString(const QTime& time, bool simplified = false);

    static QTime toTime(const QString& value);
    static QDate toDate(const QString& value);

public :
    enum column_t
    {
        DATE = 0,
        SUN_RISE,
        SUN_SET,
        DAY_DURATION,
        NIGHT_DURATION,

        LUNAR_RISE,
        LUNAR_SET,
        LUNAR_QUARTER,
        TWILIGHT_CIVIL_BEGIN,
        TWILIGHT_CIVIL_END,

        TWILIGHT_NAUTICAL_BEGIN,
        TWILIGHT_NAUTICAL_END,
        TWILIGHT_ASTRONOMICAL_BEGIN,
        TWILIGHT_ASTRONOMICAL_END,

        COLUMN_COUNT
    };

public :
    AstroDataInfo(const AstroData& item);
    ~AstroDataInfo();

    QString date() const;

    QString sunRise() const;
    QString sunSet() const;
    QString dayDuration(ObserverPoint::hemisphere_t hemisphere) const;

    QString nightDuration(ObserverPoint::hemisphere_t hemisphere) const;
    QString lunarRise() const;
    QString lunarSet() const;
    QIcon lunarQuarterIcon() const;
    QString lunarQuarterName() const;

    QString twilightCivilBegin() const;
    QString twilightCivilEnd() const;
    QString twilightNauticalBegin() const;
    QString twilightNauticalEnd() const;
    QString twilightAstronomicalBegin() const;
    QString twilightAstronomicalEnd() const;

private :
    const AstroData& m_item;
};

}

#endif // _ASTRO_DATA_INFO_H_
