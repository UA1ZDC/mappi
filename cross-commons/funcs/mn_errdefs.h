#ifndef MN_ERROR_NUMS
#define MN_ERROR_NUMS

//Нет ошибок
#define ERR_NOERR        0
//Ошибка с номером -1 пропущена намеренно. Т.к. это очень частый код возврата системных функций.

//Неверный параметр (напр, нулевой указатель, неверный тип из перечисления)
#define ERR_PARAM       -2
//Нерабочее состояние процесса
#define ERR_BADPROC     -3
//
#define ERR_LOGIN       -4
//
#define ERR_UNKNOWN     -5
//Ошибка инициализации объекта
#define ERR_INIT        -6
//Нет ошибок - перезапускать нить не надо
#define ERR_NOERR_NOT_RESTART        -7
//Нет доступа
#define ERR_ACCESS         -8
//Не поддерживается (не реализванно)
#define ERR_UNSUPPORT   -9
//Ошибка выделения памяти
#define ERR_MEMORY      -10
//Недостаточно данных
#define ERR_NODATA  -11
//функция неопределена
#define ERR_NOFUNC  -12


//Ошибка открытия файла
#define ERR_OPENFILE  -100
//Неверный формат файла (например, при попытке причитать настройки из не xml-файла)
#define ERR_FILE_TYPE -101
//Искомых данных в файле нет
#define ERR_NOINFILE  -102
//Невернй тип/значение данных, прочитанных из файла
#define ERR_FILEDATA  -103
//Ошибка чтения файла
#define ERR_READFILE  -104
//Ошибка сканирования директории
#define ERR_SCANDIR   -105
//Ошибка открытия директории
#define ERR_OPENDIR   -106
//Ошибка записи в сокет
#define ERR_WRITESOCK -107
//Ошибка запирания семафора
#define ERR_SEMLOCK   -108
//Ошибка освобождения семафора
#define ERR_SEMUNLOCK -109
//Ошибка чтения из сокета
#define ERR_READSOCK  -110
//Ошибка открытия сокета
#define ERR_SOCKET    -111
//Ошибка перемещения по файлу
#define ERR_SEEKFILE  -112
//Ошибка записи в файл
#define ERR_WRITEFILE -113
//Ошибка
#define ERR_READHEADER -114
//Ошибка создания директории
#define ERR_CREATEFILE -115
#define ERR_CREATEDIR ERR_CREATEFILE
#define ERR_FILEPATH   -116

//Ошибка вызова select
#define ERR_SELECT_SOCKET -117

//select закончил работу по таймауту
#define ERR_SELECT_TIMEOUT -118

//Ошибка. Файл не найден
#define ERR_SOCKET_PARAM   -119

//Ошибка чтения настроек
#define ERR_READSETT  -200
//Некорректные настройки
#define ERR_SETTINGS  -201

//Специфические ошибки в классах, входящих
//в библиотеку mn_uisettings
#define ERR_XMLSTRUCTURE -300


//Ошибка ftp
#define ERR_FTPERR  -400
//не известный ответ ftp сервера
#define ERR_FTPREQUEST -401
//неподдерживаемый фтп сервер (непонятный ответ сервера)
#define ERR_FTPFORMAT -402
//неподдерживаемый тип хоста
#define ERR_HOSTTYPE -403
//не создан поток приемник или получатель - gstream
#define ERR_GSTREAM -404
//нет файлов для скачивания
#define ERR_NOFTPFILE -405
//нет файлов для скачивания
#define ERR_INCORRECTLOGIN -406

//ошибки mappi
//Ошибка.Не возможно определить спутник по id. Неизвестный спутник.
#define ERR_SATID -500
//Ошибка определения положения спутника.
#define ERR_SATPOS -501
//Нет данных канала
#define ERR_NO_CHAN  -502
//Нет инфо данных
#define ERR_NO_INFO  -503
//синхра не найдена
#define ERR_NO_SYNCHRO  -504
//ошибка открытия устройства
#define ERR_OPEN_DEV  -505
//ошибка остановки устройства
#define ERR_STOP_DEV  -506
//ошибка очистки буферов устройства
#define ERR_CLEAR_DEV  -507
//ошибка установки параметров устройства
#define ERR_START_DEV  -508
//ошибка чтения временной метки из потока
#define ERR_TIME_SEARCH  -509
//данные уже загружены
#define ERR_ALREADY_EXIST  -510
//процесс остановлен
#define ERR_PROCESS_STOPED  -511
//ошибка открытия файла со станциями
#define ERR_NO_STATIONS_DATA  -512
//ошибка открытия файла с метеоданными
#define ERR_NO_METEO_DATA  -513

#define ERR_READTLE     -514

#define ERR_PROCESS_RESTARTED  -515

#define  ERR_PROCESS_PAUSE -516
//
#define ERR_SCHEDULE_OBSOLETE  -517

//ошибки ptkpp
#define UNIMAS_NO_ASK -600
#define UNIMAS_NO_DATA -601
#define ERR_PTKADDR_NOTFOUND -602

#define ERR_DBCONNECT -603
#define ERR_DBSAVE -604
#define ERR_DBEXEC_QUERY -605
#define ERR_DBINSERT -606
#define ERR_NODB -607

//Ошибка загрузки карты
#define ERR_LOAD_GEODATA  -700
#define ERR_SETDOCMAP     -701

#define ERR_THREAD_LOCK -800
//Ошибка создания нити
#define ERR_PTHREADCREAT -801

#define ERR_NOBUFFER -900
#define ERR_UNKNOW_MESS_TYPE -901
#define ERR_NO_HEADER -902


//Для  АРМа
#define ERR_TYPE_NET -1000
#define ERR_NODATA_OBANAL -1001 //нет данных для анализа
#define ERR_OBANAL -1002 //ошика анализа
#define ERR_NO_NEAR_DATA -1003 //по близости нет данных 


#define ERR_NO_TLG_DATA -1005 //Нет информации в сводке
#define ERR_NO_WIND_DATA -1006 //Нет информации о ед. изм. ветра
#define ERR_NO_STATION_DATA -1007 //Нет информации о станции
#define ERR_TYPE_TLG_DATA -1008 //Строка не того типа
#define ERR_DATA -1009 //Данные не того типа или ошибка в телеграмме

//#define GEOSTROF_ERROR -1010 //Объект вышел за область действия геострофических соотношений!
//#define DATA_PERENOS_ERROR -1011 //Отсутствуют данные для переноса!
#define ERR_MODEL_NOT_CORRECT -1012 //неподдерживаемый тип модели
#define ERR_TIME_NOT_CORRECT -1013 //не верно задано время
#define ERR_DATE_NOT_CORRECT -1014 //не верно задана дата 
#define ERR_DATE_TIME_NOT_CORRECT -1015 //не верно задано дата и время
#define ERR_DATA_SOURCE_NOT_CORRECT -1016 //не верно задан источник данных
#define ERR_POINT_OUT_OF_MAP -1017 //точка вне пределов карты


#define ERR_NOTGRIB -1100 //данные не формата GRIB

namespace MnCommon {
  const char* getErrStr(int num);
}


#endif //MN_ERROR_NUMS

