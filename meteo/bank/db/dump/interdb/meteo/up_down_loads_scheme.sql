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
-- Name: up_down_loads; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.up_down_loads (
    upload_id bigint NOT NULL,
    gridfs_id text,
    dt_write timestamp without time zone,
    file_id text,
    id bigint NOT NULL,
    chunk_id integer,
    next_chunk integer
);


ALTER TABLE meteo.up_down_loads OWNER TO postgres;

--
-- Name: up_down_loads__id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.up_down_loads__id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.up_down_loads__id_seq OWNER TO postgres;

--
-- Name: up_down_loads__id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.up_down_loads__id_seq OWNED BY meteo.up_down_loads.id;


--
-- Name: up_down_loads_upload_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.up_down_loads_upload_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.up_down_loads_upload_id_seq OWNER TO postgres;

--
-- Name: up_down_loads_upload_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.up_down_loads_upload_id_seq OWNED BY meteo.up_down_loads.upload_id;


--
-- Name: up_down_loads upload_id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.up_down_loads ALTER COLUMN upload_id SET DEFAULT nextval('meteo.up_down_loads_upload_id_seq'::regclass);


--
-- Name: up_down_loads id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.up_down_loads ALTER COLUMN id SET DEFAULT nextval('meteo.up_down_loads__id_seq'::regclass);


--
-- Name: up_down_loads up_down_loads_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.up_down_loads
    ADD CONSTRAINT up_down_loads_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

