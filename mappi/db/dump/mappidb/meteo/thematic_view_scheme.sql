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
-- Name: thematic_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.thematic_view AS
 SELECT session_view.id AS session_id,
    session_view.satellite,
    session_view.site,
    session_view.latitude,
    session_view.longitude,
    thematic.type,
    thematic.them_name AS name,
    thematic.instrument,
    instrument.name AS instr_name,
    thematic.date_start,
    thematic.date_end,
    thematic.projection,
    thematic.format,
    thematic.fpath
   FROM ((meteo.thematic
     LEFT JOIN meteo.session_view ON ((thematic.session = session_view.id)))
     LEFT JOIN meteo.instrument ON ((thematic.instrument = instrument.id)));


ALTER TABLE meteo.thematic_view OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

