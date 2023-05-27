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
-- Name: surf_decade; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.surf_decade (
    station text NOT NULL,
    station_type smallint NOT NULL,
    year smallint NOT NULL,
    month smallint NOT NULL,
    decade smallint NOT NULL,
    descrname text NOT NULL,
    value real,
    dt_write timestamp without time zone DEFAULT now()
);


ALTER TABLE meteo.surf_decade OWNER TO postgres;

--
-- Name: surf_decade surf_decade_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.surf_decade
    ADD CONSTRAINT surf_decade_pkey PRIMARY KEY (station, station_type, year, month, decade, descrname);


--
-- PostgreSQL database dump complete
--

