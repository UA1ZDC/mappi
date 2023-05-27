#ifndef METEO_COMMONS_GLOBAL_LOG_H
#define METEO_COMMONS_GLOBAL_LOG_H

#include <qobject.h>

namespace meteo {
namespace msglog {

/* общие*/
const QString kDateTimeError = QObject::tr("Не удалось преобразовать строку %1 в QDateTime");

/* All Data service */

const QString kDataServiceNoData = QObject::tr("Данных, удовлетворяющих условию, не найдено");

const QString kDataNoDataSrok = QObject::tr("Недостаточно данных для анализа за срок %1 (центр %2)");

/* прогонзирование */

const QString kForecastNoMethods = QObject::tr("В каталоге %1 не обнаружено правил для работы ПК прогнозирования");

const QString kForecastNoStation = QObject::tr("Нет информации о пункте прогнозирования %1");
const QString kForecastAddStation = QObject::tr("Добавлена информация о новом пункте прогнозирования %1");

const QString kForecastSucess = QObject::tr("Результат прогнозирования величины %1 на уровне %2 (%3) на срок %4 методом %5 в пункте %6: %7");
const QString kForecastAccuracySucess = QObject::tr("Фактическое значение величины %1 за срок %2 на уровне %3 (%4) в пункте %5: %6");

/* Src Data service */

const QString kSrcBadRequest = QObject::tr("Запрос задан некорректно или ошибка выполнения запроса");
const QString kDbObanalNotReady = QObject::tr("База данных с результатами объективного анализа недоступна");
const QString kDbSrcNotReady = QObject::tr("База данных с результатами наблюдений недоступна");
const QString kDbSprinfNotReady = QObject::tr("База справочных данных недоступна");
const QString kDbForecastNotReady = QObject::tr("База данных с результатами прогнозирования недоступна");

/* Field data service */

const QString DB_NOT_READY = QObject::tr("База данных с результатами объективного анализа недоступна");
const QString kNoData = QObject::tr("Данных, удовлетворяющих условию, не найдено");
const std::string ALL_OK = QObject::tr("Запрос выполнен успешно.").toStdString();
const QString ALL_OK_1 = QObject::tr("Запрос выполнен успешно. Найдено %1 записи");

/* Работа с файловой системой */
  const QString kFileOpenError = QObject::tr("Ошибка при открытии файла %1. Ошибка: %2");
  const QString kFileNotFound = QObject::tr("Файл %1 не найден");
  const QString kFileReadFailed = QObject::tr("Не удалось прочитать файл %1. Ошибка: %2");
  const QString kFileWriteFailed = QObject::tr("Не удалось записать файл %1. Ошибка: %2");
  const QString kFileReadWriteFailed = QObject::tr("Не удалось открыть файл %1. Ошибка: %2");
  const QString kFileWriteSuccess = QObject::tr("Операция записи в файл %1 выполнена успешно");
  const QString kFileReadSuccess = QObject::tr("Операция чтения из файла %1 выполнена успешно");
  const QString kFileNewSuccess = QObject::tr("Создан файл %1");
  const QString kFileNewFailed = QObject::tr("Не удалось создать файл %1");
  const QString kDirCreateFailed = QObject::tr("Ошибка создания директории %1");
  const QString kDirReadFailed = QObject::tr("Ошибка чтения директории %1");
  const QString kDirWriteFailed = QObject::tr("Ошибка записи в директорию %1");
  const QString kDirNotFound = QObject::tr("Директория %1 не найдена");
  const QString kDirRemoveFailed = QObject::tr("Ошибка при удалении директории '%1'");
  const QString kFileRemoveFailed = QString::fromUtf8("Ошибка при удалении файла '%1': %2");
  const QString kFileWriteIncomplete = QString::fromUtf8("Данные в файл '%1' записаны не полностью");
  const QString kFileBytesFailed = QString::fromUtf8("Количество прочитанных байт не совпадает с требуемым");
  const QString kFileEndSurprise = QString::fromUtf8("Неожиданный конец файла");
  const QString kFileExistsAndRewrite = QString::fromUtf8("Файл '%1' уже существует и будет перезаписан");
  const QString kSaveDirNotSpec = QString::fromUtf8("Не задана директория для сохранения файлов");

/* Процессы */
  const QString kProcessNotFound = QObject::tr("Процесс %1 не найден");
  const QString kProcessFinishedFailed = QObject::tr("Процесс %1 завершил работу с ошибками");
  const QString kProcessFinishedSuccess = QObject::tr("Процесс %1 завершил работу без ошибок");
  const QString kProcessNotStarted = QObject::tr("Процесс %1 не запущен");
  const QString kProcessAlreadyStarted = QObject::tr("Процесс %1 уже запущен");

/* Сервисы */
  const QString kServiceFinishedFailed = QObject::tr("Сервис %1 завершил работу с ошибками");
  const QString kServiceDisconnect = QObject::tr("Соединение с сервисом %1 прервано");
  const QString kServiceDisconnectTryToReconnect = QObject::tr("Соединение с сервисом %1 прервано. Выполняется попытка возобновить соединение");
  const QString kServiceNotFound = QObject::tr("Сервис %1 не найден");
  const QString kSubscriptionNew = QObject::tr("Обнаружена новая подписка");
  const QString kArmNewFound = QObject::tr("Обнаружено новое АРМ с IP-адресом %1");
  const QString kMsgSentSuccess = QObject::tr("Сообщение успешно отправлено");
  const QString kMsgSentFailed = QObject::tr("Не удалось отправить сообщение");
  const QString kServiceCallError = QObject::tr("Ошибка взаимодействия с сервисом %1");
  const QString kServerCreateFailed = QObject::tr("Не удалось создать сервер %1");
  const QString kServerAnswerFailed = QObject::tr("Ответ от сервера %1 не получен");
  const QString kServerAnswerError = QObject::tr("Запрос к сервису %1 не выполнен. Описание ошибки: %2");
  const QString kServicePublishedSuccess = QObject::tr("Сервис %1 опубликован");
  const QString kServicePublishedFailed = QObject::tr("Не удалось опубликовать сервис %1");
  const QString kServiceConnectFailed = QObject::tr("Не удалось установить соединение с сервисом %1 по адресу %2:%3"); //host:port
  const QString kServiceConnectFailedSimple = QObject::tr("Не удалось установить соединение с сервисом %1"); //host:port
  const QString kServiceInfoAnswerFailed = QObject::tr("При попытке получить %1, ответ от сервиса %2 не получен");
  const QString kServiceAnalyzeAnswerFailed = QObject::tr("При попытке выполнить анализ %1 ответ от сервиса %2 не получен");
  const QString kServiceAnswerFailed = QObject::tr("Ответ от сервиса %1 не получен");
  const QString kServiceParamsFailed = QObject::tr("Неверно указаны параметры запуска: %1");
  const QString kServiceParamFailed = QObject::tr("Неверно указано значение параметра %1");
  const QString kServiceStatusUpdateFailed = QObject::tr("Не удалось отправить обновление статуса для сервиса %1");
  const QString kServiceRequestFailed = QObject::tr("Неверно заданы параметры запроса");
  const QString kServiceRequestInvalid = QObject::tr("Получены поврежденные данные от сервиса %1");
  const QString kServiceRequestFailedErr = QObject::tr("Превышно время ожидания или ошибка выполнения запроса");
  const QString kServiceRequestTime = QString::fromUtf8("Время обработки запроса %1: %2 мсек");
  const QString kServiceErrLoadData = QString::fromUtf8("Ошибка при загрузке данных");
  const QString kServiceNoData = QString::fromUtf8("Нет данных, соответствующих запросу");

  const QString kServerAnswerOK = QObject::tr("Запрос к сервису %1 выполнен успешно.");
  const QString kServerAnswerOKArg = QObject::tr("Запрос к сервису %1 выполнен успешно. Найдено %1 записи");
  const QString kServiceAnsverFailed = QObject::tr("Запрос к сервису %1 выполнен c ошибками");


  /* Менеджер сервисов */
  const QString kAppManagerPortEmpty = QObject::tr("Не указан сетевой порт для клиентов менеджера");
  const QString kAppManagerSocketEmpty = QObject::tr("Не указан путь к локальному сокету для клиентов менеджера");
  const QString kAppManagerNetServerCreateFailed = QObject::tr("Не удалось создать сетевой сервер связи с клиентами");
  const QString kAppManagerLocalServerCreateFailed = QObject::tr("Не удалось создать локальный сервер связи с клиентами");
  const QString kAppManageServersCreateFailed = QObject::tr("Не удалось создать локальный и сетевой серверы связи с клиентами");
  const QString kAppManageRollcallDisconnect = QObject::tr("Произошло отключение во время обхода подписчиков. Обход прерван");

/* Обслуживание БД */
  const QString kCleanerDirNotSpec = QObject::tr("Не задана директория размещения архивов");
  const QString kTelegramArchiveCreateFailed = QObject::tr("Ошибка при архивировании телеграмм '%1'");
  const QString kCleanerTaskTimeIncorrect = QObject::tr("Невозможно удалить устаревшие файлы, получено некорректное значение времени");
  const QString kCleanerTaskTemplateIncorrect = QObject::tr("Шаблон расписания %1 некорректен");

/* Запуск приложений по расписанию */
  const QString kCronBulletinFileNotSpec = QObject::tr("Не задано имя файла бюллетеня");

/* Декодер, факсы, телеграммы, alphanum, bufr... msgparser, msgsender, msgstream */
  const QString kDecoderLoadFormatsError = QObject::tr("Ошибка загрузки форматов сообщений");
  const QString kTelegramEmpty = QObject::tr("Получена пустое тело телеграммы %1: %2");
  const QString kFaxEmpty = QObject::tr("Получена пустое тело факса %1: %2");
  const QString kFaxUnzipFailed = QObject::tr("Процесс распаковки и сохранения факса %1 завершился неудачно");
  const QString kTelegramContentGetFailed = QObject::tr("Не удалось получить содержание телеграммы %1");
  const QString kTelegramDateEmpty = QObject::tr("Отсутствует информация о дате формирования телеграммы");
  const QString kTelegrambSaveFailed = QObject::tr("Ошибка при сохранении телеграммы в базу данных");
  const QString kTelegramSizeRuleNotFound = QObject::tr("Нет правил поиска размера сообщения типа %1, номер редакции %2");
  const QString kTelegramRuleLoadFailed = QObject::tr("Ошибка при загрузке формата сообщений %1 из файла %2");
  const QString kTemporaryFileOpenFailed = QObject::tr("Не удалось открыть временный файл для %1");
  const QString kSocketReadFailed = QObject::tr("Ошибка при чтении данных из сокета");
  const QString kTelegramMaxSizeExcess = QObject::tr("Невозможно передать телеграмму больше %1 байт");
  const QString kTelegramUnknownType = QObject::tr("Неизвестный тип телеграммы");
  const QString kCodeformNotLoadedAny = QObject::tr("Не загружена ни одна кодовая форма");
  const QString kCodeformUnknown = QObject::tr("Неизвестная кодовая форма '%1'");
  const QString kCodeformLoadFailed = QObject::tr("Ошибка загрузки кодовой формы из xml-файлов");
  const QString kDecodeTypeLoadFailed = QObject::tr("Ошибка считывания значения типа раскодировки %1");
  const QString kDecodeFailed =  QObject::tr("Ошибка раскодирования сводки '%1'. Заголовок:'%2'");
  const QString kMsgRegexpDoesntMatch = QObject::tr("Сообщение %1 не соответствует регулярному выражению %1");
  const QString kAlphanumFileStructError = QObject::tr("Ошибка структуры файла для сводки: %1");
  const QString kXmlDescriptorWrong = QObject::tr("Ошибка xml %1. Неверный дескриптор %2. Номер %3, текст: %4");
  const QString kXmlDescriptorError = QObject::tr("Ошибка раскодирования дескриптора '%1'");
  const QString kXmlDescriptorIndicatorError = QObject::tr("Ошибочное значение указателя %1: '%2', качество: '%3')");
  const QString kXmlRegexpNotSpec = QObject::tr("Ошибка структуры xml %1. Не указаны значения для выбора варианта регулярного выражения");
  const QString kDataCreateTimeNotSpec = QObject::tr("Нет информации о времени образования данных");
  const QString kDataTimeInvalid = QObject::tr("Некорректная дата h=%1 min=%2 y=%3 m=%4 d=%5");
  const QString kMsgHeaderReadFailed = QObject::tr("Ошибка чтения заголовка сообщения");
  const QString kMsgCodecNotSupported = QObject::tr("Кодек %1 не поддерживается");
  const QString kDescriptorLocalNameUnknown = QObject::tr("Неизвестное внутреннее название дескриптора %1");
  const QString kDescriptorAlreadyExists = QObject::tr("Дескриптор %1(%2) уже существует %3(%4)");
  const QString kDescriptorNotExists = QObject::tr("Дескриптор %1 не существует");
  const QString kMsgDecodingError = QObject::tr("Ошибки при раскодировании %1");
  const QString kDescriptorProcessingError = QObject::tr("Ошибка обработки дескриптора %1");
  const QString kBufrUncompressedDataOnly = QObject::tr("Реализация возможна только для несжатых данных");
  const QString kBufrSignGroupError = QObject::tr("Ошибка вертикальной значимости группы");
  const QString kBufrTableNotExists = QObject::tr("Нет таблицы %5 для: Центр=%1 Подцентр=%2 Эталонная таблица=%3 Местая таблица=%4");
  const QString kBufrVersionNotSupported = QObject::tr("Версия bufr %1 не поддерживается");
  const QString kBufrDescriptorRepeateError = QObject::tr("Ошибка. Повтор номера дескриптора номер %1, значение: %2");
  const QString kBufrDescriptorOrderError = QObject::tr("Неверная последовательность дескриптора %1 значение -группа = %2");
  const QString kDescriptorTempLevelIncorrect = QObject::tr("Не верно задан дескриптор температура='%1', уровень='%2'");
  const QString kXmlQualNotSpec = QObject::tr("Ошибка xml-файла %1. Не указан показатель контроля качества");
  const QString kBarometricTendencyControlError = QObject::tr("Ошибка контроля барической тенденции");
  const QString kXmlDescriptorNameNotSpec = QObject::tr("Ошибка xml файла %1. Нет имени дескриптора");
  const QString kValuesCountDoesntMatchLevelsCount = QObject::tr("Количество значений не совпадает с количеством уровней");
  const QString kMethodAlreadyExists = QObject::tr("Ошибка добавления метода %1: такой метод уже существует");
  const QString kLevelsCountDoesntMatchValuesCount = QObject::tr("Количество уровней (дескр. %1=%2) не совпадает с количеством значений (дескр.%3=%4)");
  const QString kDirFontsNotExists = QObject::tr("Директория со шрифтами %1 не существует");
  const QString kFontsNotFoundInDir = QObject::tr("Шрифты не найдены в директории %1");
  const QString kFontLoadFailed = QObject::tr("Не удалось загрузить шрифт из файла %1");
  const QString kGribGridFailed = QObject::tr("Ошибка формирования данных сетки. Номер шаблона %1");
  const QString kGribProdFailed = QObject::tr("Ошибка формирования данных продукта. Номер шаблона %1");
  const QString kGribDataLocalTblsOnly = QObject::tr("Данные содержат только локальные таблицы");
  const QString kGribVersionNotSupported = QObject::tr("Версия GRIB %1 не поддерживается");
  const QString kGribSectionDecodeFailed = QObject::tr("Ошибка раскодирования секции %1");
  const QString kGribUnregularGridNotSupported = QObject::tr("Нерегулярная сетка в базе данных не поддерживается");
  const QString kGribPrecisionNotSupported = QObject::tr("Точность %1 не поддерживается");
  const QString kDecodeDataFormatError = QObject::tr("Ошибка формата данных");
  const QString kGribVersionRuleIncorrect = QObject::tr("Некорректное правило получения версии %1");
  const QString kMsgKeyFieldNotFound = QObject::tr("В структуре на найдено поле %1 для ключа %2");
  const QString kGribPropertiesLoadFailed = QObject::tr("Ошибка получения свойств GRIB: %1");
  const QString kDataTypeNotSpec = QObject::tr("Не задан тип запрашиваемых данных");

/* RPC */

  const QString kNoConnect = QObject::tr("Не удалось подключиться к %1");
  const QString kSocketAddressIncorrect = QObject::tr("Тип адреса - не локальный сокет");
  const QString kServerCreateFailedSocketBusy = QObject::tr("Сокет %1 уже занят. не могу создать сервер");
  const QString kSocketDirCreateFailed = QObject::tr("Не могу создать директорию локального сокета %1");
  const QString kServerLocalCreateFailed = QObject::tr("Не могу создать локальный сервер");
  const QString kMsgSerializeFailed = QObject::tr("Не удалось сериализовать сообщение.");
  const QString kForeignChannelConnectFailed = QObject::tr("Канал в списке сторонних подключений, но соединение по каналу отсутствует.");
  const QString kForeignFinderServiceAnswerFailed = QObject::tr("Не удалось получить ответ от сторонней службы поиска по адресу %1");
  const QString kVisaIncorrect = QObject::tr("Некорректная визитка  %1");
  const QString kNetAddressesFoundFailed = QObject::tr("Нет сетевых адресов. К сторонним службам поиска подключение невозможно");
  const QString kDatagramSizeFailed = QObject::tr("Размер прочитанной датаграммы меньше минимального: %1 ");
  const QString kDatagramDecodingFailed = QObject::tr("Датаграмма не распознана. Содержимое датаграммы\n\t=");
  const QString kServicePortFailed = QObject::tr("Не удалось привязаться к порту %1. Служба поиска и публикации сервисов не работает");
  const QString kTransportFailed = QObject::tr("Не удалось создать транспорт");
  const QString kChannelFailed = QObject::tr("Непредвиденная ситуация. Канал для связи не установлен. Вызов метода невозможен. Необходимо удалить обработчик ответа.");
  const QString kServiceDescriptorLoadFailed = QObject::tr("Не удалось получить дескриптор сервиса");
  const QString kMethodDescriptorLoadFailed = QObject::tr("Не удалось получить дескриптор метода %1");
  const QString kRpcRequestIncorrect = QObject::tr("Некорректный запрос к сервису");;

/* Авторизация */
  const QString kSessionFinished = QObject::tr("Сессия окончена");
  const QString kAuthSuccess = QObject::tr("Авторизация %1 успешна");
  const QString kUserAddFailed = QObject::tr("Не удалось добавить пользователя %1");
  const QString kUserAddSuccess = QObject::tr("Пользователь %1 успешно добавлен");
  const QString kAuthFailed = QObject::tr("Авторизация пользователя %1 не удалась");
  const QString kUserAccessDenied = QObject::tr("Доступ для пользователя %1 запрещен"); // Обратитесь к администратору для регистрации пользователя %1
  const QString kAuthFormEmptyFields = QObject::tr("Заполните все поля для авторизации");
  const QString kAppParamsLoadFailedTryConnectDb = QObject::tr("При загрузке параметров приложения возникла ошибка: %1. Тем не менее попытка подключиться к базе %2 будет выполнена");
  const QString kAppParamsLoadFailed = QObject::tr("При загрузке параметров приложения возникла ошибка: %1");
  const QString kUserUnkwnon = QObject::tr("Не предоставлен идентификатор пользователя");
  const QString kUserUpdateFailed = QObject::tr("Не удается обновить информацию о пользователе %1");
  const QString kUserDeleteFailed = QObject::tr("Не удается удалить пользователя %1");

/* Приложения */
  const QString kAppStartSuccess = QObject::tr("Приложение %1 успешно запущено");
  const QString kAppStartFailed = QObject::tr("Не удалось запустить приложение %1");
  const QString kAppFinishSuccess = QObject::tr("Приложение %1 завершило работу");
  const QString kAppKill = QObject::tr("Приложение %1 завершило работу некорректно");
  const QString kAppFinishFailed = QObject::tr("Приложение %1 завершило работу с ошибками");
  const QString kAppNotFound = QObject::tr("Не найдено приложение %1");
  const QString kAppNotExecutable = QObject::tr("Файл приложения %1 не является исполняемым");
  const QString kAppFinish = QObject::tr("Приложение %1 завершилось c кодом возврата %2");
  const QString kAppDataWriteFailed = QObject::tr("Ошибка передачи данных в приложение %1");

/* Исключения - critical */
  const QString kNullPointer = QObject::tr("Нулевой указатель %1");
  const QString kMemoryAllocFailed = QObject::tr("Не удалось выделить память %1");

/* БД */
  const QString kDbConnectFailed = QObject::tr("Не удалось установить соединение с базой данных %1");
  const QString kDbConnectHostPortFailed = QObject::tr("Не удалось установить соединение с базой данных %1:%2");
  const QString kDbConnectSuccess = QObject::tr("Установлено соединение с базой данных %1");
  const QString kDbDisconnect = QObject::tr("Завершено соединение с базой данных %1");
  const QString kDbRequestFailed = QObject::tr("Не удалось выполнить запрос к базе данных");
  const QString kDbCursorCreationFailed = QObject::tr("Не удается создать курсор из запроса");
  const QString kDbRequestFailedArg = QObject::tr("Ошибка выполнения запроса = %1");
  const QString kDbRequestNotFound = QObject::tr("Не удается найти шаблон запроса к базе данных: %1");    

  const QString kDbRequestSuccess = QObject::tr("Запрос к базе данных выполнен успешно");
  const QString kDbConnectionNotFound = QObject::tr("Подключение '%1' не найдено");
  const QString kDbConnectionNotSpec = QObject::tr("Подключение к базе данных '%1' не задано");

  const QString kDbSaveInnerDocSuccess = QObject::tr("Операция сохранения %1 в БД выполнена успешно");

  /* JSON */
  const QString kJsonArrayFieldValueIncorrect = QObject::tr("Ошибка: не удается получить данные из массива json");
  const QString kJsonFieldNotFound = QObject::tr("Не удается получить поле из документа: %1");
  const QString kJsonSingleFieldExpected = QObject::tr("Ошибка: ожидалось ровно 1 значение.");
  const QString kJsonToProtoFailed = QObject::tr("Ошибка при при преобразовании JSON в proto: %1");

/* Настройки */
  const QString kSettingsSaveSuccess = QObject::tr("Настройки %1 успешно сохранены");
  const QString kSettingsSaveFailed = QObject::tr("Не удалось сохранить настройки %1");
  const QString kSettingsLoadSuccess = QObject::tr("Настройки %1 успешно загружены");
  const QString kSettingsLoadFailed = QObject::tr("Не удалось загрузить настройки %1");
  const QString kSettingsIncorrect = QObject::tr("Настройки %1 некорректны");
  const QString kSettingsDirNotFound = QObject::tr("Отсутствует директория с конфигурационными файлами '%1'");
  const QString kSettingsIsoNotFound = QObject::tr("Не найдена настройка изолинии для дескриптора %1");

/* Резервное копирование и восстановление */
  const QString kDumpStart = QObject::tr("Начинаю создание резервной копии...");
  const QString kDumpDbFinishSuccess = QObject::tr("Создание архива базы данных %1 успешно выполнено");
  const QString kDumpDbFinishFailed = QObject::tr("При создании архива базы данных %1 произошла ошибка");
  const QString kDumpDbStart = QObject::tr("Создание архива базы данных %1...");
  const QString kDumpDirFinishSuccess = QObject::tr("Создание архива файлов %1 успешно выполнено");
  const QString kDumpDirFinishFailed = QObject::tr("При создании архива файлов %1 произошла ошибка");
  const QString kDumpDirStart = QObject::tr("Создание архива файлов %1...");
  const QString kDumpFinish = QObject::tr("Создание резервной копии завершено");
  const QString kReserveSaveDirNotSpec = QObject::tr("Укажите директорию для сохранения резервной копии");
  const QString kReserveLoadDirNotSpec = QObject::tr("Укажите путь к директории с резервной копией");
  const QString kRestoreStart = QObject::tr("Начинаю восстановление из резервной копии...");
  const QString kRestoreDumpStart = QObject::tr("Восстановление базы данных %1...");
  const QString kRestoreDumpFinishSuccess = QObject::tr("Восстановление базы данных %1 успешно выполнено");
  const QString kRestoreDumpFinishFailed = QObject::tr("При восстановлении базы данных %1 произошла ошибка");
  const QString kRestoreDirStart = QObject::tr("Восстановление файлов %1...");
  const QString kRestoreDirFinishSuccess = QObject::tr("Восстановление файлов %1 успешно выполнено");
  const QString kRestoreDirFinishFailed = QObject::tr("При восстановлении файлов %1 произошла ошибка");
  const QString kRestoreFinish = QObject::tr("Восстановление из резервной копии окончено");
  const QString kReserveConfigFileNotFound = QObject::tr("Отсутствует конфигурационный файл '%1'");
  const QString kReserveConfigFileParseError = QObject::tr("Ошибка при обработке конфигурационного файла '%1'");
  const QString kReserveConfigFileOpenError = QObject::tr("Ошибка при открытии файла конфигурации '%1'");

/* Карта*/
  const QString kMapCreateFailed = QObject::tr("Не удалось загрузить географическую основу %1");

/* Без категории */
  const QString kDecodingFailed = QObject::tr("Ошибка раскодирования %1");
  const QString kDecodingSuccess = QObject::tr("Раскодирование %1 успешно");
  const QString kBulletinBlankBadFormat = QObject::tr("Неверный формат бланка бюллетеня");
  const QString kTarExecError = QObject::tr("Ошибка при взаимодействии с архиватором");
  const QString kDateFormatIncorrect = QObject::tr("Неверный формат даты = %1");
  const QString kDateBeginIncorrect = QObject::tr("Начальная дата некорректна");
  const QString kDateEndIncorrect = QObject::tr("Дата окончания некорректна");
  const QString kGuiIncorrectElement = QObject::tr("Не найден %1. Возврат");

  const QString kNoSelectedStation = QObject::tr("Не выбрана станция!");

  const QString kImageCreateFailed = QObject::tr("Ошибка при обработке изображений");

  const QString kMeteoDataRangeWarning = QObject::tr("Значение %1 не соответствует диапазону метеопараметра %2");

/* Предупреждения, вопросы пользователю */
//настройка изолиний
  const QString kSettingsChanged = QObject::tr("Настройки были изменены. Сохранить перед закрытием?");
  const QString kRemove =  QObject::tr("Удалить %1?");
  const QString kSetValue = QObject::tr("Введите значение от %1 до %2");
  const QString kSetPositiveValue = QObject::tr("Введите значение больше нуля и меньше %1");
  const QString kSetIsoName = QObject::tr("Укажите название изолинии");
  const QString kNameIsolineNotFound = QObject::tr("Изолиния с названием %1 не найдена");
  const QString kIsolineAlreadyAdded = QObject::tr("Элемент с индексом %1 уже существует, заменить?");
  const QString kRemoveLevel = QObject::tr("Удалить \"Уровень %1, тип уровня %2\" изолинии \"%3\"?");
  const QString kIsoSelect = QObject::tr("Необходимо выбрать изолинию");
  const QString kIsoNotFound = QObject::tr("Нет данных для отображения или неправильные настройки изолиний");

  //сохранение
  const QString kSaveChooseDest = QObject::tr("Необходимо указать место сохранения");

  /* Плагины */
  const QString kTargetMenyNotFound = QObject::tr("Не удается найти пункт меню %1. Часть действий будет недоступна.");

} // msglog
} // meteo

#endif // METEO_COMMONS_GLOBAL_LOG_H
