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
-- Name: forecast_regiongroup; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.forecast_regiongroup (
    id integer NOT NULL,
    title text,
    description text,
    is_active boolean
);


ALTER TABLE meteo.forecast_regiongroup OWNER TO postgres;

--
-- Name: forecast_regiongroup_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_regiongroup_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_regiongroup_id_seq OWNER TO postgres;

--
-- Name: forecast_regiongroup_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.forecast_regiongroup_id_seq OWNED BY meteo.forecast_regiongroup.id;


--
-- Name: forecast_regiongroup id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_regiongroup ALTER COLUMN id SET DEFAULT nextval('meteo.forecast_regiongroup_id_seq'::regclass);


--
-- Name: forecast_regiongroup forecast_regiongroup_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_regiongroup
    ADD CONSTRAINT forecast_regiongroup_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

