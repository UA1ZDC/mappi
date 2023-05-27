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
-- Name: aero; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.aero (
    station text NOT NULL,
    station_type smallint NOT NULL,
    dt timestamp without time zone NOT NULL,
    level_type smallint NOT NULL,
    level real NOT NULL,
    descrname text NOT NULL,
    dt_write timestamp without time zone DEFAULT now() NOT NULL,
    value real NOT NULL
);


ALTER TABLE meteo.aero OWNER TO postgres;

--
-- Name: aero aero_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.aero
    ADD CONSTRAINT aero_pkey PRIMARY KEY (station, station_type, dt, level_type, level, descrname);


--
-- PostgreSQL database dump complete
--

