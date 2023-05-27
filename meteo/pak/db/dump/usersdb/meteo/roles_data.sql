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
-- Data for Name: roles; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.roles (id, name) VALUES (2, 'Служебный пользователь');
INSERT INTO meteo.roles (id, name) VALUES (3, 'Пользователь');


--
-- PostgreSQL database dump complete
--

