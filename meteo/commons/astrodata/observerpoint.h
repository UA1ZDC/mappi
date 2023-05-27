#ifndef _OBSERVER_POINT_H_
#define _OBSERVER_POINT_H_

#include <QString>
#include <QDateTime>


namespace astro
{

/** @brief Географическое место наблюдения за астрономическими данными */
class ObserverPoint
{
public :
    enum hemisphere_t
    {
        NORTH = 0,
        SOUTHERN
    };

public :
    ObserverPoint();
    ~ObserverPoint();

    QString getNumber() const;
    void setNumber(const QString& value);

    QString getNameRU() const;
    void setNameRU(const QString& value);

    QString getNameENG() const;
    void setNameENG(const QString& value);

    double getLatitude() const;
    void setLatitude(double value);

    double getLongitude() const;
    void setLongitude(double value);

    int getHeight() const;
    void setHeight(int value);

    QDateTime getFirstDateTime() const;
    void setFirstDateTime(const QDateTime& fdt);

    QDateTime getSecondDateTime() const;
    void setSecondDateTime(const QDateTime& sdt);

    int daysTo();

    hemisphere_t hemisphere() const;

private :
    QString m_number;
    double m_latitude;
    double m_longitude;
    int m_height;

    QString m_nameRU;
    QString m_nameENG;

    QDateTime m_fdt;
    QDateTime m_sdt;
};

}

#endif // _OBSERVER_POINT_H_
