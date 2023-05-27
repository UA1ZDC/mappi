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

--
-- Name: pretreatment_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.pretreatment_view AS
 SELECT session_view.id AS session_id,
    session_view.satellite,
    session_view.site,
    session_view.latitude,
    session_view.longitude,
    instrument_channel_view.instrument_id,
    instrument_channel_view.instrument_name,
    instrument_channel_view.channel_number,
    instrument_channel_view.channel_alias,
    pretreatment.date_start,
    pretreatment.date_end,
    pretreatment.calibration,
    pretreatment.fpath
   FROM ((meteo.pretreatment
     LEFT JOIN meteo.instrument_channel_view ON ((pretreatment.instrument_channel = instrument_channel_view.id)))
     LEFT JOIN meteo.session_view ON ((pretreatment.session = session_view.id)));


ALTER TABLE meteo.pretreatment_view OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

