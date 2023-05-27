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
-- Data for Name: satellite; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.satellite (id, name) VALUES (1, 'NOAA 19');
INSERT INTO meteo.satellite (id, name) VALUES (2, 'NOAA 18');
INSERT INTO meteo.satellite (id, name) VALUES (3, 'NOAA 15');
INSERT INTO meteo.satellite (id, name) VALUES (4, 'METEOR-M 2');
INSERT INTO meteo.satellite (id, name) VALUES (5, 'METOP-A');
INSERT INTO meteo.satellite (id, name) VALUES (6, 'FENGYUN 3A');
INSERT INTO meteo.satellite (id, name) VALUES (7, 'FENGYUN 3B');
INSERT INTO meteo.satellite (id, name) VALUES (8, 'FENGYUN 3C');
INSERT INTO meteo.satellite (id, name) VALUES (9, 'ELEKTRO-L 2');
INSERT INTO meteo.satellite (id, name) VALUES (10, 'NOAA 20');
INSERT INTO meteo.satellite (id, name) VALUES (11, 'TERRA');


--
-- Name: satellite_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.satellite_id_seq', 12, true);


--
-- PostgreSQL database dump complete
--

