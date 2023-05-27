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
-- Name: report_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.report_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.report_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = true;

--
-- Name: report; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.report (
    id bigint DEFAULT nextval('meteo.report_id_seq'::regclass) NOT NULL,
    dt timestamp without time zone NOT NULL,
    station text NOT NULL,
    station_type smallint,
    data_type smallint NOT NULL,
    level real NOT NULL,
    level_type smallint NOT NULL,
    param jsonb,
    location public.geometry(Point,4326),
    dt_write timestamp without time zone DEFAULT now(),
    dt_update timestamp without time zone
)
WITH (fillfactor='90');


ALTER TABLE meteo.report OWNER TO postgres;

--
-- Name: TABLE report; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.report IS 'таблица для хранения раскодированных данных со станций';


--
-- Name: report insert_new_data1; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.report
    ADD CONSTRAINT insert_new_data1 UNIQUE (dt, station, station_type, data_type, level, level_type);


--
-- Name: report report_pkey1; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.report
    ADD CONSTRAINT report_pkey1 PRIMARY KEY (id);


--
-- Name: meteo_report_gin_idx1; Type: INDEX; Schema: meteo; Owner: postgres
--

CREATE INDEX meteo_report_gin_idx1 ON meteo.report USING gin (param jsonb_path_ops);


--
-- PostgreSQL database dump complete
--

