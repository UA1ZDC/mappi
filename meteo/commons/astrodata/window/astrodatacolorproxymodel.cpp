#include "astrodatacolorproxymodel.h"
#include <meteo/commons/astrodata/astrodatainfo.h>
#include <QColor>

using namespace astro;


AstroDataColorProxyModel::AstroDataColorProxyModel(QObject* parent /*=*/) :
    QIdentityProxyModel(parent)
{
}

AstroDataColorProxyModel::~AstroDataColorProxyModel()
{
}

int AstroDataColorProxyModel::rowCount(const QModelIndex& parent /*=*/) const
{
    return sourceModel()->rowCount(parent);
}

int AstroDataColorProxyModel::columnCount(const QModelIndex& parent /*=*/) const
{
    return sourceModel()->columnCount(parent);
}

QVariant AstroDataColorProxyModel::headerData(int section, Qt::Orientation orientation, int role /*=*/) const
{
    return sourceModel()->headerData(section, orientation, role);
}

Qt::ItemFlags AstroDataColorProxyModel::flags(const QModelIndex& index) const
{
    return sourceModel()->flags(index);
}

QVariant AstroDataColorProxyModel::data(const QModelIndex& index, int role /*=*/) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::BackgroundColorRole)
    {
        switch (index.column())
        {
            case AstroDataInfo::SUN_RISE :
            case AstroDataInfo::SUN_SET :
            case AstroDataInfo::DAY_DURATION :
                return QColor(255, 255, 0);

            case AstroDataInfo::NIGHT_DURATION :
            case AstroDataInfo::LUNAR_RISE :
            case AstroDataInfo::LUNAR_SET :
            case AstroDataInfo::LUNAR_QUARTER :
                return QColor(50, 100, 160);

            case AstroDataInfo::TWILIGHT_CIVIL_BEGIN :
            case AstroDataInfo::TWILIGHT_CIVIL_END :
                return QColor(253, 233, 217);

            case AstroDataInfo::TWILIGHT_NAUTICAL_BEGIN :
            case AstroDataInfo::TWILIGHT_NAUTICAL_END :
                return QColor(252, 213, 180);

            case AstroDataInfo::TWILIGHT_ASTRONOMICAL_BEGIN :
            case AstroDataInfo::TWILIGHT_ASTRONOMICAL_END :
                return QColor(250, 191, 143);

            default :
                break;
        }
    }

    return sourceModel()->data(index, role);
}
