#include "astrodatafile.h"
#include "astrodatainfo.h"
#include "exception.h"
#include <QStringList>
#include <QTextStream>

using namespace astro;


AstroDataFile::AstroDataFile(const QString& filePath)
{
    m_file.setFileName(filePath);
}

AstroDataFile::~AstroDataFile()
{
}

void AstroDataFile::load(AstroDataCalendar& calendar)
{
    if (!m_file.open(QFile::ReadOnly))
    {
        throw Exception(m_file.errorString());
    }

    QTextStream in(&m_file);
    QString timespecstr;
    loadObserver(in.readLine(), calendar.observerPoint(), &timespecstr );
    if ( -1 != timespecstr.indexOf("UTC") ) {
      calendar.setTimeSpec( Qt::UTC );
    }
    else {
      calendar.setTimeSpec( Qt::LocalTime );
    }
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.isEmpty() || !line.contains(DATA_FILE_SEPARATOR))
        {
            continue ;
        }

        AstroData item;
        loadItem(line, item);
        calendar.appendDay(item);
    }
}

void AstroDataFile::save(const AstroDataCalendar& calendar)
{
    if (!m_file.open(QFile::WriteOnly))
    {
        throw Exception(m_file.errorString());
    }

    QString timespecstr = ( Qt::UTC == calendar.timeSpec() ) ? QObject::tr("UTC") : QObject::tr("Местное время");

    QTextStream out(&m_file);
    out << saveObserver(calendar.observerPoint(), timespecstr ) << endl;
    out << endl;
    out << header() << endl;

    int countItem = calendar.daysTo();
    for (int item = 0; item < countItem; ++item)
    {
        out << saveItem(calendar.day(item), calendar.observerPoint().hemisphere()) << endl;
    }
}

void AstroDataFile::loadItem(const QString& line, AstroData& item)
{
    QStringList list = line.split(DATA_FILE_SEPARATOR);
    if (list.size() != AstroDataInfo::COLUMN_COUNT)
    {
        return ;
    }

    item.setDate(AstroDataInfo::toDate(list.at(AstroDataInfo::DATE)));
    item.setSunRise(AstroDataInfo::toTime(list.at(AstroDataInfo::SUN_RISE)));
    item.setSunSet(AstroDataInfo::toTime(list.at(AstroDataInfo::SUN_SET)));
    // no usage
    // AstroDataInfo::DAY_DURATION
    // AstroDataInfo::NIGHT_DURATION
    item.setLunarRise(AstroDataInfo::toTime(list.at(AstroDataInfo::LUNAR_RISE)));
    item.setLunarSet(AstroDataInfo::toTime(list.at(AstroDataInfo::LUNAR_SET)));
    item.setLunarQuarter(AstroData::lunarQuarter(list.at(AstroDataInfo::LUNAR_QUARTER).simplified()));
    item.setTwilightCivilBegin(AstroDataInfo::toTime(list.at(AstroDataInfo::TWILIGHT_CIVIL_BEGIN)));
    item.setTwilightCivilEnd(AstroDataInfo::toTime(list.at(AstroDataInfo::TWILIGHT_CIVIL_END)));
    item.setTwilightNauticalBegin(AstroDataInfo::toTime(list.at(AstroDataInfo::TWILIGHT_NAUTICAL_BEGIN)));
    item.setTwilightNauticalEnd(AstroDataInfo::toTime(list.at(AstroDataInfo::TWILIGHT_NAUTICAL_END)));
    item.setTwilightAstronomicalBegin(AstroDataInfo::toTime(list.at(AstroDataInfo::TWILIGHT_ASTRONOMICAL_BEGIN)));
    item.setTwilightAstronomicalEnd(AstroDataInfo::toTime(list.at(AstroDataInfo::TWILIGHT_ASTRONOMICAL_END)));
}

QString AstroDataFile::saveItem(const AstroData& item, ObserverPoint::hemisphere_t hemisphere)
{
    QStringList result;
    result << AstroDataInfo::toString(item.getDate())
        << AstroDataInfo::toString(item.getSunRise(), true)
        << AstroDataInfo::toString(item.getSunSet(), true)
        << AstroDataInfo::toString(item.dayDuration(hemisphere), true)
        << AstroDataInfo::toString(item.nightDuration(hemisphere), true)
        << AstroDataInfo::toString(item.getLunarRise(), true)
        << AstroDataInfo::toString(item.getLunarSet(), true)
        << AstroData::lunarQuarterName(item.getLunarQuarter()).leftJustified(11, ' ')
        << AstroDataInfo::toString(item.getTwilightCivilBegin(), true)
        << AstroDataInfo::toString(item.getTwilightCivilEnd(), true)
        << AstroDataInfo::toString(item.getTwilightNauticalBegin(), true)
        << AstroDataInfo::toString(item.getTwilightNauticalEnd(), true)
        << AstroDataInfo::toString(item.getTwilightAstronomicalBegin(), true)
        << AstroDataInfo::toString(item.getTwilightAstronomicalEnd(), true);
    return result.join(DATA_FILE_SEPARATOR);
}

void AstroDataFile::loadObserver(const QString& line, ObserverPoint& observer, QString* timespecstr )
{
    QStringList list = line.split(DATA_FILE_SEPARATOR);
    if (list.size() != 9)
    {
        return ;
    }

    observer.setNumber(list.at(0));
    observer.setNameRU(list.at(1));
    observer.setNameENG(list.at(2));
    observer.setLatitude(list.at(3).toDouble());
    observer.setLongitude(list.at(4).toDouble());
    observer.setHeight(list.at(5).toInt());
    observer.setFirstDateTime(QDateTime(AstroDataInfo::toDate(list.at(6)), QTime()));
    observer.setSecondDateTime(QDateTime(AstroDataInfo::toDate(list.at(7)), QTime()));
    *timespecstr = list.at(8);
}

QString AstroDataFile::saveObserver(const ObserverPoint& observer, const QString& timespecstr )
{
    QStringList result;
    result << QString("%1").arg(observer.getNumber())
        << observer.getNameRU()
        << observer.getNameENG()
        << QString("%1").arg(observer.getLatitude())
        << QString("%1").arg(observer.getLongitude())
        << QString("%1").arg(observer.getHeight())
        << AstroDataInfo::toString(observer.getFirstDateTime().date())
        << AstroDataInfo::toString(observer.getSecondDateTime().date())
        << timespecstr;
    return result.join(DATA_FILE_SEPARATOR);
}

QString AstroDataFile::header() const
{
    return RU(
    "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
    "|            |          Солнце              |              |               |                     Луна                      |     Гражданские сумерки       |     Навигационные сумерки     | Астрономические сумерки    |\n"
    "|    Дата    |------------------------------|     День     |      Ночь     |-----------------------------------------------|-------------------------------|-------------------------------|----------------------------|\n"
    "|            |   восход    |      заход     |              |               |     восход    |     заход      |     фаза     |     начало    |     конец     |     начало    |     конец     |    начало      |    конец  |\n"
    "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"
    );
}
