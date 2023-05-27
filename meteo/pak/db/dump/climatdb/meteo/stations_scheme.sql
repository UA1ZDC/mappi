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
-- Name: stations; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.stations (
    station text NOT NULL,
    type smallint NOT NULL,
    cltype smallint NOT NULL,
    dtbegin timestamp without time zone NOT NULL,
    dtend timestamp without time zone NOT NULL,
    utc real NOT NULL,
    location public.geometry,
    name_ru text,
    name_en text,
    country smallint
);


ALTER TABLE meteo.stations OWNER TO postgres;

--
-- Name: stations stations_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.stations
    ADD CONSTRAINT stations_pkey PRIMARY KEY (station, type, cltype);


--
-- PostgreSQL database dump complete
--

