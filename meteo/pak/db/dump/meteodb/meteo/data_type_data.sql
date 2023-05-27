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
-- Data for Name: data_type; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.data_type (id, datatype, description) VALUES (40, 0, 'Продукция анализа');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (41, 1, 'Продукция прогноза');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (42, 2, 'Продукция анализа и прогноза');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (43, 3, 'Продукция проконтролированного прогноза');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (44, 4, 'Продукция возмущенного прогноза');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (45, 5, 'Продукция проконтролированного и возмущенного прогноза');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (46, 6, 'Обработанные спутниковые наблюдения');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (47, 7, 'Обработанные радиолокационные наблюдения');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (48, 8, 'Зарезервированы');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (49, 191, 'Зарезервированы');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (50, 192, 'Зарезервированы для местного использования');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (51, 254, 'Зарезервированы для местного использования');
INSERT INTO meteo.data_type (id, datatype, description) VALUES (52, 255, 'Отсутствующее');


--
-- Name: data_type_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.data_type_id_seq', 52, true);


--
-- PostgreSQL database dump complete
--

