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
-- Name: pretreatment; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment (
    session bigint,
    instrument_channel integer NOT NULL,
    date_start timestamp without time zone NOT NULL,
    date_end timestamp without time zone NOT NULL,
    calibration boolean,
    fpath character varying NOT NULL
);


ALTER TABLE meteo.pretreatment OWNER TO postgres;

--
-- Name: pretreatment pretreatment_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment
    ADD CONSTRAINT pretreatment_unique UNIQUE (session, instrument_channel);


--
-- Name: pretreatment partitioning; Type: TRIGGER; Schema: meteo; Owner: postgres
--

CREATE TRIGGER partitioning BEFORE INSERT ON meteo.pretreatment FOR EACH ROW EXECUTE FUNCTION meteo.pretreatment_partitioning_range_day();


--
-- Name: pretreatment $1; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment
    ADD CONSTRAINT "$1" FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE CASCADE NOT VALID;


--
-- Name: pretreatment $2; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment
    ADD CONSTRAINT "$2" FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- PostgreSQL database dump complete
--

