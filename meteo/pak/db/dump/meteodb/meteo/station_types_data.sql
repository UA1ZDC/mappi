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
-- Data for Name: station_types; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (189, 4, 'Аэрологическая подвижная станция', 80, 99, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (190, 5, 'Гидрологический пост', 100, 119, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (191, 6, 'Океан (по глубинам)', 120, 139, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (192, 7, 'Радиолокационная система, передающая информацию о тропических циклонах', 140, 149, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (193, 8, 'Радиолокационная система, данные на карте', 150, 159, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (194, 9, 'Геофизическая', 160, 179, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (195, 0, 'Синоптическая фиксированная наземная станция', 0, 19, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (196, 1, 'Синоптическая подвижная станция', 20, 39, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (197, 3, 'Аэрологическая фиксированная наземная станция', 60, 79, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (198, 10, 'Консультативные центры по вулканическому пеплу', 180, 180, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (199, 11, 'Районы полетной информации', 190, 209, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (200, 12, 'Данные с борта самолёта', 210, 219, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (201, 13, 'Справочник станций для обмена служебными сообщениями', 1000, 1000, '');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (202, 14, 'Аэродромная система (военный)', 58, 58, 'Наши');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (203, 15, 'Спутник, вертикальное зондирование', 220, 229, 'Спутник, вертикальное зондирование');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (204, 16, 'Спутник, приземные данные', 230, 239, 'Спутник, приземные данные');
INSERT INTO meteo.station_types (id, type, description, min_data_type, max_data_type, comment) VALUES (188, 2, 'Аэродромная система', 40, 57, '');


--
-- Name: station_types_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.station_types_id_seq', 204, true);


--
-- PostgreSQL database dump complete
--

