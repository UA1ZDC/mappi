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
-- Name: obanal_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.obanal_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.obanal_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = true;

--
-- Name: obanal; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.obanal (
    id bigint DEFAULT nextval('meteo.obanal_id_seq'::regclass) NOT NULL,
    dt timestamp without time zone,
    descr integer,
    hour integer,
    model integer,
    level integer,
    level_type integer,
    center integer,
    net_type integer,
    forecast_start timestamp without time zone,
    forecast_end timestamp without time zone,
    count_points integer,
    time_range integer,
    dt_write timestamp without time zone,
    fs_id bigint
)
WITH (fillfactor='90');


ALTER TABLE meteo.obanal OWNER TO postgres;

--
-- Name: obanalfs_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.obanalfs_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.obanalfs_id_seq OWNER TO postgres;

--
-- Name: obanal insert_new_obanal_data; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.obanal
    ADD CONSTRAINT insert_new_obanal_data UNIQUE (dt, descr, hour, model, level, level_type, center, net_type, forecast_start, forecast_end);


--
-- Name: obanal obanal_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.obanal
    ADD CONSTRAINT obanal_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

