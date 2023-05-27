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
-- Name: stations_history_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.stations_history_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.stations_history_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: stations_history; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.stations_history (
    id bigint DEFAULT nextval('meteo.stations_history_id_seq'::regclass) NOT NULL,
    station text NOT NULL,
    station_type integer,
    data_type integer NOT NULL,
    dt timestamp without time zone NOT NULL
);


ALTER TABLE meteo.stations_history OWNER TO postgres;

--
-- Name: TABLE stations_history; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.stations_history IS 'Таблица с указанием даты получения данных со станции ';


--
-- Name: stations_history station_date; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.stations_history
    ADD CONSTRAINT station_date UNIQUE (station, data_type, dt);


--
-- Name: stations_history stations_history_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.stations_history
    ADD CONSTRAINT stations_history_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

