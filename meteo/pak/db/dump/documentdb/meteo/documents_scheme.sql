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
-- Name: documents_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.documents_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.documents_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: documents; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.documents (
    id bigint DEFAULT nextval('meteo.documents_seq'::regclass) NOT NULL,
    center integer,
    dt timestamp without time zone,
    format text,
    hour integer,
    job_title text,
    jobid text,
    map_title text,
    mapid text,
    model integer,
    path text,
    idfile bigint,
    dt_write timestamp without time zone
);


ALTER TABLE meteo.documents OWNER TO postgres;

--
-- Name: documents documents_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.documents
    ADD CONSTRAINT documents_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

