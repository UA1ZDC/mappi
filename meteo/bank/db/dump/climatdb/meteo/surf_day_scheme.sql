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
-- Name: surf_day; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.surf_day (
    station text NOT NULL,
    station_type smallint NOT NULL,
    descrname text NOT NULL,
    date date NOT NULL,
    value real,
    dt_write timestamp without time zone DEFAULT now()
);


ALTER TABLE meteo.surf_day OWNER TO postgres;

--
-- Name: surf_day surf_day_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.surf_day
    ADD CONSTRAINT surf_day_pkey PRIMARY KEY (station, station_type, descrname, date);


--
-- PostgreSQL database dump complete
--

