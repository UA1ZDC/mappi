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
-- Name: disciplines; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.disciplines (
    id integer NOT NULL,
    discipline integer,
    description text
);


ALTER TABLE meteo.disciplines OWNER TO postgres;

--
-- Name: disciplines_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.disciplines_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.disciplines_id_seq OWNER TO postgres;

--
-- Name: disciplines_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.disciplines_id_seq OWNED BY meteo.disciplines.id;


--
-- Name: disciplines id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.disciplines ALTER COLUMN id SET DEFAULT nextval('meteo.disciplines_id_seq'::regclass);


--
-- Name: disciplines disciplines_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.disciplines
    ADD CONSTRAINT disciplines_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

