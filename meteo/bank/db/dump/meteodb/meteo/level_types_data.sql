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
-- Data for Name: level_types; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (233, 6, 'Уровень максимального ветра', 'Уровень максимального ветра', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (234, 7, 'Тропопауза', 'Тропопауза', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (235, 901, 'Отличие прогноза от диагноза', 'Отличие прогноза от диагноза', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (236, 1, 'Земная или водная поверхность', 'Земная или водная поверхность', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (237, 9, 'Морское дно', 'Морское дно', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (238, 201, 'Весь океан', 'Весь океан (считается единым слоем)', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (239, 15000, 'OT500/1000', 'OT500/1000', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (240, 4, 'Уровень изотермы 0 °C', 'Уровень изотермы 0 °C', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (241, 10, 'Вся атмосфера', 'Вся атмосфера (считается единым слоем)', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (242, 160, 'Глубина ниже уровня моря', 'Глубина ниже уровня моря', '{5,10,15,20,25,30,40,50,100,150,200,250,300,400}');
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (243, 20, 'Изотермический уровень', 'Изотермический уровень', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (244, 11, 'Основание кучево-дождевого облака', 'Основание кучево-дождевого облака', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (245, 12, 'Вершина кучево-дождевого облака', 'Вершина кучево-дождевого облака', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (246, 210, 'Изобарическая поверхность (высокая точность)', 'Изобарическая поверхность (высокая точность)', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (247, 2, 'Уровень основания облаков', 'Уровень основания облаков', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (248, 106, 'Глубина от поверхности земли', 'Глубина ниже поверхности земли', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (249, 3, 'Уровень вершин облаков', 'Уровень вершин облаков', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (250, 100, 'Изобарическая поверхность', 'Изобарическая поверхность', '{5,7,10,20,30,50,70,100,150,200,250,300,400,500,700,850,925,1000}');
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (251, 101, 'Средний уровень моря', 'Средний уровень моря', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (252, 102, 'Высота над уровнем моря', 'Установленная высота над средним уровнем моря', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (253, 103, 'Высота над поверхностью', 'Установленный уровень высоты над поверхностью', '{2,10}');
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (254, 104, 'слой между двумя уровнями', 'слой между двумя уровнями высоты выше  СУМ', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (255, 105, 'Фиксированная высота над землей', 'Фиксированная высота над землей', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (256, 107, 'Уровень сигмы', 'Уровень сигмы', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (257, 108, 'Уровень между двумя уровнями сигмы', 'Уровень между двумя уровнями сигмы', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (258, 109, 'Гибридный уровень', 'Гибридный уровень', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (259, 110, 'Слой между двумя гибридными уровнями', 'Слой между двумя гибридными уровнями', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (260, 111, 'Глубина ниже поверхности земли', 'Глубина ниже поверхности земли', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (262, 113, 'Изоэнтропический (тета) уровень', 'Изоэнтропический (тета) уровень', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (263, 114, 'Слой между двумя изоэнтропическими уровнями', 'Слой между двумя изоэнтропическими уровнями', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (264, 121, 'Слой между двумя изобарическими поверхностями', 'Слой между двумя изобарическими поверхностями', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (265, 200, 'Вся атмосфера', 'Вся атмосфера', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (266, 141, 'Слой между двумя изобарическими поверхностями', 'Слой между двумя изобарическими поверхностями', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (267, 125, 'Высота над землей', 'Высота над землей', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (268, 128, 'Слой между двумя уровнями сигмы', 'Слой между двумя уровнями сигмы', NULL);
INSERT INTO meteo.level_types (id, type, description, full_descr, standard_value) VALUES (261, 112, 'Геопотенциальная высота', 'Геопотенциальная высота, гп.м.', NULL);


--
-- Name: level_types_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.level_types_id_seq', 268, true);


--
-- PostgreSQL database dump complete
--

