#include "astrodata.h"
#include "defines.h"
#include "AstroOps.h"
#include "PlanetData.h"
#include "DateOps.h"
#include "RiseSet.h"
#include "PlanetData.h"
#include <QMap>
#include <stdio.h>

#include <cross-commons/debug/tlog.h>

using namespace astro;


// class AstroData
QString AstroData::lunarQuarterName(lunar_quarter_t quarter)
{
    static QMap<AstroData::lunar_quarter_t, QString> result;
    if (result.isEmpty())
    {
        result.insert(AstroData::LUNAR_NEW, LUNAR_NEW_NAME);
        result.insert(AstroData::LUNAR_Q1, LUNAR_Q1_NAME);
        result.insert(AstroData::LUNAR_FULL, LUNAR_FULL_NAME);
        result.insert(AstroData::LUNAR_Q3, LUNAR_Q3_NAME);
        result.insert(AstroData::LUNAR_YOUNG, LUNAR_YOUNG_NAME);
        result.insert(AstroData::LUNAR_WAXING, LUNAR_WAXING_NAME);
        result.insert(AstroData::LUNAR_WANING, LUNAR_WANING_NAME);
        result.insert(AstroData::LUNAR_OLD, LUNAR_OLD_NAME);
    }
    return (result.contains(quarter) ? result.value(quarter) : QString());
}

AstroData::lunar_quarter_t AstroData::lunarQuarter(const QString& name)
{
    static QMap<QString, AstroData::lunar_quarter_t> result;
    if (result.isEmpty())
    {
        result.insert(LUNAR_NEW_NAME, AstroData::LUNAR_NEW);
        result.insert(LUNAR_Q1_NAME, AstroData::LUNAR_Q1);
        result.insert(LUNAR_FULL_NAME, AstroData::LUNAR_FULL);
        result.insert(LUNAR_Q3_NAME, AstroData::LUNAR_Q3);
        result.insert(LUNAR_YOUNG_NAME, AstroData::LUNAR_YOUNG);
        result.insert(LUNAR_WAXING_NAME, AstroData::LUNAR_WAXING);
        result.insert(LUNAR_WANING_NAME, AstroData::LUNAR_WANING);
        result.insert(LUNAR_OLD_NAME, AstroData::LUNAR_OLD);
    }
    return (result.contains(name) ? result.value(name) : AstroData::LUNAR_UNKNOWN);
}


AstroData::AstroData() :
    m_date(QDate::currentDate()),
    m_sunRise(QTime()),
    m_sunSet(QTime()),
    m_lunarRise(QTime()),
    m_lunarSet(QTime()),
    m_lunarQuarter(AstroData::LUNAR_UNKNOWN),
    m_twilightCivilBegin(QTime()),
    m_twilightCivilEnd(QTime()),
    m_twilightNauticalBegin(QTime()),
    m_twilightNauticalEnd(QTime()),
    m_twilightAstronomicalBegin(QTime()),
    m_twilightAstronomicalEnd(QTime())
{
}

AstroData::AstroData(const QDate& date) :
    m_date(date),
    m_sunRise(QTime()),
    m_sunSet(QTime()),
    m_lunarRise(QTime()),
    m_lunarSet(QTime()),
    m_lunarQuarter(AstroData::LUNAR_UNKNOWN),
    m_twilightCivilBegin(QTime()),
    m_twilightCivilEnd(QTime()),
    m_twilightNauticalBegin(QTime()),
    m_twilightNauticalEnd(QTime()),
    m_twilightAstronomicalBegin(QTime()),
    m_twilightAstronomicalEnd(QTime())
{
}

AstroData::~AstroData()
{
}

QDate AstroData::getDate() const
{
    return m_date;
}

void AstroData::setDate(const QDate& value)
{
    m_date = value;
}

QTime AstroData::getSunRise() const
{
    return m_sunRise;
}

void AstroData::setSunRise(const QTime& value)
{
    m_sunRise = value;
}

QTime AstroData::getSunSet() const
{
    return m_sunSet;
}

void AstroData::setSunSet(const QTime& value)
{
    m_sunSet = value;
}

QTime AstroData::dayDuration(ObserverPoint::hemisphere_t hemisphere) const
{
    if (!m_sunRise.isValid() || !m_sunSet.isValid())
    {
        switch (hemisphere)
        {
            case ObserverPoint::NORTH :
                return (betweenEquinox() ? QTime(23, 59, 59) : QTime(0, 0, 0));

            case ObserverPoint::SOUTHERN :
                return (betweenEquinox() ? QTime(0, 0, 0) : QTime(23, 59, 59));
        }
    }

    QTime time(0,0,0);
    return time.addSecs(86400 - m_sunSet.secsTo(m_sunRise));
}

QTime AstroData::nightDuration(ObserverPoint::hemisphere_t hemisphere) const
{
    if (!m_sunRise.isValid() || !m_sunSet.isValid())
    {
        switch (hemisphere)
        {
            case ObserverPoint::NORTH :
                return (betweenEquinox() ? QTime(0, 0, 0) : QTime(23, 59, 59));

            case ObserverPoint::SOUTHERN :
                return (betweenEquinox() ? QTime(23, 59, 59) : QTime(0, 0, 0));
        }
    }

    QTime time(0,0,0);
    return time.addSecs(m_sunSet.secsTo(m_sunRise));
}

QTime AstroData::getLunarRise() const
{
    return m_lunarRise;
}

void AstroData::setLunarRise(const QTime& value)
{
    m_lunarRise = value;
}

QTime AstroData::getLunarSet() const
{
    return m_lunarSet;
}

void AstroData::setLunarSet(const QTime& value)
{
    m_lunarSet = value;
}

AstroData::lunar_quarter_t AstroData::getLunarQuarter() const
{
    return m_lunarQuarter;
}

void AstroData::setLunarQuarter(AstroData::lunar_quarter_t value)
{
    m_lunarQuarter = value;
}

AstroData::lunar_quarter_t AstroData::nextLunarQuarter() const
{
    int result = static_cast<int>(m_lunarQuarter + 10);
    if (static_cast<int>(LUNAR_OLD) < result)
    {
        return LUNAR_NEW;
    }

    return static_cast<lunar_quarter_t>(result);
}

AstroData::lunar_quarter_t AstroData::previousLunarQuarter() const
{
    int result = static_cast<int>(m_lunarQuarter - 1);
    if (static_cast<int>(result < LUNAR_NEW))
    {
        return LUNAR_OLD;
    }

    return static_cast<lunar_quarter_t>(result + 10);
}

QTime AstroData::getTwilightCivilBegin() const
{
    return m_twilightCivilBegin;
}

void AstroData::setTwilightCivilBegin(const QTime& value)
{
    m_twilightCivilBegin = value;
}

QTime AstroData::getTwilightCivilEnd() const
{
    return m_twilightCivilEnd;
}

void AstroData::setTwilightCivilEnd(const QTime& value)
{
    m_twilightCivilEnd = value;
}

QTime AstroData::getTwilightNauticalBegin() const
{
    return m_twilightNauticalBegin;
}

void AstroData::setTwilightNauticalBegin(const QTime& value)
{
    m_twilightNauticalBegin = value;
}

QTime AstroData::getTwilightNauticalEnd() const
{
    return m_twilightNauticalEnd;
}

void AstroData::setTwilightNauticalEnd(const QTime& value)
{
    m_twilightNauticalEnd = value;
}

QTime AstroData::getTwilightAstronomicalBegin() const
{
    return m_twilightAstronomicalBegin;
}

void AstroData::setTwilightAstronomicalBegin(const QTime& value)
{
    m_twilightAstronomicalBegin = value;
}

QTime AstroData::getTwilightAstronomicalEnd() const
{
    return m_twilightAstronomicalEnd;
}

void AstroData::setTwilightAstronomicalEnd(const QTime& value)
{
    m_twilightAstronomicalEnd = value;
}

bool AstroData::betweenEquinox() const
{
    return ((QDate(m_date.year(), 3, 20) <= m_date) && (m_date <= QDate(m_date.year(), 9, 23)));
}


// class AstroDataCalendar
AstroDataCalendar::AstroDataCalendar()
  : m_tmspec( Qt::UTC )
{
  QString str1 = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
  QString str2 = QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmss");
  m_localdiff = QDateTime::fromString( str2, "yyyyMMddhhmmss" ).secsTo( QDateTime::fromString( str1, "yyyyMMddhhmmss" ) );
}

AstroDataCalendar::~AstroDataCalendar()
{
}

ObserverPoint& AstroDataCalendar::observerPoint()
{
    return m_observer;
}

const ObserverPoint& AstroDataCalendar::observerPoint() const
{
    return m_observer;
}

void AstroDataCalendar::clear()
{
    m_list.clear();
}

void AstroDataCalendar::makeUp(MakeFlags flags)
{
    AstroData::lunar_quarter_t previousLunarQuarter = getFirstlunarQuarter();

    QDate firstDate = m_observer.getFirstDateTime().date();
    ObsInfo position(
        m_observer.getLongitude(),
        m_observer.getLatitude(),
        0,
        m_observer.getHeight()
    );

    int daysTo = m_observer.daysTo();
    while (0 <= daysTo)
    {
        AstroData data(firstDate.addDays(m_observer.daysTo() - daysTo));
        double jd = data.getDate().toJulianDay() - .5;//.addDays(1).toJulianDay() - .5;
        TimePair timePair;

        QTime tm1;
        QTime tm2;

        if ( 0 != (flags & kMakeSun) ) {
          RiseSet::getTimes(timePair, RiseSet::SUN, jd, position);
          QTime tm1 = convertTimePaitToQTime(timePair.a);
          QTime tm2 = convertTimePaitToQTime(timePair.b);
          if ( Qt::LocalTime == m_tmspec ) {
            if ( true == tm1.isValid() ) {
              tm1 = tm1.addSecs(m_localdiff);
            }
            if ( true == tm2.isValid() ) {
              tm2 = tm2.addSecs(m_localdiff);
            }
          }
          data.setSunRise(tm1);
          data.setSunSet(tm2);
        }


        if ( 0 != (flags & kMakeMoon) ) {
          RiseSet::getTimes(timePair, RiseSet::MOON, jd, position);
          tm1 = convertTimePaitToQTime(timePair.a);
          tm2 = convertTimePaitToQTime(timePair.b);
          if ( Qt::LocalTime == m_tmspec ) {
            if ( true == tm1.isValid() ) {
              tm1 = tm1.addSecs(m_localdiff);
            }
            if ( true == tm2.isValid() ) {
              tm2 = tm2.addSecs(m_localdiff);
            }
          }
          data.setLunarRise(tm1);
          data.setLunarSet(tm2);
        }

        if ( 0 != (flags & kMakeLunarQuarter) ) {
          AstroData::lunar_quarter_t currentLunarQuarter = lunarQuarter(jd);
          if (currentLunarQuarter != AstroData::LUNAR_UNKNOWN)
          {
            data.setLunarQuarter(currentLunarQuarter);
            previousLunarQuarter = data.nextLunarQuarter();
          }
          else
          {
            data.setLunarQuarter(previousLunarQuarter);
          }
        }

        if ( 0 != (flags & kMakeAstronomicalTwi) ) {
          RiseSet::getTimes(timePair, RiseSet::ASTRONOMICAL_TWI, jd, position);
          tm1 = convertTimePaitToQTime(timePair.a);
          tm2 = convertTimePaitToQTime(timePair.b);
          if ( Qt::LocalTime == m_tmspec ) {
            if ( true == tm1.isValid() ) {
              tm1 = tm1.addSecs(m_localdiff);
            }
            if ( true == tm2.isValid() ) {
              tm2 = tm2.addSecs(m_localdiff);
            }
          }
          data.setTwilightAstronomicalBegin(tm1);
          data.setTwilightAstronomicalEnd(tm2);
        }

        if ( 0 != (flags & kMakeNautricalTwi) ) {
          RiseSet::getTimes(timePair, RiseSet::NAUTICAL_TWI, jd, position);
          tm1 = convertTimePaitToQTime(timePair.a);
          tm2 = convertTimePaitToQTime(timePair.b);
          if ( Qt::LocalTime == m_tmspec ) {
            if ( true == tm1.isValid() ) {
              tm1 = tm1.addSecs(m_localdiff);
            }
            if ( true == tm2.isValid() ) {
              tm2 = tm2.addSecs(m_localdiff);
            }
          }
          data.setTwilightNauticalBegin(tm1);
          data.setTwilightNauticalEnd(tm2);
        }

        if ( 0 != (flags & kMakeCivilTwi) ) {
          RiseSet::getTimes(timePair, RiseSet::CIVIL_TWI, jd, position);
          tm1 = convertTimePaitToQTime(timePair.a);
          tm2 = convertTimePaitToQTime(timePair.b);
          if ( Qt::LocalTime == m_tmspec ) {
            if ( true == tm1.isValid() ) {
              tm1 = tm1.addSecs(m_localdiff);
            }
            if ( true == tm2.isValid() ) {
              tm2 = tm2.addSecs(m_localdiff);
            }
          }
          data.setTwilightCivilBegin(tm1);
          data.setTwilightCivilEnd(tm2);
        }

        m_list.append(data);
        daysTo--;
    }
}

void AstroDataCalendar::setEmptyDay(int count)
{
    for (int i = 0; i < count; ++i)
    {
        m_list.append(AstroData());
    }
}

int AstroDataCalendar::daysTo() const
{
    return m_list.size();
}

AstroData& AstroDataCalendar::day(int index)
{
    return m_list[index];
}

const AstroData& AstroDataCalendar::day(int index) const
{
    return m_list[index];
}

void AstroDataCalendar::appendDay(const AstroData& item)
{
    m_list.append(item);
}

QString AstroDataCalendar::info() const
{
    QString name = (m_observer.getNameENG().isEmpty() ? m_observer.getNameRU() : m_observer.getNameENG());
    return QString("%1_%2_%3_%4_%5_%6")
        .arg(name.isEmpty() ? RU("unknown") : name)
        .arg(m_observer.getLatitude())
        .arg(m_observer.getLongitude())
        .arg(m_observer.getHeight())
        .arg(m_observer.getFirstDateTime().toString(DATE_FORMAT))
        .arg(m_observer.getSecondDateTime().toString(DATE_FORMAT));
}

QTime AstroDataCalendar::convertTimePaitToQTime(double timeJD)
{
    QTime time;
    if (timeJD < 0)
    {
        return time;
    }

    return QTime(0,0,0).addSecs(static_cast<long int>((timeJD * 24. * 3600.) + .5));
}

AstroData::lunar_quarter_t AstroDataCalendar::getFirstlunarQuarter()
{
    AstroData data(m_observer.getFirstDateTime().date());
    while (data.getLunarQuarter() == AstroData::LUNAR_UNKNOWN)
    {
        double jd = data.getDate().toJulianDay() - .5;
        data.setLunarQuarter(lunarQuarter(jd));
        data.setDate(data.getDate().addDays(1));
    }

    return data.previousLunarQuarter();
}

AstroData::lunar_quarter_t AstroDataCalendar::lunarQuarter(double timeJD)
{
    static PlanetData pd;
    double lunarLon[2], solarLon[2];
    ObsInfo position(m_observer.getLongitude(), m_observer.getLatitude());
    for (int j = 0; j < 2; j++)
    {
        pd.calc(EARTH, (timeJD + j), position);
        solarLon[j] = pd.eclipticLon();

        pd.calc(LUNA, (timeJD + j), position);
        lunarLon[j] = pd.eclipticLon();
    }

    int quad1 = RiseSet::quadrant(lunarLon[1] - solarLon[1] );
    if (quad1 != RiseSet::quadrant(lunarLon[0] - solarLon[0] ))
    {
        switch (quad1)
        {
            case 0 :
                return AstroData::LUNAR_NEW;

            case 1 :
                return AstroData::LUNAR_Q1;

            case 2 :
                return AstroData::LUNAR_FULL;

            case 3 :
                return AstroData::LUNAR_Q3;

            default :
                break ;
        }
    }

    return AstroData::LUNAR_UNKNOWN;
}
