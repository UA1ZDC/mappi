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
-- Name: conferences; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.conferences (
    id bigint NOT NULL,
    name text NOT NULL,
    owner text,
    members text[]
);


ALTER TABLE meteo.conferences OWNER TO postgres;

--
-- Name: conferences__id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.conferences__id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.conferences__id_seq OWNER TO postgres;

--
-- Name: conferences__id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.conferences__id_seq OWNED BY meteo.conferences.id;


--
-- Name: conferences id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.conferences ALTER COLUMN id SET DEFAULT nextval('meteo.conferences__id_seq'::regclass);


--
-- Name: conferences conferences_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.conferences
    ADD CONSTRAINT conferences_pkey PRIMARY KEY (name);


--
-- PostgreSQL database dump complete
--

