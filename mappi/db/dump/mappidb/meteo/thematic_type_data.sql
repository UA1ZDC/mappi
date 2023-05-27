--
-- PostgreSQL database dump
--

-- Dumped from database version 13.6 (Debian 13.6-1.pgdg110+1)
-- Dumped by pg_dump version 14.2 (Debian 14.2-1.pgdg110+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Data for Name: thematic_type; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_type (id, name, title) VALUES (1, 'Grayscale', 'Оттенки серого');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (2, 'FalseColor', 'Псевдоцвета');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (3, 'Airmass', 'Воздушные массы');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (4, 'Dust', 'Пыль');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (5, 'Ash', 'Пепел');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (6, 'Microphysics', 'Микрофизика (24ч)');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (7, 'DayMicrophysics', 'Микрофизика (день)');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (8, 'NightMicrophysics', 'Микрофизика (ночь)');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (9, 'SevereStorm', 'Шторм');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (10, 'Snow', 'Снег');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (11, 'Natural', 'Естественные цвета');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (12, 'Clouds', 'Облака');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (13, 'Fog', 'Туман');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (14, 'TrueColor', 'Настоящий цвет');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (15, 'CloudPhase', 'Фаза облаков');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (16, 'CloudType', 'Тип облаков');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (17, 'NDVI', 'Вегетационный индекс');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (18, 'Index8', 'Пользовательский');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (19, 'Grayscale1', 'Оттенки серого');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (23, 'CloudMask', 'Маска облачности');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (34, 'kTotalPrecip', 'Полное влагосодержание');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (24, 'kCloudType', 'Типы облаков');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (25, 'kCloudTempr', 'Температура верхней границы облаков');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (26, 'kCloudAlt', 'Высота верхней границы облаков');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (27, 'SeaTempr', 'Температура поверхности');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (32, 'kSnowBorder', 'Граница снежного покрова');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (33, 'kIceBorder', 'Граница льда на поверхности океана');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (36, 'kCloudFull', 'Вся облачность');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (35, 'kSeaIceConc', 'Сплоченность морского льда');


--
-- Name: thematic_type_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.thematic_type_id_seq', 21, true);


--
-- PostgreSQL database dump complete
--

