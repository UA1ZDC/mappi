#ifndef TEST_H
#define TEST_H

#include <qobject.h>

namespace meteo {
namespace rpc {

class Stub : public QObject
{
  Q_OBJECT
  public:
    Stub();
    ~Stub();

    bool connectToAddress( const QString& addr );
};

}
}

#endif
