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
-- Name: forecast_region_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_region_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_region_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: forecast_region; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.forecast_region (
    id integer DEFAULT nextval('meteo.forecast_region_id_seq'::regclass) NOT NULL,
    title text,
    descr text,
    fi_start real,
    fi_end real,
    la_start real,
    la_end real,
    is_active boolean,
    region_coords public.geometry,
    region_center_point public.geometry,
    okrug_id integer,
    is_okrug boolean DEFAULT false,
    CONSTRAINT enforce_dims_region_center_point CHECK ((public.st_ndims(region_center_point) = 2)),
    CONSTRAINT enforce_dims_region_coords CHECK ((public.st_ndims(region_coords) = 2)),
    CONSTRAINT enforce_geotype_region_center_point CHECK (((public.geometrytype(region_center_point) = 'POINT'::text) OR (region_center_point IS NULL))),
    CONSTRAINT enforce_srid_region_center_point CHECK ((public.st_srid(region_center_point) = 4326)),
    CONSTRAINT enforce_srid_region_coords CHECK ((public.st_srid(region_coords) = 4326))
);


ALTER TABLE meteo.forecast_region OWNER TO postgres;

--
-- Name: forecast_regiongroup_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.forecast_regiongroup_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.forecast_regiongroup_id_seq OWNER TO postgres;

--
-- Name: forecast_region forecast_region_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_region
    ADD CONSTRAINT forecast_region_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

