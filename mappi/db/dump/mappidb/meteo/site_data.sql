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
-- Data for Name: site; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.site (id, name, coord) VALUES (0, 'Санкт-Петербург', '010100000052B81E85EB513E40D7A3703D0AF74D40');


--
-- Name: site_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.site_id_seq', 1, true);


--
-- PostgreSQL database dump complete
--

