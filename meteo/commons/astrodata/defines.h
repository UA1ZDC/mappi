#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <QString>

#define RU(text) QString::fromUtf8(text)
#define RU_COUT(text) text.toLocal8Bit().data()

#define DATE_FORMAT "dd.MM.yyyy"
#define TIME_FORMAT "hh:mm:ss"
#define TIME_FAIL "    --:--:--    "

#define DATA_FILE_SEPARATOR "\t"
#define INTERVAL_SEPARATOR "-"
#define DATA_FILE_EXT ".astro"

#define LUNAR_NEW_NAME RU("новолуние")
#define LUNAR_Q1_NAME RU("1 четверть")
#define LUNAR_FULL_NAME RU("полнолуние")
#define LUNAR_Q3_NAME RU("3 четверть")
#define LUNAR_YOUNG_NAME RU("молодая")
#define LUNAR_WAXING_NAME RU("прибывающая")
#define LUNAR_WANING_NAME RU("убывающая")
#define LUNAR_OLD_NAME RU("старая")

#endif // _DEFINES_H_
