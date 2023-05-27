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
-- Name: nodes_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.nodes_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.nodes_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: nodes; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.nodes (
    id integer DEFAULT nextval('meteo.nodes_id_seq'::regclass) NOT NULL,
    name text,
    station text,
    host text,
    port integer,
    location public.geometry,
    alt real,
    country text,
    cccc text,
    status integer,
    quality real,
    station_type integer DEFAULT 7
);


ALTER TABLE meteo.nodes OWNER TO postgres;

--
-- Name: nodes pk_nodes; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.nodes
    ADD CONSTRAINT pk_nodes PRIMARY KEY (id);


--
-- Name: nodes unique_nodes; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.nodes
    ADD CONSTRAINT unique_nodes UNIQUE (station, port, host);


--
-- PostgreSQL database dump complete
--

