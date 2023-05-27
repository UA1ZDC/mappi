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
-- Name: centers; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.centers (
    id integer NOT NULL,
    center integer,
    full_name text,
    short_name text,
    priority integer,
    subcenters jsonb,
    priority_value double precision,
    forecast_count integer,
    forecast_true_count integer
);


ALTER TABLE meteo.centers OWNER TO postgres;

--
-- Name: centers_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.centers_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.centers_id_seq OWNER TO postgres;

--
-- Name: centers_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.centers_id_seq OWNED BY meteo.centers.id;


--
-- Name: centers id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.centers ALTER COLUMN id SET DEFAULT nextval('meteo.centers_id_seq'::regclass);


--
-- Name: centers centers_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.centers
    ADD CONSTRAINT centers_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

