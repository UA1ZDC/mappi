#include "astrodatainfo.h"
#include "defines.h"

using namespace astro;


QString AstroDataInfo::toString(const QDate& date)
{
    return date.toString(DATE_FORMAT);
}

QString AstroDataInfo::toString(const QTime& time, bool simplified /*=*/)
{
    if (time.isValid())
    {
        return time.toString(TIME_FORMAT);
    }

    QString timeString(TIME_FAIL);
    return (simplified ? timeString.simplified() : timeString);
}

QTime AstroDataInfo::toTime(const QString& value)
{
    return QTime::fromString(value, TIME_FORMAT);
}

QDate AstroDataInfo::toDate(const QString& value)
{
    return QDate::fromString(value, DATE_FORMAT);
}


AstroDataInfo::AstroDataInfo(const AstroData& item) :
    m_item(item)
{
}

AstroDataInfo::~AstroDataInfo()
{
}

QString AstroDataInfo::date() const
{
    return toString(m_item.getDate());
}

QString AstroDataInfo::sunRise() const
{
    return toString(m_item.getSunRise());
}

QString AstroDataInfo::sunSet() const
{
    return toString(m_item.getSunSet());
}

QString AstroDataInfo::dayDuration(ObserverPoint::hemisphere_t hemisphere) const
{
    return toString(m_item.dayDuration(hemisphere));
}

QString AstroDataInfo::nightDuration(ObserverPoint::hemisphere_t hemisphere) const
{
    return toString(m_item.nightDuration(hemisphere));
}

QString AstroDataInfo::lunarRise() const
{
    return toString(m_item.getLunarRise());
}

QString AstroDataInfo::lunarSet() const
{
    return toString(m_item.getLunarSet());
}

QIcon AstroDataInfo::lunarQuarterIcon() const
{
    switch (m_item.getLunarQuarter())
    {
        case AstroData::LUNAR_NEW :
            return QIcon(":/astro/moon_new");

        case AstroData::LUNAR_Q1 :
            return QIcon(":/astro/moon_1q");

        case AstroData::LUNAR_FULL :
            return QIcon(":/astro/moon_full");

        case AstroData::LUNAR_Q3 :
            return QIcon(":/astro/moon_3q");

         case AstroData::LUNAR_YOUNG :
             return QIcon(":/astro/moon_young");

         case AstroData::LUNAR_WAXING :
             return QIcon(":/astro/moon_waxing");

         case AstroData::LUNAR_WANING :
             return QIcon(":/astro/moon_waning");

         case AstroData::LUNAR_OLD :
             return QIcon(":/astro/moon_old");

        default :
            return QIcon();
    }
}

QString AstroDataInfo::lunarQuarterName() const
{
    switch (m_item.getLunarQuarter())
    {
        case AstroData::LUNAR_NEW :
            return RU("новолуние");

        case AstroData::LUNAR_Q1 :
            return RU("первая четверть");

        case AstroData::LUNAR_FULL :
            return RU("полнолуние");

        case AstroData::LUNAR_Q3 :
            return RU("последняя четверть");

         case AstroData::LUNAR_YOUNG :
             return RU("молодая луна");

         case AstroData::LUNAR_WAXING :
             return RU("прибывающая луна");

         case AstroData::LUNAR_WANING :
             return RU("убывающая луна");

         case AstroData::LUNAR_OLD :
             return RU("старая луна");

        default :
            return QString();
    }
}

QString AstroDataInfo::twilightCivilBegin() const
{
    return toString(m_item.getTwilightCivilBegin());
}

QString AstroDataInfo::twilightCivilEnd() const
{
    return toString(m_item.getTwilightCivilEnd());
}

QString AstroDataInfo::twilightNauticalBegin() const
{
    return toString(m_item.getTwilightNauticalBegin());
}

QString AstroDataInfo::twilightNauticalEnd() const
{
    return toString(m_item.getTwilightNauticalEnd());
}

QString AstroDataInfo::twilightAstronomicalBegin() const
{
    return toString(m_item.getTwilightAstronomicalBegin());
}

QString AstroDataInfo::twilightAstronomicalEnd() const
{
    return toString(m_item.getTwilightAstronomicalEnd());
}
