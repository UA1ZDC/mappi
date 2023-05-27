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
-- Name: typec_area_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.typec_area_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.typec_area_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: typec_area; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.typec_area (
    id bigint DEFAULT nextval('meteo.typec_area_id_seq'::regclass) NOT NULL,
    name_smb text NOT NULL,
    name_lat text,
    name_cyr text
);


ALTER TABLE meteo.typec_area OWNER TO postgres;

--
-- Name: typec_area typec_area_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.typec_area
    ADD CONSTRAINT typec_area_pkey PRIMARY KEY (name_smb);


--
-- PostgreSQL database dump complete
--

