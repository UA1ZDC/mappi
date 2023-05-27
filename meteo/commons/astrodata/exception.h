#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "defines.h"
#include <QString>
#include <stdexcept>


namespace astro
{

/** @brief Исключение */
class Exception :
    public std::logic_error
{
public :
    Exception(const QString& cause) :
        logic_error(RU_COUT(cause)) { }

    virtual ~Exception() throw() { }
};

}

#endif // _EXCEPTION_H_
