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
-- Name: gribreport_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.gribreport_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.gribreport_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = true;

--
-- Name: grib; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.grib (
    id bigint DEFAULT nextval('meteo.gribreport_id_seq'::regclass) NOT NULL,
    dt timestamp without time zone NOT NULL,
    hour bigint,
    edition integer,
    center integer,
    subcenter integer,
    product integer,
    descr bigint,
    level real,
    "levelType" integer,
    "procType" integer,
    "timeRange" integer,
    sign_dt integer,
    dt1 timestamp without time zone,
    dt2 timestamp without time zone,
    status bigint,
    data_type integer,
    discipline integer,
    category integer,
    "defType" integer,
    "stepLat" integer,
    "stepLon" integer,
    fileid bigint,
    tlgid text,
    crc integer,
    analysed boolean,
    dt_write timestamp without time zone NOT NULL,
    error boolean,
    comment text
)
WITH (fillfactor='90');


ALTER TABLE meteo.grib OWNER TO postgres;

--
-- Name: TABLE grib; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.grib IS 'таблица для хранения данных grib';


--
-- Name: grib grib_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.grib
    ADD CONSTRAINT grib_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

