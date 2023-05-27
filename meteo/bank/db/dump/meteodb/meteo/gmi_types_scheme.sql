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
-- Name: gmi_types; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.gmi_types (
    id integer NOT NULL,
    gmitype integer,
    description text,
    subtype jsonb
);


ALTER TABLE meteo.gmi_types OWNER TO postgres;

--
-- Name: gmi_types_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.gmi_types_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.gmi_types_id_seq OWNER TO postgres;

--
-- Name: gmi_types_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.gmi_types_id_seq OWNED BY meteo.gmi_types.id;


--
-- Name: gmi_types id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.gmi_types ALTER COLUMN id SET DEFAULT nextval('meteo.gmi_types_id_seq'::regclass);


--
-- Name: gmi_types gmi_types_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.gmi_types
    ADD CONSTRAINT gmi_types_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

