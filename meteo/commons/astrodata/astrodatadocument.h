#ifndef _ASTRO_DATA_DOCUMENT_H_
#define _ASTRO_DATA_DOCUMENT_H_

#include "astrodata.h"
#include <QList>


namespace astro
{

/** @brief Документ для печати */
class AstroDataDocument
{
public :
    static const int MAX_ROWS_ON_PAGE;

public :
    AstroDataDocument(const AstroDataCalendar& calendar);
    ~AstroDataDocument();

    int pageCount() const;
    QString pageToHtml(int number) const;

private :
    QString html(const QString& content) const;
    QString table(const QStringList& rowList, const QString& caption) const;
    QString tableRow(const QStringList& cellList) const;
    QString cellHeader(const QString& value, int rowSpan = 1, int colSpan = 1) const;
    QString cellData(const QString& value, const QString& align = "center") const;
    QString paragraph(const QString& name, const QString& value) const;

    int pageOffset(int number) const;

private :
    const AstroDataCalendar& m_calendar;
    QList<int> m_pageList;
};

}

#endif // _ASTRO_DATA_DOCUMENT_H_
