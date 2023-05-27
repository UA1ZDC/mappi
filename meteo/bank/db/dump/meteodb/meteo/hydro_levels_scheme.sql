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
-- Name: hydro_levels; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.hydro_levels (
    id integer NOT NULL,
    station text,
    floodplain integer,
    flooding integer,
    danger_level integer,
    max_level integer,
    mid_level integer,
    min_level integer
);


ALTER TABLE meteo.hydro_levels OWNER TO postgres;

--
-- Name: hydro_levels_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.hydro_levels_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.hydro_levels_id_seq OWNER TO postgres;

--
-- Name: hydro_levels_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.hydro_levels_id_seq OWNED BY meteo.hydro_levels.id;


--
-- Name: hydro_levels id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.hydro_levels ALTER COLUMN id SET DEFAULT nextval('meteo.hydro_levels_id_seq'::regclass);


--
-- Name: hydro_levels hydro_levels_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.hydro_levels
    ADD CONSTRAINT hydro_levels_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

