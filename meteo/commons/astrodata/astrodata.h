#ifndef _ASTRO_DATA_H_
#define _ASTRO_DATA_H_

#include "observerpoint.h"
#include <QList>


namespace astro
{

/** @brief Астрономические данные */
class AstroData
{
public :
    enum lunar_quarter_t
    {
        LUNAR_NEW = 0,
        LUNAR_Q1,
        LUNAR_FULL,
        LUNAR_Q3,

        LUNAR_YOUNG = 10,
        LUNAR_WAXING = 11,
        LUNAR_WANING = 12,
        LUNAR_OLD = 13,

        LUNAR_UNKNOWN
    };

    static QString lunarQuarterName(lunar_quarter_t quarter);
    static lunar_quarter_t lunarQuarter(const QString& name);

public :
    AstroData();
    AstroData(const QDate& date);

    ~AstroData();

    QDate getDate() const;
    void setDate(const QDate& value);

    QTime getSunRise() const;
    void setSunRise(const QTime& value);

    QTime getSunSet() const;
    void setSunSet(const QTime& value);

    QTime dayDuration(ObserverPoint::hemisphere_t hemisphere) const;
    QTime nightDuration(ObserverPoint::hemisphere_t hemisphere) const;

    QTime getLunarRise() const;
    void setLunarRise(const QTime& value);

    QTime getLunarSet() const;
    void setLunarSet(const QTime& value);

    lunar_quarter_t getLunarQuarter() const;
    void setLunarQuarter(lunar_quarter_t value);
    lunar_quarter_t nextLunarQuarter() const;
    lunar_quarter_t previousLunarQuarter() const;

    QTime getTwilightCivilBegin() const;
    void setTwilightCivilBegin(const QTime& value);

    QTime getTwilightCivilEnd() const;
    void setTwilightCivilEnd(const QTime& value);

    QTime getTwilightNauticalBegin() const;
    void setTwilightNauticalBegin(const QTime& value);

    QTime getTwilightNauticalEnd() const;
    void setTwilightNauticalEnd(const QTime& value);

    QTime getTwilightAstronomicalBegin() const;
    void setTwilightAstronomicalBegin(const QTime& value);

    QTime getTwilightAstronomicalEnd() const;
    void setTwilightAstronomicalEnd(const QTime& value);

private :
    bool betweenEquinox() const;

private :
    QDate m_date;
    QTime m_sunRise;
    QTime m_sunSet;
    QTime m_lunarRise;
    QTime m_lunarSet;
    lunar_quarter_t m_lunarQuarter;
    QTime m_twilightCivilBegin;
    QTime m_twilightCivilEnd;
    QTime m_twilightNauticalBegin;
    QTime m_twilightNauticalEnd;
    QTime m_twilightAstronomicalBegin;
    QTime m_twilightAstronomicalEnd;
};


/** @brief Календарь астрономических данных */
class AstroDataCalendar
{
public :
  enum MakeFlags {
    kMakeSun              = 0x01,
    kMakeMoon             = 0x02,
    kMakeLunarQuarter     = 0x04,
    kMakeAstronomicalTwi  = 0x08,
    kMakeNautricalTwi     = 0x10,
    kMakeCivilTwi         = 0x20,
    kMakeAll = kMakeSun | kMakeMoon | kMakeLunarQuarter | kMakeAstronomicalTwi | kMakeNautricalTwi | kMakeCivilTwi,
  };

public :
    AstroDataCalendar();
    ~AstroDataCalendar();

    ObserverPoint& observerPoint();
    const ObserverPoint& observerPoint() const;

    Qt::TimeSpec timeSpec() const { return m_tmspec; }
    void setTimeSpec( Qt::TimeSpec spec ) { m_tmspec = spec; }

    void clear();
    void makeUp(MakeFlags flags = kMakeAll);
    void setEmptyDay(int count);

    int daysTo() const;
    AstroData& day(int index);
    const AstroData& day(int index) const;
    void appendDay(const AstroData& item);

    QString info() const;

private :
    QTime convertTimePaitToQTime(double timeJD);

    AstroData::lunar_quarter_t getFirstlunarQuarter();
    AstroData::lunar_quarter_t lunarQuarter(double timeJD);

private :
    ObserverPoint m_observer;
    QList<AstroData> m_list;
    Qt::TimeSpec m_tmspec;
    int m_localdiff;
};

}

#endif // _ASTRO_DATA_H_
