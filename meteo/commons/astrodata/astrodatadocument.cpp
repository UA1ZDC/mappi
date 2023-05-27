#include "astrodatadocument.h"
#include "astrodatainfo.h"
#include "defines.h"

using namespace astro;

const int AstroDataDocument::MAX_ROWS_ON_PAGE = 31;


AstroDataDocument::AstroDataDocument(const AstroDataCalendar& calendar) :
    m_calendar(calendar)
{
    if (m_calendar.daysTo() <= MAX_ROWS_ON_PAGE)
    {
        m_pageList.append(m_calendar.daysTo());
    }
    else
    {
        QDate fDate = m_calendar.observerPoint().getFirstDateTime().date();
        QDate sDate = m_calendar.observerPoint().getSecondDateTime().date();
        int daysTo = fDate.daysTo(sDate);
        if (0 < daysTo)
        {
            // по месяцам целиком, кол-во строк зависит от кол-ва дней в месяце
            if (fDate.day() == 1)
            {
                QDate nextDate = fDate;
                while (nextDate < sDate)
                {
                    m_pageList.append(sDate.month() == nextDate.month() ?
                        sDate.day() :
                        nextDate.daysInMonth()
                    );
                    nextDate = nextDate.addMonths(1);
                }
            }
            // по 31 дню на странице, на посленей остаток
            else
            {
                int days = 0;
                while (days < daysTo)
                {
                    m_pageList.append(daysTo < (days + MAX_ROWS_ON_PAGE) ?
                        daysTo - days + 1 :
                        MAX_ROWS_ON_PAGE
                    );
                    days += MAX_ROWS_ON_PAGE;
                }
            }
        }
    }
}

AstroDataDocument::~AstroDataDocument()
{
}

int AstroDataDocument::pageCount() const
{
    return m_pageList.count();
}

QString AstroDataDocument::pageToHtml(int number) const
{
    QStringList rowTable;
    rowTable.append(
        tableRow(
            QStringList()
            << cellHeader(RU("Дата"), 2, 1)
            << cellHeader(RU("Солнце"), 1, 2)
            << cellHeader(RU("День"), 2, 1)
            << cellHeader(RU("Ночь"), 2, 1)
            << cellHeader(RU("Луна"), 1, 3)
            << cellHeader(RU("Гражданские сумерки"), 1, 2)
            << cellHeader(RU("Навигационные сумерки"), 1, 2)
            << cellHeader(RU("Астрономические сумерки"), 1, 2)
        )
    );
    rowTable.append(
        tableRow(
            QStringList()
            << cellHeader(RU("восход"))
            << cellHeader(RU("заход"))
            << cellHeader(RU("восход"))
            << cellHeader(RU("заход"))
            << cellHeader(RU("фаза"))
            << cellHeader(RU("утренние"))
            << cellHeader(RU("вечерние"))
            << cellHeader(RU("утренние"))
            << cellHeader(RU("вечерние"))
            << cellHeader(RU("утренние"))
            << cellHeader(RU("вечерние"))
        )
    );

    int countItem = m_pageList.at(number);
    int offset = pageOffset(number);
    for (int item = 0; item < countItem; ++item)
    {
        const AstroData& data = m_calendar.day(item + offset);
        const AstroDataInfo& dataInfo(data);

        rowTable.append(
            tableRow(
                QStringList()
                    << cellData(dataInfo.date())
                    << cellData(dataInfo.sunRise())
                    << cellData(dataInfo.sunSet())
                    << cellData(dataInfo.dayDuration(m_calendar.observerPoint().hemisphere()))
                    << cellData(dataInfo.nightDuration(m_calendar.observerPoint().hemisphere()))
                    << cellData(dataInfo.lunarRise())
                    << cellData(dataInfo.lunarSet())
                    << cellData(AstroData::lunarQuarterName(data.getLunarQuarter()))
                    << cellData(AstroDataInfo::toString(data.getTwilightCivilBegin()))
                    << cellData(AstroDataInfo::toString(data.getTwilightCivilEnd()))
                    << cellData(AstroDataInfo::toString(data.getTwilightNauticalBegin()))
                    << cellData(AstroDataInfo::toString(data.getTwilightNauticalEnd()))
                    << cellData(AstroDataInfo::toString(data.getTwilightAstronomicalBegin()))
                    << cellData(AstroDataInfo::toString(data.getTwilightAstronomicalEnd()))
            )
        );
    }

    const ObserverPoint& point = m_calendar.observerPoint();
    QString timespecstr = ( Qt::UTC == m_calendar.timeSpec() ) ? QObject::tr("UTC") : QObject::tr("Местное время");
    return html(QString("%1\n%2\n%3")
        .arg(number != 0 ? QString() :
            paragraph(
                RU("Станция:"),
                RU("№ %1 - %2(%3)")
                    .arg(point.getNumber())
                    .arg(point.getNameRU())
                    .arg(point.getNameENG())
            )
        )
        .arg(number != 0 ? QString() :
            paragraph(
                RU("Географические координаты:"),
                RU("%1; %2; высота %3 м.")
                    .arg(point.getLatitude())
                    .arg(point.getLongitude())
                    .arg(point.getHeight())
            )
        )
        .arg(
            table(
                rowTable,
                RU("Астрономические данные за период %1 - %2. (%3)")
                .arg(AstroDataInfo::toString(point.getFirstDateTime().date()))
                .arg(AstroDataInfo::toString(point.getSecondDateTime().date()))
                .arg(timespecstr)
            )
        )
    );
}

QString AstroDataDocument::html(const QString& content) const
{
    return QString("<html><body>%1</body></html>").arg(content);
}

QString AstroDataDocument::table(const QStringList& rowList, const QString& caption) const
{
    return QString("<table border=\"0\" cellspacing=\"1\" bgcolor=\"#000000\" %3>"
        "<caption>%2</caption>"
            "%1"
        "</table>")
        .arg(rowList.join("\n"))
        .arg(caption)
        .arg("width=\"100%\"");
}

QString AstroDataDocument::tableRow(const QStringList& cellList) const
{
    return QString("<tr bgcolor=\"#FFFFFF\">%1</tr>")
        .arg(cellList.join("\n"));
}

QString AstroDataDocument::cellHeader(const QString& value, int rowSpan /*=*/, int colSpan /*=*/) const
{
    return QString("<th rowspan=\"%1\" colspan=\"%2\">%3</th>")
            .arg(rowSpan)
            .arg(colSpan)
            .arg(value);
}

QString AstroDataDocument::cellData(const QString& value, const QString& align /*=*/) const
{
    return QString("<td align=\"%1\">%2</td>")
        .arg(align)
        .arg(value);
}

QString AstroDataDocument::paragraph(const QString& name, const QString& value) const
{
    return QString("<p><strong>%1</strong> %2</p>")
        .arg(name)
        .arg(value);
}

int AstroDataDocument::pageOffset(int number) const
{
    int offset = 0;
    for (int i = 0; i < m_pageList.size(); ++i)
    {
        if (i == number)
        {
            break ;
        }
        offset += m_pageList.at(i);
    }
    return offset;
}
