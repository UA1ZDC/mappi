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
-- Name: session_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.session_view AS
 SELECT session.id,
    satellite.name AS satellite,
    session.date_start,
    session.date_end,
    session.date_start_offset,
    session.revol,
    session.direction,
    session.elevat_max,
    site_view.name AS site,
    site_view.latitude,
    site_view.longitude,
    session.fpath,
    session.tle
   FROM ((meteo.session
     LEFT JOIN meteo.site_view ON ((session.site = site_view.id)))
     LEFT JOIN meteo.satellite ON ((session.satellite = satellite.id)));


ALTER TABLE meteo.session_view OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

