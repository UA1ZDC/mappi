--
-- PostgreSQL database dump
--

-- Dumped from database version 13.6 (Debian 13.6-1.pgdg110+1)
-- Dumped by pg_dump version 14.2 (Debian 14.2-1.pgdg110+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: thematic_type; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_type (
    id integer NOT NULL,
    name character varying NOT NULL,
    title character varying
);


ALTER TABLE meteo.thematic_type OWNER TO postgres;

--
-- Name: thematic_type_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.thematic_type_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.thematic_type_id_seq OWNER TO postgres;

--
-- Name: thematic_type_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.thematic_type_id_seq OWNED BY meteo.thematic_type.id;


--
-- Name: thematic_type id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_type ALTER COLUMN id SET DEFAULT nextval('meteo.thematic_type_id_seq'::regclass);


--
-- Name: thematic_type thematic_type_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_type
    ADD CONSTRAINT thematic_type_pk PRIMARY KEY (id);


--
-- Name: thematic_type thematic_type_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_type
    ADD CONSTRAINT thematic_type_unique UNIQUE (name);


--
-- PostgreSQL database dump complete
--

