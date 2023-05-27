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
-- Name: gmi_condition_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.gmi_condition_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.gmi_condition_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: gmi_condition; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.gmi_condition (
    id bigint DEFAULT nextval('meteo.gmi_condition_id_seq'::regclass) NOT NULL,
    name text,
    t1 text,
    t2 text
);


ALTER TABLE meteo.gmi_condition OWNER TO postgres;

--
-- Name: gmi_condition gmi_condition_t1_t2_cnst; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.gmi_condition
    ADD CONSTRAINT gmi_condition_t1_t2_cnst UNIQUE (t1, t2);


--
-- PostgreSQL database dump complete
--

