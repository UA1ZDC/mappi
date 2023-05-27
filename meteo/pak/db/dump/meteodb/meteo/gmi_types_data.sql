--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.10
-- Dumped by pg_dump version 9.6.10

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

--
-- Data for Name: gmi_types; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (64, 0, 'Приземные наблюдения', '[{"type": 2, "description": "Приземные наблюдения с фиксированной береговой станции"}, {"type": 1, "description": "Приземные наблюдения с фиксированной наземной станции (FM12)"}, {"type": 39, "description": "Агронаблюдения"}, {"type": 23, "description": "Наблюдения за поверхностью моря по маршруту судна (FM62)"}, {"type": 22, "description": "Приземные наблюдения с морской станции (FM13)"}, {"type": 21, "description": "Приземные наблюдения с подвижной наземной станции (FM14)"}, {"type": 13, "description": "Данные с наземной станции за декаду (КН-19, ДЕКАДА)"}, {"type": 12, "description": "Месячные значения с наземной станции(FM71)"}, {"type": 5, "description": "Данные снегосъемки (КН-24)"}, {"type": 4, "description": "Оперативная сводка об опасных гидрометеорологических явлениях (WAREP)"}, {"type": 3, "description": "Приземные наблюдения (+ за одночасовой период) с фиксированной наземной станции"}, {"type": 6, "description": "Другие приземные наблюдения из BUFR (synop полноценный и с частью данных; 10-минутные интервалы)"}, {"type": 7, "description": "Данные наземных ГНСС"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (65, 40, 'Aэродромные наблюдения', '[{"type": 41, "description": "Регулярная сводка по аэродрому (FM15)"}, {"type": 42, "description": "Специальная сводка по аэродрому (FM16)"}, {"type": 44, "description": "Предупреждения по аэродрому (AD WRNG)"}, {"type": 45, "description": "Предупреждения о сдвиге ветра по аэродрому (WS WRNG)"}, {"type": 51, "description": "Прогноз по аэродрому (FM51)"}, {"type": 52, "description": "Текстовый прогноз по аэродрому (АРМ-ВГМ)"}, {"type": 53, "description": "Краткосрочный прогноз (TREND)"}, {"type": 54, "description": "Уточнение прогноза (FM51)"}, {"type": 46, "description": "Штормовое оповещение (АРМ-ВГМ)"}, {"type": 47, "description": "Штормовое предупреждение (АРМ-ВГМ)"}, {"type": 48, "description": "Аэродромные (ГМЦ)"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (66, 60, 'Аэрологические наблюдения', '[{"type": 85, "description": "Сводка данных о ветре на высотах с подвижной наземной станции (FM34)"}, {"type": 62, "description": "Температурно-ветровое зондирование (Метео-44)"}, {"type": 71, "description": "Аэрологические наблюдения с фиксированной станции в слое (СЛОЙ)"}, {"type": 84, "description": "Сводка данных о ветре на высотах с морской станции (FM33)"}, {"type": 81, "description": "Аэрологические наблюдения с морской станции (FM36)"}, {"type": 82, "description": "Аэрологические наблюдения с зонда, сбрасываемого с аэростата-носителя или самолета (FM37)"}, {"type": 83, "description": "Аэрологические наблюдения с подвижной назменой станции (FM38)"}, {"type": 65, "description": "Метеосредний (Метео11)"}, {"type": 61, "description": "Аэрологические наблюдения с фиксированной наземной станции (FM35)"}, {"type": 63, "description": "Сводка данных о ветре на высотах с фиксированной наземной станции (FM32)"}, {"type": 86, "description": "Аэрологические данные BUFR. Фиксированная или подвижная или морская"}, {"type": 66, "description": "Профилометр ветра"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (67, 100, 'Гидрологические наблюдения', '[{"type": 101, "description": "Сводка данных о гидрологическом наблюдении с гидрологической стнации (FM67, КН-15)"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (68, 120, 'Океанологические наблюдения', '[{"type": 121, "description": "Наблюдения с буя (FM18)"}, {"type": 123, "description": "Cводка данных с морской станции о температуре, солености и течении (КН-05, FM64)"}, {"type": 122, "description": "Сводка данных батитермического наблюдения (FM63, КН-06)"}, {"type": 124, "description": "Измеритель волн цунами"}, {"type": 125, "description": "Другие данные BUFR"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (69, 140, 'Данные о тропических циклонах', '[{"type": 141, "description": "Информация о тропическом циклоне. Наземные радиолокационные наблюдения (FM20, Часть A)"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (70, 150, 'Радиолокационные данные на карте', '[{"type": 151, "description": "Информация о характерных особенностях. Наземные радиолокационные наблюдения (FM20, Часть B)"}, {"type": 152, "description": "Измерения радиолокатора (ветер, отражаемость)"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (71, 160, 'Геофизические данные', '[{"type": 161, "description": "Прогноз геомагнитной возмущенности (ИОНФО)"}, {"type": 180, "description": "Консультативная информация о вулканическом пепле (VAA)"}, {"type": 162, "description": "IONEX"}, {"type": 163, "description": "GTEX"}, {"type": 164, "description": "IONKA"}, {"type": 165, "description": "Суточный Ак и Ap-индекс (AFRED, APFRD, APMOS)"}, {"type": 166, "description": "Магнитометрические наблюдения (UMAGF, МАГКС)"}, {"type": 167, "description": "Спутниковые SATIN, OPPCM "}, {"type": 168, "description": "Плотность потока радиоизлучения F10.7 (TENCM, TENMN)"}, {"type": 169, "description": "Плотность потока радиоизлучения (URANJ)"}, {"type": 170, "description": "Солнечная активность (GEOALERT)"}, {"type": 171, "description": "Солнечные пятна (USSPS, USSPT, USSPI, USSPY)"}, {"type": 172, "description": "Солнечные вспышки (UFLAE)"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (72, 190, 'Данные по районам полетной информации', '[{"type": 191, "description": "Особые явления по маршруту полета (SIGMET, AIRMET)"}, {"type": 201, "description": "Прогноз погоды для полетов на нижних эшелонах (GAMET)"}, {"type": 202, "description": "SIGWX"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (73, 210, 'Донесения с борта самолёта', '[{"type": 211, "description": "Донесения с борта (AIREP, AIREP SPECIAL)"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (74, 220, 'Спутник, вертикальное зондирование', '[{"type": 221, "description": "Аэрология по уровням (спутник)"}, {"type": 222, "description": "ГНСС (спутник)"}, {"type": 223, "description": "Аэрология по одному уровню (спутник)"}]');
INSERT INTO meteo.gmi_types (id, gmitype, description, subtype) VALUES (75, 230, 'Спутник, приземные', '[{"type": 231, "description": "Скаттерометр"}, {"type": 232, "description": "Альтиметр"}, {"type": 233, "description": "Альтиметр, радиометр"}, {"type": 234, "description": "Другие призменые спутниковые данные"}]');


--
-- Name: gmi_types_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.gmi_types_id_seq', 75, true);


--
-- PostgreSQL database dump complete
--

