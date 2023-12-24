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
-- Data for Name: instrument; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.instrument (id, name) VALUES (1, 'AVHRR/3');
INSERT INTO meteo.instrument (id, name) VALUES (3, 'VIRR');
INSERT INTO meteo.instrument (id, name) VALUES (2, 'MSU-MR(h)');
INSERT INTO meteo.instrument (id, name) VALUES (4, 'MSU-MR(l)');
INSERT INTO meteo.instrument (id, name) VALUES (5, 'MTVZA-GY');
INSERT INTO meteo.instrument (id, name) VALUES (6, 'MHS');
INSERT INTO meteo.instrument (id, name) VALUES (9, 'MSU-GS');
INSERT INTO meteo.instrument (id, name) VALUES (7, 'AMSU-A1');
INSERT INTO meteo.instrument (id, name) VALUES (8, 'AMSU-A2');
INSERT INTO meteo.instrument (id, name) VALUES (10, 'VIIRS');
INSERT INTO meteo.instrument (id, name) VALUES (11, 'MODIS');
INSERT INTO meteo.instrument (id, name) VALUES (12, 'MERSI-1');
INSERT INTO meteo.instrument (id, name) VALUES (13, 'MERSI-2');
INSERT INTO meteo.instrument (id, name) VALUES (14, 'MERSI-3');
INSERT INTO meteo.instrument (id, name) VALUES (15, 'MERSI-LL');
INSERT INTO meteo.instrument (id, name) VALUES (16, 'MERSI-RM');


--
-- Name: instrument_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.instrument_id_seq', 12, true);


--
-- PostgreSQL database dump complete
--

