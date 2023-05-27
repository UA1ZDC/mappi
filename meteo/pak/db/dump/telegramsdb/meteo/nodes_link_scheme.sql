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

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: nodes_link; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.nodes_link (
    parent integer,
    child integer
);


ALTER TABLE meteo.nodes_link OWNER TO postgres;

--
-- Name: nodes_link unique_nodes_link; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.nodes_link
    ADD CONSTRAINT unique_nodes_link UNIQUE (parent, child);


--
-- PostgreSQL database dump complete
--

