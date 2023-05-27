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
-- Name: forecast_region_to_regiongroup; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.forecast_region_to_regiongroup (
    regiongroup_id bigint,
    region_id bigint,
    station_id bigint,
    parent_id bigint
);


ALTER TABLE meteo.forecast_region_to_regiongroup OWNER TO postgres;

--
-- Name: forecast_region_to_regiongroup region_uniq_data; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_region_to_regiongroup
    ADD CONSTRAINT region_uniq_data UNIQUE (regiongroup_id, region_id, station_id, parent_id);


--
-- Name: forecast_region_to_regiongroup parent_id; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_region_to_regiongroup
    ADD CONSTRAINT parent_id FOREIGN KEY (parent_id) REFERENCES meteo.forecast_regiongroup(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: forecast_region_to_regiongroup station_id; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.forecast_region_to_regiongroup
    ADD CONSTRAINT station_id FOREIGN KEY (station_id) REFERENCES meteo.stations(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

