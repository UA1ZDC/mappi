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
-- Name: forecast_centers; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.forecast_centers (
    id bigint NOT NULL,
    name text,
    property jsonb
);


ALTER TABLE meteo.forecast_centers OWNER TO postgres;

--
-- Name: forecast_centers forecast_centers_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_centers
    ADD CONSTRAINT forecast_centers_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

