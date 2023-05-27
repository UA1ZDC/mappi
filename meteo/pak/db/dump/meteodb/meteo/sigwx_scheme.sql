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
-- Name: sigwx_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.sigwx_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.sigwx_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: sigwx; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.sigwx (
    id bigint DEFAULT nextval('meteo.sigwx_id_seq'::regclass) NOT NULL,
    dt_write timestamp without time zone,
    dt timestamp without time zone NOT NULL,
    center smallint,
    dt_beg timestamp without time zone,
    dt_end timestamp without time zone,
    level_type smallint,
    level_lo integer,
    level_hi integer,
    descriptor integer,
    value double precision,
    fileid bigint,
    tlgid bigint,
    data_type integer,
    source text,
    ii integer
);


ALTER TABLE meteo.sigwx OWNER TO postgres;

--
-- Name: sigwx sigwx_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.sigwx
    ADD CONSTRAINT sigwx_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

