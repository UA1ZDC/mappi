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
-- Name: punkts; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.punkts (
    name text,
    fi real,
    la real,
    height real,
    station_id text,
    is_active boolean DEFAULT true,
    station_type integer,
    _id bigint NOT NULL
);
ALTER TABLE ONLY meteo.punkts ALTER COLUMN height SET STATISTICS 0;


ALTER TABLE meteo.punkts OWNER TO postgres;

--
-- Name: COLUMN punkts.name; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.punkts.name IS 'название пункта';


--
-- Name: COLUMN punkts.fi; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.punkts.fi IS 'широта пункта';


--
-- Name: COLUMN punkts.la; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.punkts.la IS 'долгота пункта';


--
-- Name: COLUMN punkts.height; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.punkts.height IS 'высота пункта над уровнем моря';


--
-- Name: COLUMN punkts.station_id; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.punkts.station_id IS 'номер в базе stations';


--
-- Name: punkts__id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.punkts__id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.punkts__id_seq OWNER TO postgres;

--
-- Name: punkts__id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.punkts__id_seq OWNED BY meteo.punkts._id;


--
-- Name: punkts _id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.punkts ALTER COLUMN _id SET DEFAULT nextval('meteo.punkts__id_seq'::regclass);


--
-- Name: punkts insert_punkt; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.punkts
    ADD CONSTRAINT insert_punkt UNIQUE (name, fi, la, height, station_id, station_type);


--
-- Name: punkts punkts_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.punkts
    ADD CONSTRAINT punkts_pkey PRIMARY KEY (_id);


--
-- PostgreSQL database dump complete
--

