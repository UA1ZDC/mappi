#ifndef COMMONS_CONTAINER_THASH_H
#define COMMONS_CONTAINER_THASH_H

#include <qhash.h>


template < class Key, class T > class THash : public QHash< Key, T >
{
public:
  inline THash<Key,T>& operator<<( const QPair<Key,T>& t ){
    this->insert(t.first,t.second);
    return *this;
  }
};


#endif // COMMONS_CONTAINER_THASH_H
