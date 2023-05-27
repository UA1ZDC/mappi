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
-- Name: dopusk_opr; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.dopusk_opr (
    id bigint NOT NULL,
    descriptor integer DEFAULT 0,
    level integer[],
    type_level integer[] DEFAULT '{1}'::integer[],
    delta real,
    dopusk_grad integer DEFAULT 0,
    time_delta integer DEFAULT 3,
    gradacii jsonb,
    hour_min integer,
    hour_max integer
);


ALTER TABLE meteo.dopusk_opr OWNER TO postgres;

--
-- Name: dopusk_opr_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.dopusk_opr_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.dopusk_opr_id_seq OWNER TO postgres;

--
-- Name: dopusk_opr_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.dopusk_opr_id_seq OWNED BY meteo.dopusk_opr.id;


--
-- Name: dopusk_opr id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.dopusk_opr ALTER COLUMN id SET DEFAULT nextval('meteo.dopusk_opr_id_seq'::regclass);


--
-- Name: dopusk_opr dopusk_opr_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.dopusk_opr
    ADD CONSTRAINT dopusk_opr_pkey PRIMARY KEY (id);


--
-- Name: dopusk_opr dopusk_opr_uniq; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.dopusk_opr
    ADD CONSTRAINT dopusk_opr_uniq UNIQUE (descriptor, level, type_level, hour_min, hour_max);


--
-- PostgreSQL database dump complete
--

