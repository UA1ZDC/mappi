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

--
-- Name: countries_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.countries_id_seq
    START WITH 0
    INCREMENT BY 1
    MINVALUE 0
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.countries_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: countries; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.countries (
    id bigint DEFAULT nextval('meteo.countries_id_seq'::regclass) NOT NULL,
    code text,
    name_common text,
    name_ru text,
    name_en text,
    wmocode text,
    wmonumber integer,
    world_number integer NOT NULL,
    priority integer,
    icao text[]
);


ALTER TABLE meteo.countries OWNER TO postgres;

--
-- Name: TABLE countries; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.countries IS 'Страны';


--
-- Name: countries country_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.countries
    ADD CONSTRAINT country_pkey PRIMARY KEY (id);


--
-- Name: countries unique_number; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.countries
    ADD CONSTRAINT unique_number UNIQUE (world_number);


--
-- PostgreSQL database dump complete
--

