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
-- Name: forecast; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.forecast (
    id bigint NOT NULL,
    dt timestamp without time zone,
    dt_beg timestamp without time zone,
    dt_end timestamp without time zone,
    station text,
    station_type integer,
    level integer,
    level_type smallint,
    center bigint,
    hour integer,
    model smallint,
    station_info jsonb,
    descrname text,
    param jsonb,
    observ jsonb DEFAULT '[]'::jsonb,
    field jsonb DEFAULT '[]'::jsonb,
    flag_opravd boolean DEFAULT false,
    opr boolean,
    error boolean
);


ALTER TABLE meteo.forecast OWNER TO postgres;

--
-- Name: forecast_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_id_seq OWNER TO postgres;

--
-- Name: forecast_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.forecast_id_seq OWNED BY meteo.forecast.id;


--
-- Name: forecast_manual_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_manual_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_manual_id_seq OWNER TO postgres;

--
-- Name: forecast_opr__id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_opr__id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_opr__id_seq OWNER TO postgres;

--
-- Name: forecast_region_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_region_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_region_id_seq OWNER TO postgres;

--
-- Name: forecast_regiongroup_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_regiongroup_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_regiongroup_id_seq OWNER TO postgres;

--
-- Name: forecast id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast ALTER COLUMN id SET DEFAULT nextval('meteo.forecast_id_seq'::regclass);


--
-- Name: forecast forecast_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast
    ADD CONSTRAINT forecast_pkey PRIMARY KEY (id);


--
-- Name: forecast insert_forecast_uniq; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast
    ADD CONSTRAINT insert_forecast_uniq UNIQUE (station, station_type, dt_beg, dt_end, level, level_type, center, hour, model, descrname);


--
-- PostgreSQL database dump complete
--

