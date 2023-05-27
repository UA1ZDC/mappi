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
-- Name: forecast_manual; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.forecast_manual (
    id bigint NOT NULL,
    user_id text,
    dt timestamp without time zone NOT NULL,
    dt_start timestamp without time zone NOT NULL,
    dt_end timestamp without time zone NOT NULL,
    forecast_hour integer NOT NULL,
    forecast_text text,
    forecast_code text,
    region_id bigint,
    station_id bigint,
    forecast_params jsonb
);


ALTER TABLE meteo.forecast_manual OWNER TO postgres;

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
-- Name: forecast_manual_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.forecast_manual_id_seq OWNED BY meteo.forecast_manual.id;


--
-- Name: forecast_manual id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_manual ALTER COLUMN id SET DEFAULT nextval('meteo.forecast_manual_id_seq'::regclass);


--
-- Name: forecast_manual forecast_manual_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_manual
    ADD CONSTRAINT forecast_manual_pkey PRIMARY KEY (id);


--
-- Name: forecast_manual station_id; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_manual
    ADD CONSTRAINT station_id FOREIGN KEY (station_id) REFERENCES meteo.stations(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

