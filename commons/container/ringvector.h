#ifndef COMMONS_CONTAINER_RINGVECTOR_H
#define COMMONS_CONTAINER_RINGVECTOR_H

#include <qvector.h>

//! Кольцевой буфер на основе QVector.
template<class T> class RingVector
{
public:
  RingVector(){}

  //! Изменяет размер буфера на size.
  void resize(int size)
  {
    QVector<T> v;
    v = v_.mid(pos_);
    v += v_.mid(0,pos_);
    v.resize(size);
    pos_ = 0;
    v_ = v;
  }
  //! Возвращает размер буфера.
  inline int size() const                 { return v_.size(); }
  //! Возвращает true, если буфер содержит элемент value.
  inline bool contains(const T& t) const  { return v_.contains(t); }
  //! Возвращает ссылку на элемент буфера с индексом i.
  inline const T& operator[](int i) const { return v_[(pos_ + i) % v_.size()]; }
  //! Добавляет элемент в буфер.
  void insert(const T& t)
  {
    pos_ = (pos_ + 1) % v_.size();
    v_[pos_] = t;
  }

private:
  QVector<T> v_;

  int pos_ = 0;   // текущая позиция
};

#endif // COMMONS_CONTAINER_RINGVECTOR_H
