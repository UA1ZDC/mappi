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
-- Data for Name: users; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.users (id, username, role, rank, gender, department, permissions, birthdate, block, avatar) VALUES (1, 'vgmdaemon', 2, 2, 2, NULL, '{"contact_visibility": 0}', NULL, false, NULL);
INSERT INTO meteo.users (id, username, role, rank, gender, department, permissions, birthdate, block, avatar) VALUES (2, 'gotur', 3, 23, 2, NULL, '{"contact_visibility": 0}', NULL, false, NULL);


--
-- PostgreSQL database dump complete
--

