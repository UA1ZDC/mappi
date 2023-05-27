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
-- Name: ranks_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.ranks_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.ranks_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: ranks; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.ranks (
    id integer DEFAULT nextval('meteo.ranks_id_seq'::regclass) NOT NULL,
    name text NOT NULL
);


ALTER TABLE meteo.ranks OWNER TO postgres;

--
-- Name: ranks ranks_id_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.ranks
    ADD CONSTRAINT ranks_id_pkey PRIMARY KEY (id);


--
-- Name: ranks ranks_name_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.ranks
    ADD CONSTRAINT ranks_name_unique UNIQUE (name);


--
-- PostgreSQL database dump complete
--

