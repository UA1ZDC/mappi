#ifndef func_sun_moon_h
#define func_sun_moon_h

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <QDateTime>


// // Структура, содержащая время, используются только первые 6 переменных
// struct tm {
//         int tm_sec;     // seconds after the minute - [0,59]
//         int tm_min;     // minutes after the hour - [0,59]
//         int tm_hour;    // hours since midnight - [0,23]
//         int tm_mday;    // day of the month - [1,31]
//         int tm_mon;     // months since January - [0,11]
//         int tm_year;    // years since 1900
// //      int tm_wday;    // days since Sunday - [0,6]
// //      int tm_yday;    // days since January 1 - [0,365]
// //      int tm_isdst;   // daylight savings time flag
//         };


// Вычисление восхода и захода солнца (везде время местное)
// вход:
// loc_time : локальное время (любое время текущего дня)
// time_zone : часовой сдвиг от гринвича (например для Москвы зимой=4, летом=5)
// f, l : широта и долгота местоположения (в градусах)
// выход:
// t_rise : указатель на число, которое будет содержать время восхода солнца (в часах)
// t_set  : указатель на число, которое будет содержать время захода солнца (в часах)
// возвращает:
// 1 : все ОК
//-1 : сегодня не всходит (весь день темно или полярная ночь)
//-2 : сегодня не заходит (весь день светло или полярный день)
void FillAmS(double t);
double getJG(struct tm *newtime);
double  getE(double T);
double  retlJ(double T);
double  retl1J(double T);
double  retl_Wn(double T);
double  retp_Wn(double T);
double  retl_Mar(double T);
double  retp_Mar(double T);
double  retl_Jup(double T);
double  retp_Jup(double T);
double  retl_Sat(double T);
double  retp_Sat(double T);
double  retFJ(double T);
double  retDJ(double T);
double  retl_Sl(double T);
double get_lon(double t);
double get_ri(double t);
double	star_time(double t);
void get_LaLo(double& La,double& Lo,double e);
int sun_rise_set(struct tm *loc_time,double time_zone,double f, double l, double *t_rise,double *t_set);
int getSunRise(QDate date_t, double d_fi0, double d_la0, int i_N,QTime &zah, QTime &vosh);
void setQTimePoDouble(QTime &tm,double &td);
int secToGrinv(double);
#endif
