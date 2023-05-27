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
-- Name: site_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.site_view AS
 SELECT site.id,
    site.name,
    public.st_y(site.coord) AS latitude,
    public.st_x(site.coord) AS longitude
   FROM meteo.site;


ALTER TABLE meteo.site_view OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

