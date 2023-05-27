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
-- Name: surf_accumulation; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.surf_accumulation (
    station text NOT NULL,
    station_type smallint NOT NULL,
    dt timestamp without time zone NOT NULL,
    dtend timestamp without time zone,
    descrname text NOT NULL,
    value real,
    dt_write timestamp without time zone DEFAULT now(),
    calc_day boolean DEFAULT false,
    calc_decade boolean DEFAULT false,
    calc_month boolean DEFAULT false
);


ALTER TABLE meteo.surf_accumulation OWNER TO postgres;

--
-- Name: surf_accumulation surf_accumulation_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.surf_accumulation
    ADD CONSTRAINT surf_accumulation_pkey PRIMARY KEY (station, station_type, dt, descrname);


--
-- PostgreSQL database dump complete
--

