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
-- Name: parameters; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.parameters (
    id integer NOT NULL,
    grib1 integer,
    grib2 integer,
    grb2discipline integer,
    grb2category integer,
    fullname text,
    unit text,
    bufr_descr integer,
    unit_en text
);


ALTER TABLE meteo.parameters OWNER TO postgres;

--
-- Name: parameters_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.parameters_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.parameters_id_seq OWNER TO postgres;

--
-- Name: parameters_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.parameters_id_seq OWNED BY meteo.parameters.id;


--
-- Name: parameters id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.parameters ALTER COLUMN id SET DEFAULT nextval('meteo.parameters_id_seq'::regclass);


--
-- Name: parameters grib1_uniq; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.parameters
    ADD CONSTRAINT grib1_uniq UNIQUE (grib1);


--
-- Name: parameters grib2_uniq; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.parameters
    ADD CONSTRAINT grib2_uniq UNIQUE (grb2discipline, grb2category, grib2);


--
-- Name: parameters parameters_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.parameters
    ADD CONSTRAINT parameters_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

