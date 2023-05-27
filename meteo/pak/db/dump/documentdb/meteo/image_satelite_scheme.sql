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
-- Name: image_satelite_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.image_satelite_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.image_satelite_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: image_satelite; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.image_satelite (
    id bigint DEFAULT nextval('meteo.image_satelite_seq'::regclass) NOT NULL,
    t1 text,
    t2 text,
    a1 text,
    a2 text,
    ii integer,
    cccc text,
    yygggg text,
    dt timestamp without time zone,
    dt_write timestamp without time zone,
    fileid bigint
);


ALTER TABLE meteo.image_satelite OWNER TO postgres;

--
-- Name: image_satelite image_satelite_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.image_satelite
    ADD CONSTRAINT image_satelite_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

