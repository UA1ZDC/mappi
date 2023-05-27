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
-- Name: data_type; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.data_type (
    id integer NOT NULL,
    datatype integer,
    description text
);


ALTER TABLE meteo.data_type OWNER TO postgres;

--
-- Name: data_type_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.data_type_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.data_type_id_seq OWNER TO postgres;

--
-- Name: data_type_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.data_type_id_seq OWNED BY meteo.data_type.id;


--
-- Name: data_type id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.data_type ALTER COLUMN id SET DEFAULT nextval('meteo.data_type_id_seq'::regclass);


--
-- Name: data_type data_type_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.data_type
    ADD CONSTRAINT data_type_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

