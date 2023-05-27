#ifndef MN_COMMON_FUNCS
#define MN_COMMON_FUNCS

//! для всякой всячины, но обязательно общей для всех :)
namespace MnCommon {
  template< typename T, template<typename> class Container > void reverseContainer( Container<T>* data );
  void reverseArray( char* data, const int dsize, const int usize );
  QString splitDigits( int digit, int grouplength = 3, QChar delim = QChar(' ') );
}

/*!
 * \brief Перевернуть массив data
 * \param data Массив данных
 * \param dsize Размер массива
 */
template<class T> void MnCommon::reverseArray( T* data, const int dsize ) {
  if ( 0 == data || 0 >= dsize ) {
    return;
  }
  T brick;
  const int nbrick = sizeof(T);
  for ( int i = 0, j = dsize-1; i < j; ++i, --j ) {
    memcpy( &brick, data + i, nbrick );
    memcpy( data + i, data + j, nbrick );
    memcpy( data + j, &brick, nbrick );
  }
}

template< typename T, template<typename> class Container > void MnCommon::reverseContainer( Container<T>* data )
{
  for ( int i = 0, j = data->size() - 1; i < j; ++i, --j ) {
    T brick = (*data)[i];
    (*data)[i] = (*data)[j];
    (*data)[j] = brick;
  }
}

#endif

