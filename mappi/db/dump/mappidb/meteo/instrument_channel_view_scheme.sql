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
-- Name: instrument_channel_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.instrument_channel_view AS
 SELECT instrument_channel.id,
    instrument.id AS instrument_id,
    instrument.name AS instrument_name,
    channel.number AS channel_number,
    channel.alias AS channel_alias
   FROM ((meteo.instrument_channel
     LEFT JOIN meteo.instrument ON ((instrument_channel.instrument = instrument.id)))
     LEFT JOIN meteo.channel ON ((instrument_channel.channel = channel.id)));


ALTER TABLE meteo.instrument_channel_view OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

