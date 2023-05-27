#ifndef METEO_COMMONS_UI_MAP_DATAEXCHANGE_SERIALIZETOBYTEARRAY_H
#define METEO_COMMONS_UI_MAP_DATAEXCHANGE_SERIALIZETOBYTEARRAY_H

#include <QByteArray>

namespace meteo {
namespace dataexchange {


/*!
 * \brief:  Заполняет bytesSize байт, начиная с адреса buffer, значениями value.
 * \param:  buffer - указатель на буфер, который необходимо заполнить значениями.
 * \param:  bytesSize - размер заполняемого буфера в байтах.
 * \param:  value - значение, которым должен быть заплнен буфер.
 * \return: указатель на заполненный буфер.
 */
char* fillWithValue(char* buffer, int bytesSize, char value = '\0');

/*!
 * \brief:  Реализует побайтовое копирование bytesSize байт, начиная с адреса value.
 * \param:  value - указатель на сериализуемые данные.
 * \param:  bytesSize - размер сериализуемых данных в байтах.
 * \return: глубокая копия данных в виде массива байтов.
 */
const QByteArray serializeValue(const char* value, int bytesSize);

/*!
 * \brief:  Перегрузка для указателей любого типа
 */
template <typename T>
const QByteArray serializeValue(const T* value, int bytesSize)
{
  const void* pv = value;
  return serializeValue(static_cast<const char*>(pv), bytesSize);
}

/*!
 * \brief:  Перегрузка для ссылок любого типа (NB: размер типа можно не передавать)
 */
template <typename T>
const QByteArray serializeValue(const T& value, int bytesSize = sizeof(T))
{
  const void* pv = &value;
  return serializeValue(static_cast<const char*>(pv), bytesSize);
}

/*!
 * \brief:  Реализует побайтовое копирование bytesLength байт, но не более чем from.size() байт,
 *          из байтового массива from в память, начиная с адреса to.
 * \param:  from - ссылка на источник данных.
 * \param:  to - указатель на начало буфера для записи.
 * \param:  bytesLength - размер копируемых данных в байтах.
 * \return: количество фактически скопированных данных в байтах.
 */
int parseFromByteArray(const QByteArray& from, char* to, int bytesLength);

/*!
 * \brief:  Перегрузка для указателей любого типа
 */
template <typename T>
int parseFromByteArray(const QByteArray& from, T* to, int bytesLength)
{
  void* pv = to;
  return parseFromByteArray(from, static_cast<char*>(pv), bytesLength);
}

/*!
 * \brief:  Перегрузка для ссылок любого типа (NB: размер типа можно не передавать)
 */
template <typename T>
int parseFromByteArray(const QByteArray& from, T& to, int bytesLength = sizeof(T))
{
  void* pv = &to;
  return parseFromByteArray(from, static_cast<char*>(pv), bytesLength);
}


} // dataexchange
} // meteo

#endif // METEO_COMMONS_UI_MAP_DATAEXCHANGE_SERIALIZETOBYTEARRAY_H
