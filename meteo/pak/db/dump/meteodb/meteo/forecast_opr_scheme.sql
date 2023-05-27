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
-- Name: forecast_opr; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.forecast_opr (
    id bigint NOT NULL,
    punkt_id text,
    type_level integer,
    level integer,
    descrname text,
    center integer,
    hour integer,
    ftype_method text,
    opr integer,
    not_opr integer,
    total integer
);


ALTER TABLE meteo.forecast_opr OWNER TO postgres;

--
-- Name: forecast_opr__id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_opr__id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_opr__id_seq OWNER TO postgres;

--
-- Name: forecast_opr__id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.forecast_opr__id_seq OWNED BY meteo.forecast_opr.id;


--
-- Name: forecast_opr id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_opr ALTER COLUMN id SET DEFAULT nextval('meteo.forecast_opr__id_seq'::regclass);


--
-- Name: forecast_opr forecast_opr_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_opr
    ADD CONSTRAINT forecast_opr_pkey PRIMARY KEY (id);


--
-- Name: forecast_opr forecast_opr_uniq; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_opr
    ADD CONSTRAINT forecast_opr_uniq UNIQUE (punkt_id, type_level, level, descrname, center, hour, ftype_method);


--
-- PostgreSQL database dump complete
--

