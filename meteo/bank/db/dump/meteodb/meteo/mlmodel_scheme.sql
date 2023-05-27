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
-- Name: mlmodel_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.mlmodel_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.mlmodel_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = true;

--
-- Name: mlmodel; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.mlmodel (
    id bigint DEFAULT nextval('meteo.mlmodel_id_seq'::regclass) NOT NULL,
    descr integer,
    hour integer,
    model integer,
    level integer,
    level_type integer,
    center integer,
    net_type integer,
    fs_id bigint,
    count_points integer,
    mlmodel_type text,
    dt_write timestamp without time zone,
    location public.geometry,
    CONSTRAINT enforce_dims_location CHECK ((public.st_ndims(location) = 2)),
    CONSTRAINT enforce_geotype_location CHECK (((public.geometrytype(location) = 'POINT'::text) OR (location IS NULL))),
    CONSTRAINT enforce_srid_location CHECK ((public.st_srid(location) = 4326))
)
WITH (fillfactor='90');


ALTER TABLE meteo.mlmodel OWNER TO postgres;

--
-- Name: mlmodelfs_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.mlmodelfs_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.mlmodelfs_id_seq OWNER TO postgres;

--
-- Name: mlmodel mlmodel_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.mlmodel
    ADD CONSTRAINT mlmodel_pkey PRIMARY KEY (id);


--
-- Name: mlmodel mlmodel_uniq_data; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.mlmodel
    ADD CONSTRAINT mlmodel_uniq_data UNIQUE (descr, hour, model, level, level_type, center, net_type, mlmodel_type, location);


--
-- PostgreSQL database dump complete
--

