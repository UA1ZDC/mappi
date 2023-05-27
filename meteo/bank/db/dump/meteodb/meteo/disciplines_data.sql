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
-- Data for Name: disciplines; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.disciplines (id, discipline, description) VALUES (16, 0, 'Метеорологическая продукция');
INSERT INTO meteo.disciplines (id, discipline, description) VALUES (17, 1, 'Гидрологическая продукция');
INSERT INTO meteo.disciplines (id, discipline, description) VALUES (18, 2, 'Продукция, связанная с поверхностью суши');
INSERT INTO meteo.disciplines (id, discipline, description) VALUES (19, 3, 'Космическая продукция');
INSERT INTO meteo.disciplines (id, discipline, description) VALUES (20, 10, 'Океанографическая продукция');


--
-- Name: disciplines_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.disciplines_id_seq', 20, true);


--
-- PostgreSQL database dump complete
--

