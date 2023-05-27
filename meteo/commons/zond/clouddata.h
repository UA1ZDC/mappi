#ifndef CLOUDDATA_H
#define CLOUDDATA_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/geobasis/geovector.h>

#include <qlist.h>
#include <qvector.h>

namespace zond {

enum CloudDataType {
    h,
    Nh,
    N,
    CL,
    CM,
    CH,
    C_LAST
};

class CloudData {
public:

    CloudData();

    void set (CloudDataType type, float value, int quality );
    void setC (float value, int quality );
    
    float value( CloudDataType type );

    bool isGood ( CloudDataType type );

    void print();

    void clear();

    int station_index;
    QDateTime datetime;
    friend QDataStream &operator<<(QDataStream &out, const CloudData &mtp);
    friend QDataStream &operator>>(QDataStream &in, CloudData &mtp);
    
private :
    QPair<float,int> *pair ( CloudDataType type );
    //значение, показатель качества
    QPair<float, int> _N;  //!< общее количество облачности
    QPair<float, int> _Nh;  //!< количество облачности
    QPair<float, int> _h;  //!< высота облачности
    QPair<float, int> _CL;  //!< тип облаков нижнего яруса
    QPair<float, int> _CM;  //!< тип облаков среднего яруса
    QPair<float, int> _CH;  //!< тип облаков верхнего яруса
};
}//zond

#endif
