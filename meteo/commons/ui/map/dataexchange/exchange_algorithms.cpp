#include "exchange_algorithms.h"

#include <algorithm>

namespace meteo {
namespace dataexchange {

char* fillWithValue(char* buffer, int bytesSize, char value)
{
  std::fill(buffer, buffer + bytesSize, value);
  return buffer;
}

const QByteArray serializeValue(const char* value, int bytesSize)
{
  QByteArray result;
  result.fill('\0', bytesSize);
  std::copy(value, value + bytesSize, result.data());
  return result;
}

int parseFromByteArray(const QByteArray& from, char* to, int bytesLength)
{
  int sz = bytesLength > from.size() ? from.size() : bytesLength;
  char* final = std::copy(from.data(), from.data() + sz, to);
  return static_cast<int>(final-to);
}

} // dataexchange
} // meteo
