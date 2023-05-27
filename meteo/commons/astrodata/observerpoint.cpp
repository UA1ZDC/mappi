#include "observerpoint.h"

using namespace astro;


ObserverPoint::ObserverPoint() :
    m_latitude(.0),
    m_longitude(.0),
    m_height(0),
    m_nameRU(QString()),
    m_nameENG(QString()),
    m_fdt(QDateTime::currentDateTimeUtc()),
    m_sdt(QDateTime::currentDateTimeUtc())
{
}

ObserverPoint::~ObserverPoint()
{
}

QString ObserverPoint::getNumber() const
{
    return m_number;
}

void ObserverPoint::setNumber(const QString& value)
{
    m_number = value;
}

QString ObserverPoint::getNameRU() const
{
    return m_nameRU;
}

void ObserverPoint::setNameRU(const QString& value)
{
    m_nameRU = value;
}

QString ObserverPoint::getNameENG() const
{
    return m_nameENG;
}

void ObserverPoint::setNameENG(const QString& value)
{
    m_nameENG = value;
}

double ObserverPoint::getLatitude() const
{
    return m_latitude;
}

void ObserverPoint::setLatitude(double value)
{
    m_latitude = value;
}

double ObserverPoint::getLongitude() const
{
    return m_longitude;
}

void ObserverPoint::setLongitude(double value)
{
    m_longitude = value;
}

int ObserverPoint::getHeight() const
{
    return m_height;
}

void ObserverPoint::setHeight(int value)
{
    m_height = value;
}

QDateTime ObserverPoint::getFirstDateTime() const
{
    return m_fdt;
}

void ObserverPoint::setFirstDateTime(const QDateTime& fdt)
{
    m_fdt = fdt;
}

QDateTime ObserverPoint::getSecondDateTime() const
{
    return m_sdt;
}

void ObserverPoint::setSecondDateTime(const QDateTime& sdt)
{
    m_sdt = sdt;
}

int ObserverPoint::daysTo()
{
    return m_fdt.daysTo(m_sdt);
}

ObserverPoint::hemisphere_t ObserverPoint::hemisphere() const
{
    return (0 < m_latitude ? NORTH : SOUTHERN);
}
