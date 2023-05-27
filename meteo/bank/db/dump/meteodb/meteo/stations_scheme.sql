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
-- Name: stations_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.stations_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.stations_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: stations; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.stations (
    id integer DEFAULT nextval('meteo.stations_id_seq'::regclass) NOT NULL,
    station text NOT NULL,
    station_type smallint NOT NULL,
    index jsonb NOT NULL,
    name jsonb,
    source text,
    alt integer,
    last_update timestamp without time zone,
    last_used timestamp without time zone,
    "check" timestamp without time zone,
    operational boolean,
    water_depth real,
    country integer,
    hydro_levels jsonb,
    airstrip_direction double precision,
    location public.geometry(Point,4326)
);


ALTER TABLE meteo.stations OWNER TO postgres;

--
-- Name: stations_history_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.stations_history_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.stations_history_id_seq OWNER TO postgres;

--
-- Name: stations id_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.stations
    ADD CONSTRAINT id_unique UNIQUE (id);


--
-- Name: stations stations_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.stations
    ADD CONSTRAINT stations_pkey PRIMARY KEY (station_type, index);


--
-- Name: location_indx; Type: INDEX; Schema: meteo; Owner: postgres
--

CREATE INDEX location_indx ON meteo.stations USING gist (location);


--
-- Name: type_operational_indx; Type: INDEX; Schema: meteo; Owner: postgres
--

CREATE INDEX type_operational_indx ON meteo.stations USING btree (station_type, operational);


--
-- Name: stations country_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.stations
    ADD CONSTRAINT country_fkey FOREIGN KEY (country) REFERENCES meteo.countries(world_number) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: stations station_type_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.stations
    ADD CONSTRAINT station_type_fkey FOREIGN KEY (station_type) REFERENCES meteo.station_types(type) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- PostgreSQL database dump complete
--

