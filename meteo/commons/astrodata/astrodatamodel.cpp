#include "astrodatamodel.h"
#include "astrodatainfo.h"
#include "defines.h"

#include <cross-commons/debug/tlog.h>

using namespace astro;


AstroDataModel::AstroDataModel(AstroDataCalendar* calendar, QObject* parent /*=*/) :
    QAbstractTableModel(parent),
    m_calendar(calendar)
{
}

AstroDataModel::~AstroDataModel()
{
}

int AstroDataModel::rowCount(const QModelIndex& parent /*=*/) const
{
    Q_UNUSED(parent)
    return m_calendar->daysTo();
}

int AstroDataModel::columnCount(const QModelIndex& parent /*=*/) const
{
    Q_UNUSED(parent)
    return AstroDataInfo::COLUMN_COUNT;
}

QVariant AstroDataModel::headerData(int section, Qt::Orientation orientation, int role /*=*/) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case AstroDataInfo::DATE :
                return RU("Дата");

            case AstroDataInfo::SUN_RISE :
                return RU("Солнце восход");

            case AstroDataInfo::SUN_SET :
                return RU("Солнце закат");

            case AstroDataInfo::DAY_DURATION :
                return RU("День");

            case AstroDataInfo::NIGHT_DURATION :
                return RU("Ночь");

            case AstroDataInfo::LUNAR_RISE :
                return RU("Луна восход");

            case AstroDataInfo::LUNAR_SET :
                return RU("Закат закат");

            case AstroDataInfo::LUNAR_QUARTER :
                return RU("Фаза");

            case AstroDataInfo::TWILIGHT_CIVIL_BEGIN :
            case AstroDataInfo::TWILIGHT_NAUTICAL_BEGIN :
            case AstroDataInfo::TWILIGHT_ASTRONOMICAL_BEGIN :
                return RU("утренние");

            case AstroDataInfo::TWILIGHT_CIVIL_END :
            case AstroDataInfo::TWILIGHT_NAUTICAL_END :
            case AstroDataInfo::TWILIGHT_ASTRONOMICAL_END :
                return RU("вечерние");

             default :
                 return QVariant();
         }
     }

     return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags AstroDataModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::ItemIsEnabled;
    }


    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (1 < rowCount())
    {
        flags |= Qt::ItemIsSelectable;
    }

    return (QAbstractTableModel::flags(index) | flags);
}

QVariant AstroDataModel::data(const QModelIndex& index, int role /*=*/) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    AstroDataInfo info(m_calendar->day(index.row()));
    switch (role)
    {
        case Qt::TextAlignmentRole :
            return Qt::AlignCenter;

        case Qt::DisplayRole :
            switch (index.column())
            {
                case AstroDataInfo::DATE :
                    return info.date();

                case AstroDataInfo::SUN_RISE :
                    return info.sunRise();

                case AstroDataInfo::SUN_SET :
                    return info.sunSet();

                case AstroDataInfo::DAY_DURATION :
                    return info.dayDuration(m_calendar->observerPoint().hemisphere());

                case AstroDataInfo::NIGHT_DURATION :
                    return info.nightDuration(m_calendar->observerPoint().hemisphere());

                case AstroDataInfo::LUNAR_RISE :
                    return info.lunarRise();

                case AstroDataInfo::LUNAR_SET :
                    return info.lunarSet();

                case AstroDataInfo::LUNAR_QUARTER :
                    return QString("    ");

                case AstroDataInfo::TWILIGHT_CIVIL_BEGIN :
                    return QString("  %1  ").arg(info.twilightCivilBegin());

                case AstroDataInfo::TWILIGHT_CIVIL_END :
                    return QString("  %1  ").arg(info.twilightCivilEnd());

                case AstroDataInfo::TWILIGHT_NAUTICAL_BEGIN :
                    return QString("  %1  ").arg(info.twilightNauticalBegin());

                case AstroDataInfo::TWILIGHT_NAUTICAL_END :
                    return QString("  %1  ").arg(info.twilightNauticalEnd());

                case AstroDataInfo::TWILIGHT_ASTRONOMICAL_BEGIN :
                    return QString("  %1  ").arg(info.twilightAstronomicalBegin());

                case AstroDataInfo::TWILIGHT_ASTRONOMICAL_END :
                    return QString("  %1  ").arg(info.twilightAstronomicalEnd());

                default :
                    return QVariant();
            }
            break;

        case Qt::DecorationRole :
            if (index.column() == AstroDataInfo::LUNAR_QUARTER)
                return info.lunarQuarterIcon();
            break ;

        case Qt::ToolTipRole :
            if (index.column() == AstroDataInfo::LUNAR_QUARTER)
                return info.lunarQuarterName();
            break ;

        default :
            break ;
    }

    return QVariant();
}

void AstroDataModel::update()
{
  beginResetModel();
  resetInternalData();
  endResetModel();
}
