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
-- Data for Name: dopusk_opr; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (5, 12103, '{0}', '{1}', 1.5, 0, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (7, 12108, '{0}', '{1}', 1.5, 0, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (14, 20042, '{0}', '{200}', 0, 0, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (1, 20010, '{0}', '{200}', 15, 0, NULL, NULL, 0, 72);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (3, 12101, '{0}', '{1}', 1.5, 1, NULL, NULL, 0, 72);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (10, 11001, '{0}', '{1}', 10, 1, NULL, NULL, 0, 72);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (12, 11002, '{0}', '{1}', 1.5, 1, NULL, NULL, 0, 72);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (13, 11002, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 2.5, 1, NULL, NULL, 0, 72);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (9, 20001, '{0}', '{1}', NULL, 0, NULL, '[{"max": 200, "min": 0}, {"max": 400, "min": 200}, {"max": 800, "min": 400}, {"max": 1000, "min": 600}, {"max": 1500, "min": 1000}, {"max": 2000, "min": 1500}, {"max": 3000, "min": 2000}, {"max": 4000, "min": 3000}, {"max": 6000, "min": 4000}, {"max": 10000, "min": 6000}, {"max": 100000, "min": 10000}]', 0, 72);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (2, 20013, '{0}', '{200}', NULL, 1, NULL, '[{"max": 50, "min": 0}, {"max": 100, "min": 50}, {"max": 150, "min": 100}, {"max": 200, "min": 150}, {"max": 300, "min": 200}, {"max": 350, "min": 250}, {"max": 500, "min": 300}, {"max": 600, "min": 400}, {"max": 800, "min": 500}, {"max": 1000, "min": 600}, {"max": 1500, "min": 1000}, {"max": 2000, "min": 1500}, {"max": 3000, "min": 2000}, {"max": 4000, "min": 3000}, {"max": 5000, "min": 4000}, {"max": 6000, "min": 5000}, {"max": 7000, "min": 6000}, {"max": 8000, "min": 7000}, {"max": 9000, "min": 8000}, {"max": 11000, "min": 10000}]', 0, 72);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (35, 20010, '{0}', '{200}', 20, 0, NULL, NULL, 72, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (36, 20013, '{0}', '{200}', NULL, 1, NULL, '[{"max": 100, "min": 0}, {"max": 200, "min": 100}, {"max": 400, "min": 200}, {"max": 600, "min": 300}, {"max": 1000, "min": 600}, {"max": 1500, "min": 1000}, {"max": 3000, "min": 2000}, {"max": 4000, "min": 3000}, {"max": 5000, "min": 4000}, {"max": 6000, "min": 5000}, {"max": 7000, "min": 6000}, {"max": 8000, "min": 7000}, {"max": 9000, "min": 8000}, {"max": 11000, "min": 10000}]', 72, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (37, 20001, '{0}', '{1}', NULL, 0, NULL, '[{"max": 500, "min": 0}, {"max": 1500, "min": 500}, {"max": 2000, "min": 1000}, {"max": 4000, "min": 2000}, {"max": 10000, "min": 4000}, {"max": 100000, "min": 10000}]', 72, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (39, 11002, '{0}', '{1}', 2.5, 1, NULL, NULL, 72, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (40, 12101, '{0}', '{1}', 2.5, 1, NULL, NULL, 72, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (18, 10009, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 4, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (19, 10051, '{0}', '{1,102}', 4, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (20, 10051, NULL, '{6,7}', 10, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (11, 11001, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 20, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (33, 11003, NULL, '{6,7}', 5, 0, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (27, 11003, '{0}', '{1}', 10, 1, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (28, 11003, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 5, 1, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (29, 11004, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 5, 1, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (4, 12101, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 2.5, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (26, 12101, NULL, '{6,7}', 2.5, 1, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (6, 12103, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 2.5, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (34, 12108, NULL, '{6,7}', 2.5, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (8, 12108, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 2.5, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (23, 12111, '{0}', '{1}', 2.5, 0, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (24, 12112, '{0}', '{1}', 2.5, 0, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (30, 13003, '{0}', '{1}', 10, 1, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (31, 13003, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 20, 1, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (32, 13003, NULL, '{6,7}', 20, 1, 3, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (16, 13022, '{0}', '{1}', NULL, 0, NULL, '[{"max": 100, "min": 0.1}]', 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (21, 13044, '{0}', '{1}', 10, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (17, 13055, '{0}', '{1}', NULL, 0, NULL, '[{"max": 100, "min": 0.1}]', 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (15, 20042, '{1000,950,925,850,700,600,500,400,300,250,200,150,100,70,50,10,5}', '{100}', 0, 0, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (22, 22021, '{0}', '{1}', 2, 1, NULL, NULL, 0, 120);
INSERT INTO meteo.dopusk_opr (id, descriptor, level, type_level, delta, dopusk_grad, time_delta, gradacii, hour_min, hour_max) VALUES (38, 11001, '{0}', '{1}', 45, 0, 3, NULL, 72, 120);


--
-- Name: dopusk_opr_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.dopusk_opr_id_seq', 40, true);


--
-- PostgreSQL database dump complete
--

