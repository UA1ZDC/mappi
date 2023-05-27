#ifndef _ASTRO_DATA_FILE_H_
#define _ASTRO_DATA_FILE_H_

#include "astrodata.h"
#include <QFile>


namespace astro
{

/** @brief Файл астрономических данных */
class AstroDataFile
{
public :
    AstroDataFile(const QString& filePath);
    ~AstroDataFile();

    void load(AstroDataCalendar& calendar);
    void save(const AstroDataCalendar& calendar);

private :
    void loadItem(const QString& line, AstroData& item);
    QString saveItem(const AstroData& item, ObserverPoint::hemisphere_t hemisphere);

    void loadObserver(const QString& line, ObserverPoint& observer, QString* timespecstr );
    QString saveObserver(const ObserverPoint& observer, const QString& timespecstr );

    QString header() const;

private :
    QFile m_file;
};

}

#endif // _ASTRO_DATA_FILE_H_
