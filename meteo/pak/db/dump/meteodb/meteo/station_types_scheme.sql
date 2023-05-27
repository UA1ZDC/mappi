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
-- Name: station_types; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.station_types (
    id integer NOT NULL,
    type smallint NOT NULL,
    description text NOT NULL,
    min_data_type integer NOT NULL,
    max_data_type integer NOT NULL,
    comment text
);


ALTER TABLE meteo.station_types OWNER TO postgres;

--
-- Name: TABLE station_types; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.station_types IS 'Типы станций';


--
-- Name: station_types_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.station_types_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.station_types_id_seq OWNER TO postgres;

--
-- Name: station_types_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.station_types_id_seq OWNED BY meteo.station_types.id;


--
-- Name: station_types id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.station_types ALTER COLUMN id SET DEFAULT nextval('meteo.station_types_id_seq'::regclass);


--
-- Name: station_types station_type_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.station_types
    ADD CONSTRAINT station_type_pkey PRIMARY KEY (id);


--
-- Name: station_types unique_type; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.station_types
    ADD CONSTRAINT unique_type UNIQUE (type);


--
-- PostgreSQL database dump complete
--

