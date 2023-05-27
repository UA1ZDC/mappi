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
-- Name: meteo; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA meteo;


ALTER SCHEMA meteo OWNER TO postgres;

--
-- Name: add_meteodata_result; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.add_meteodata_result AS (
	type_result text,
	count integer
);


ALTER TYPE meteo.add_meteodata_result OWNER TO postgres;

--
-- Name: addmeteodatares; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.addmeteodatares AS (
	id bigint,
	inserted boolean,
	param jsonb
);


ALTER TYPE meteo.addmeteodatares OWNER TO postgres;

--
-- Name: aero_date_list; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.aero_date_list AS (
	dt timestamp without time zone
);


ALTER TYPE meteo.aero_date_list OWNER TO postgres;

--
-- Name: available_ocean; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.available_ocean AS (
	dt timestamp without time zone,
	station text,
	location jsonb,
	maxlvl real,
	count bigint
);


ALTER TYPE meteo.available_ocean OWNER TO postgres;

--
-- Name: available_stations; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.available_stations AS (
	station_type smallint,
	stations text[]
);


ALTER TYPE meteo.available_stations OWNER TO postgres;

--
-- Name: available_synmobsea; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.available_synmobsea AS (
	dt timestamp without time zone,
	station text,
	coord jsonb,
	count bigint
);


ALTER TYPE meteo.available_synmobsea OWNER TO postgres;

--
-- Name: available_zond; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.available_zond AS (
	station_type smallint,
	station text,
	location jsonb,
	station_info jsonb,
	count bigint
);


ALTER TYPE meteo.available_zond OWNER TO postgres;

--
-- Name: center_descr; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.center_descr AS (
	center integer,
	name text,
	short_name text,
	priority integer
);


ALTER TYPE meteo.center_descr OWNER TO postgres;

--
-- Name: data_for_meteosummary; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.data_for_meteosummary AS (
	forecast_start timestamp without time zone,
	center integer,
	hour integer,
	model integer,
	ids jsonb,
	descr jsonb
);


ALTER TYPE meteo.data_for_meteosummary OWNER TO postgres;

--
-- Name: field_data_descr; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.field_data_descr AS (
	_id bigint,
	dt timestamp without time zone,
	descr integer,
	hour integer,
	model integer,
	level integer,
	level_type integer,
	center integer,
	net_type integer,
	forecast_start timestamp without time zone,
	forecast_end timestamp without time zone,
	fs_id bigint,
	count_points integer,
	"centers.name" text,
	"centers.short_name" text,
	"level_types.full_name" text,
	priority integer
);


ALTER TYPE meteo.field_data_descr OWNER TO postgres;

--
-- Name: field_for_zond; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.field_for_zond AS (
	forecast_start timestamp without time zone,
	zonds jsonb
);


ALTER TYPE meteo.field_for_zond OWNER TO postgres;

--
-- Name: grib_data_descr; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.grib_data_descr AS (
	dt timestamp without time zone,
	descr jsonb,
	hour integer,
	model integer,
	level integer,
	level_type integer,
	center integer,
	step_lat integer,
	step_lon integer,
	dt1 timestamp without time zone,
	dt2 timestamp without time zone,
	time_range integer,
	"centers.name" text,
	"centers.short_name" text,
	"level_types.full_name" text
);


ALTER TYPE meteo.grib_data_descr OWNER TO postgres;

--
-- Name: id_param; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.id_param AS (
	id bigint,
	param jsonb
);


ALTER TYPE meteo.id_param OWNER TO postgres;

--
-- Name: many_ocean_data; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.many_ocean_data AS (
	coords jsonb,
	station text,
	param jsonb,
	id bigint
);


ALTER TYPE meteo.many_ocean_data OWNER TO postgres;

--
-- Name: meteo_data; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.meteo_data AS (
	id_ bigint,
	dt timestamp without time zone,
	station text,
	stat_type smallint,
	data_type smallint,
	level real,
	level_type smallint,
	param jsonb,
	location jsonb,
	dist double precision
);


ALTER TYPE meteo.meteo_data OWNER TO postgres;

--
-- Name: meteodata_count; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.meteodata_count AS (
	dt timestamp without time zone,
	count bigint,
	cover numeric,
	level real
);


ALTER TYPE meteo.meteodata_count OWNER TO postgres;

--
-- Name: meteodata_max_min_count; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.meteodata_max_min_count AS (
	dt date,
	count bigint,
	cover numeric
);


ALTER TYPE meteo.meteodata_max_min_count OWNER TO postgres;

--
-- Name: meteovalue_type; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.meteovalue_type AS (
	descrname text,
	value double precision,
	cor_number integer
);


ALTER TYPE meteo.meteovalue_type OWNER TO postgres;

--
-- Name: ocean_by_station_data; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.ocean_by_station_data AS (
	station text,
	coord jsonb,
	level real,
	documents jsonb
);


ALTER TYPE meteo.ocean_by_station_data OWNER TO postgres;

--
-- Name: one_zond_data; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.one_zond_data AS (
	level real,
	level_type smallint,
	location jsonb,
	param jsonb
);


ALTER TYPE meteo.one_zond_data OWNER TO postgres;

--
-- Name: update_data_quality; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.update_data_quality AS (
	id bigint,
	dt timestamp without time zone,
	descrname text,
	quality integer,
	control_type integer
);


ALTER TYPE meteo.update_data_quality OWNER TO postgres;

--
-- Name: upsert_report_result; Type: TYPE; Schema: meteo; Owner: postgres
--

CREATE TYPE meteo.upsert_report_result AS (
	id bigint,
	inserted boolean,
	param jsonb
);


ALTER TYPE meteo.upsert_report_result OWNER TO postgres;

--
-- Name: actualize_stations_state(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.actualize_stations_state() RETURNS integer
    LANGUAGE sql
    AS $$
WITH h AS (
    SELECT distinct(station, station_type) as d, max(dt) as max_dt FROM meteo.stations_history
	group by station, station_type, data_type  
  )
update meteo.stations s 
set operational = true, last_used = h.max_dt from h  
where (s.station,s.station_type::integer)::record = h.d;

select * from global.drop_table_partition_not_empty('stations_history');

$$;


ALTER FUNCTION meteo.actualize_stations_state() OWNER TO postgres;

--
-- Name: add_bufr_code(bigint, text, jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_bufr_code(bufr_code_ bigint, description_ text, code_values_ jsonb) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.bufr_code ( bufr_code, description, code_values ) 
	                     VALUES (  bufr_code_, description_, code_values_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_bufr_code(bufr_code_ bigint, description_ text, code_values_ jsonb) OWNER TO postgres;

--
-- Name: add_center(integer, text, text, integer, jsonb, double precision, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_center(center_ integer, full_name_ text, short_name_ text, priority_ integer, subcenters_ jsonb, priority_value_ double precision, forecast_count_ integer, forecast_true_count_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.centers ( center, full_name, short_name, priority, subcenters, priority_value, forecast_count, forecast_true_count ) 
	                     VALUES (  center_, full_name_, short_name_, priority_, subcenters_, priority_value_, forecast_count_, forecast_true_count_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_center(center_ integer, full_name_ text, short_name_ text, priority_ integer, subcenters_ jsonb, priority_value_ double precision, forecast_count_ integer, forecast_true_count_ integer) OWNER TO postgres;

--
-- Name: add_country(text, text, text, text, text[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_country(code_ text, name_common_ text, name_ru_ text, name_en_ text, icao_ text[], wmocode_ text, wmonumber_ integer, world_number_ integer, priority_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.countries ( code, name_common, name_ru, name_en, icao, wmocode, wmonumber, world_number, priority ) 
	                     VALUES (  code_, name_common_, name_ru_, name_en_, icao_, wmocode_, wmonumber_, world_number_, priority_  );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_country(code_ text, name_common_ text, name_ru_ text, name_en_ text, icao_ text[], wmocode_ text, wmonumber_ integer, world_number_ integer, priority_ integer) OWNER TO postgres;

--
-- Name: add_country(text, text, text, text, text, text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_country(code_ text, name_common_ text, name_ru_ text, name_en_ text, icao_ text, wmocode_ text, wmonumber_ integer, world_number_ integer, priority_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.countries ( code, name_common, name_ru, name_en, icao, wmocode, wmonumber, world_number, priority ) 
	                     VALUES (  code_, name_common_, name_ru_, name_en_, icao_, wmocode_, wmonumber_, world_number_, priority_  );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_country(code_ text, name_common_ text, name_ru_ text, name_en_ text, icao_ text, wmocode_ text, wmonumber_ integer, world_number_ integer, priority_ integer) OWNER TO postgres;

--
-- Name: add_data_type(integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_data_type(datatype_ integer, description_ text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.data_type ( datatype, description ) 
	                     VALUES (  datatype_, description_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_data_type(datatype_ integer, description_ text) OWNER TO postgres;

--
-- Name: add_discipline(integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_discipline(discipline_ integer, description_ text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.disciplines ( discipline, description ) 
	                     VALUES (  discipline_, description_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_discipline(discipline_ integer, description_ text) OWNER TO postgres;

--
-- Name: add_gmi_type(integer, text, jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_gmi_type(gmitype_ integer, description_ text, subtype_ jsonb) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.gmi_types ( gmitype, description, subtype ) 
	                     VALUES (  gmitype_, description_, subtype_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_gmi_type(gmitype_ integer, description_ text, subtype_ jsonb) OWNER TO postgres;

--
-- Name: add_hydro_level(text, integer, integer, integer, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_hydro_level(station_ text, floodplain_ integer, flooding_ integer, danger_level_ integer, max_level_ integer, mid_level_ integer, min_level_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.hydro_levels ( station, floodplain, flooding, danger_level, max_level, mid_level, min_level ) 
	                     VALUES (  station_, floodplain_, flooding_, danger_level_, max_level_, mid_level_, min_level_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_hydro_level(station_ text, floodplain_ integer, flooding_ integer, danger_level_ integer, max_level_ integer, mid_level_ integer, min_level_ integer) OWNER TO postgres;

--
-- Name: add_level_type(integer, text, text, double precision[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_level_type(type_ integer, description_ text, full_descr_ text, standard_value_ double precision[] DEFAULT NULL::double precision[]) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.level_types ( type, description, full_descr, standard_value ) 
	                     VALUES (  type_, description_, full_descr_, standard_value_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_level_type(type_ integer, description_ text, full_descr_ text, standard_value_ double precision[]) OWNER TO postgres;

--
-- Name: add_meteodata(timestamp without time zone, smallint, real, smallint, text, smallint, public.geometry, jsonb, boolean, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_meteodata(dt_ timestamp without time zone, data_type_ smallint, level_ real, lev_type_ smallint, station_ text, stat_type_ smallint, coord_ public.geometry, param_ jsonb, need_replace_ boolean DEFAULT false, dt_write_ timestamp without time zone DEFAULT now()) RETURNS meteo.add_meteodata_result
    LANGUAGE plpgsql
    AS $$DECLARE
 old_param_ meteo.upsert_report_result;
 result_param_ meteo.meteovalue_type;
 count_value_ integer;
 cur_value_ jsonb;
 descr_ jsonb;
 param_to_update_ jsonb;
 i_ integer;
 compare_res_ record;
 cur_descr_ text;
 v_cnt integer;
 res_ boolean;
 has_new_ boolean;
 table_name_ text;
 query_ text;
 rrr timestamp without time zone;
 result_ meteo.add_meteodata_result;
  
BEGIN
   result_.type_result := '';
   result_.count := 0;
   
 --rrr :=  clock_timestamp();

 select * from meteo.
 upsert_report( dt_, data_type_, level_, lev_type_, station_, stat_type_, coord_, param_,dt_write_) 
 into old_param_;
 if true = old_param_.inserted THEN
   result_.type_result := 'inserted';
   result_.count := 1;
  -- RAISE NOTICE '- time operation: %', clock_timestamp()-rrr;
   RETURN result_;
 END IF;
 has_new_ := FALSE;
 param_to_update_ := '[]'::jsonb;
 ----------------------------первая часть------------------------------------------
 ------------- если пришли новые данные, то добавляем их в запись
 FOR cur_descr_ IN -- ищем, чего не хватает в базе...
     select * from meteo.find_descrname_except( old_param_.param, param_)
 LOOP
     has_new_ := TRUE;
 	 select json_build_object('descrname',cur_descr_) into descr_;
	 FOR cur_value_ IN -- добавляем новое
    	 select * from jsonb_array_elements_text(param_)
	 LOOP
		 select cur_value_ @> descr_ into res_;
		 IF TRUE = res_ THEN
			select param_to_update_ || cur_value_ into param_to_update_;
            EXIT;
		 END IF;
	 END LOOP;
 END LOOP;
 select meteo.try_create_partition('report',dt_) into table_name_;

 IF TRUE = has_new_ THEN --если были изменения то добавляем
 	--RAISE notice '- были новые данные %',table_name_ ;
      query_ := format('update %s SET dt_update = now(), "param" = "param" || %L::jsonb
		where
		   "data_type"    = %L::smallint and
		   "dt"           = %L::timestamp and
		   "level"        = %L::real and 
		   "level_type"   = %L::smallint and
		   "station"      = %L and
		   "station_type" = %L::smallint and
		   "location"     = %L;',
			table_name_, param_to_update_,data_type_,dt_,level_,lev_type_,station_,stat_type_,coord_);	  
		execute query_;
 END IF;
 
 if TRUE = need_replace_ THEN
	 has_new_ := false;
	 ----------------------------вторая часть------------------------------------------
	 ------------- если пришли данные, которые уже есть, то заменяем их в записи
	 param_to_update_ := old_param_.param;
	-- RAISE notice ' old_param_.param: %', old_param_.param;
	 select  jsonb_array_length(old_param_.param) into count_value_;
	 count_value_ := count_value_-1;

	 FOR cur_descr_ IN -- ищем, что уже есть в базе...
		 select * from meteo.find_descrname_intersect( old_param_.param, param_)
	 LOOP
		 select  jsonb_array_length(param_to_update_) into count_value_;
		 count_value_ := count_value_-1;
		 has_new_ := TRUE;
		 select json_build_object('descrname',cur_descr_) into descr_;
     	 --RAISE notice ' ur_descr_ %',cur_descr_;
		 i_ := 0;
		 WHILE i_ <= count_value_ 
		 --FOR i_ IN 0..count_value_ -- удаляем записи
			 LOOP 
			 --RAISE notice ' i_ % count_value_ %',i_,count_value_;
			 cur_value_ := param_to_update_->i_;
			 select cur_value_ @> descr_ into res_;
			 IF TRUE = res_ THEN
               -- RAISE notice ' были изменения v % % iz %',cur_value_,i_,count_value_;
		    	--RAISE notice ' __________find.param: % %',descr_, param_to_update_;
				select param_to_update_ - (i_) into param_to_update_;
                --RAISE notice ' ntgth %',param_to_update_;
				count_value_ := count_value_-1;
				i_ :=0;	 
				else 
				i_:=i_+1;
			 END IF;
			 
		 END LOOP;
		 --RAISE notice ' __________find.param: %', old_param_.param;
		 FOR cur_value_ IN -- добавляем новое
			 select * from jsonb_array_elements_text(param_)
		 LOOP
			 select cur_value_ @> descr_ into res_;
			 IF TRUE = res_ THEN
				select param_to_update_ || cur_value_ into param_to_update_;
			 END IF;
		 END LOOP;
	 END LOOP;
	-- RAISE notice ' find.param: %', param_to_update_;
	 IF TRUE = has_new_ THEN -- если были изменения то добавляем
      query_ := format('update %s SET dt_update = now(), "param" = %L::jsonb
		where
		   "data_type"    = %L::smallint and
		   "dt"           = %L::timestamp and
		   "level"        = %L::real and 
		   "level_type"   = %L::smallint and
		   "station"      = %L and
		   "station_type" = %L::smallint and
		   "location"     = %L;',
			table_name_,param_to_update_,data_type_,dt_,level_,lev_type_,station_,stat_type_,coord_);	  
		execute query_;
		--RAISE notice ' обновили %', param_to_update_;
        result_.type_result := 'updated';
        result_.count := 1;
	 END IF;
 END IF;
-- RAISE NOTICE ' -time operation: %', clock_timestamp()-rrr;

 RETURN result_;
END;$$;


ALTER FUNCTION meteo.add_meteodata(dt_ timestamp without time zone, data_type_ smallint, level_ real, lev_type_ smallint, station_ text, stat_type_ smallint, coord_ public.geometry, param_ jsonb, need_replace_ boolean, dt_write_ timestamp without time zone) OWNER TO postgres;

--
-- Name: FUNCTION add_meteodata(dt_ timestamp without time zone, data_type_ smallint, level_ real, lev_type_ smallint, station_ text, stat_type_ smallint, coord_ public.geometry, param_ jsonb, need_replace_ boolean, dt_write_ timestamp without time zone); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.add_meteodata(dt_ timestamp without time zone, data_type_ smallint, level_ real, lev_type_ smallint, station_ text, stat_type_ smallint, coord_ public.geometry, param_ jsonb, need_replace_ boolean, dt_write_ timestamp without time zone) IS 'Добавление данных наблюдений в meteo.report';


--
-- Name: add_parameter(integer, integer, integer, integer, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_parameter(grib1_ integer, grib2_ integer, grb2discipline_ integer, grb2category_ integer, fullname_ text, unit_ text, bufr_descr_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.parameters ( grib1, grib2, grb2discipline, grb2category, fullname, unit, bufr_descr ) 
	                     VALUES (  grib1_, grib2_, grb2discipline_, grb2category_, fullname_, unit_, bufr_descr_ );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_parameter(grib1_ integer, grib2_ integer, grb2discipline_ integer, grb2category_ integer, fullname_ text, unit_ text, bufr_descr_ integer) OWNER TO postgres;

--
-- Name: add_station(text, smallint, jsonb, jsonb, text, public.geometry, integer, real, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer, boolean); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_station(station_ text, st_type_ smallint, st_index_ jsonb, fullname_ jsonb, source_ text, coord_ public.geometry, alt_ integer, water_depth_ real, last_update_ timestamp without time zone, last_used_ timestamp without time zone, check_date_ timestamp without time zone, country_ integer, operational_ boolean) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.stations ( station, st_type, st_index, fullname, source, coord, alt, water_depth, last_update, last_used, check_date, country, operational ) 
	                     VALUES (  station_, st_type_, st_index_, fullname_, source_, coord_, alt_, water_depth_, last_update_, last_used_, check_date_, country_, operational_  );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_station(station_ text, st_type_ smallint, st_index_ jsonb, fullname_ jsonb, source_ text, coord_ public.geometry, alt_ integer, water_depth_ real, last_update_ timestamp without time zone, last_used_ timestamp without time zone, check_date_ timestamp without time zone, country_ integer, operational_ boolean) OWNER TO postgres;

--
-- Name: add_station(text, smallint, jsonb, jsonb, text, integer, timestamp without time zone, timestamp without time zone, timestamp without time zone, boolean, public.geometry, real, integer, jsonb, double precision); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_station(station_ text, station_type_ smallint, index_ jsonb, name_ jsonb, source_ text, alt_ integer, last_update_ timestamp without time zone, last_used_ timestamp without time zone, check_ timestamp without time zone, operational_ boolean, location_ public.geometry, water_depth_ real, country_ integer, hydro_levels_ jsonb, airstrip_direction_ double precision) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.stations ( station, station_type, index, name, source, alt, last_update, last_used, "check", operational, location, water_depth, country, hydro_levels, airstrip_direction ) 
	                     VALUES (  station_, station_type_, index_, name_, source_, alt_, last_update_, last_used_, check_, operational_, location_, water_depth_, country_, hydro_levels_, airstrip_direction_  );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_station(station_ text, station_type_ smallint, index_ jsonb, name_ jsonb, source_ text, alt_ integer, last_update_ timestamp without time zone, last_used_ timestamp without time zone, check_ timestamp without time zone, operational_ boolean, location_ public.geometry, water_depth_ real, country_ integer, hydro_levels_ jsonb, airstrip_direction_ double precision) OWNER TO postgres;

--
-- Name: add_station_type(integer, text, integer, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_station_type(station_type_ integer, description_ text, min_data_type_ integer, max_data_type_ integer, comment_ text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context text;
BEGIN
	INSERT INTO meteo.station_types ( type, description, min_data_type, max_data_type, comment ) 
	                     VALUES (  station_type_, description_, min_data_type_, max_data_type_, comment_  );
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.add_station_type(station_type_ integer, description_ text, min_data_type_ integer, max_data_type_ integer, comment_ text) OWNER TO postgres;

--
-- Name: check_fs_table(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.check_fs_table(tablename_ text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
  query_ TEXT;
BEGIN
  PERFORM 1 FROM pg_tables WHERE tablename = tablename_ LIMIT 1;
  IF NOT FOUND
  THEN
    query_ := format(
	  'CREATE TABLE meteo.%s (
		id bigint NOT NULL,
		filename text NOT NULL,
		filemd5 text NOT NULL,
		filesize integer NOT NULL,
		dt_write timestamp without time zone DEFAULT  timezone(''utc''::text, now()) NOT NULL
	   );
		CREATE SEQUENCE meteo.%s_id_seq
                START WITH 1
                INCREMENT BY 1
                NO MINVALUE
                NO MAXVALUE
                CACHE 1;
        ALTER SEQUENCE meteo.%s_id_seq OWNED BY meteo.%s.id;
        ALTER TABLE ONLY meteo.%s ALTER COLUMN id SET DEFAULT nextval(''meteo.%s_id_seq''::regclass);
        ALTER TABLE ONLY meteo.%s
               ADD CONSTRAINT %s_filemd5_uniq UNIQUE (filemd5);
        ALTER TABLE ONLY meteo.%s
               ADD CONSTRAINT %s_pkey PRIMARY KEY (id);
		CREATE UNIQUE INDEX %s_filemd5_idx
          ON meteo.%s USING btree
          (filemd5 COLLATE pg_catalog."default" ASC NULLS LAST)
          TABLESPACE pg_default;

        CREATE UNIQUE INDEX %s_id_idx
          ON meteo.%s USING btree
          (id ASC NULLS LAST)
          TABLESPACE pg_default;
	  ', tablename_,     	  tablename_, tablename_, 
							  tablename_, tablename_,
							  tablename_, tablename_,
							  tablename_, tablename_,
							  tablename_, tablename_,
							  tablename_, tablename_, 
							  tablename_, tablename_
	);
	EXECUTE query_;
  END IF;
  RETURN true;
END;$$;


ALTER FUNCTION meteo.check_fs_table(tablename_ text) OWNER TO postgres;

--
-- Name: create_new_partition(text, timestamp without time zone, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.create_new_partition(parent_table_name text, partition_date timestamp without time zone, partition_name text) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
 sql text;
BEGIN
 -- Notifying
 RAISE NOTICE 'A new % partition will be created: %', parent_table_name, partition_name;

 select format('CREATE TABLE IF NOT EXISTS %s (CHECK (
         dt  > ''%s'' AND
         dt  <= ''%s''))
         INHERITS (%s)', partition_name,  partition_date,
               partition_date + interval '1 day', parent_table_name) into sql;
 -- New table, inherited from a master one
 EXECUTE sql;
 PERFORM meteo.index_partition(partition_name);
END;
$$;


ALTER FUNCTION meteo.create_new_partition(parent_table_name text, partition_date timestamp without time zone, partition_name text) OWNER TO postgres;

--
-- Name: delete_station(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.delete_station(id_ bigint) RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ double precision;
BEGIN
result_ := 1;
IF id_ IS NOT NULL THEN
	DELETE FROM meteo.stations WHERE id = id_;
ELSE
	result_ := 0; 
END IF;
RETURN result_;
END;
$$;


ALTER FUNCTION meteo.delete_station(id_ bigint) OWNER TO postgres;

--
-- Name: f_create_index(character varying, character varying[], character varying, character varying); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.f_create_index(character varying, character varying[], character varying DEFAULT 'btree'::character varying, character varying DEFAULT NULL::character varying) RETURNS integer
    LANGUAGE plpgsql
    AS $_$
declare
    tableName alias for $1;
    columnNames alias for $2;
    indexMethod alias for $3;
    indexName alias for $4;

    q varchar;
    sColumns varchar;
begin
    if(tableName is null or trim(tableName) = '') then
        return -1;
    end if;
    
    if(ColumnNames is null or array_upper(columnNames, 1) = 0) then
        return -1;
    end if;

    sColumns = array_to_string(columnNames, ',');

    q = 'create index ';
    if(indexName is not null) then
        q = q || indexName || ' ';
    end if;

    q = q || 'on ' || tableName;

    if(indexMethod is not null) then
        q = q || ' using ' || indexMethod;
    end if;

    q = q || ' (' || sColumns || ')';

    execute q;

    return 1;
    
end
$_$;


ALTER FUNCTION meteo.f_create_index(character varying, character varying[], character varying, character varying) OWNER TO postgres;

--
-- Name: f_is_index_exist(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.f_is_index_exist(text) RETURNS boolean
    LANGUAGE plpgsql
    AS $_$
declare
    iName alias for $1;

    iCount int4;
begin

    select count(pgi.indexname)  into iCount
    from
        pg_indexes pgi
    where
        pgi.indexname = iName;

    if(iCount >= 1) then
        return TRUE;
    else
        return FALSE;
    end if;

    return FALSE;
    
    exception when others then
        return FALSE;

end
$_$;


ALTER FUNCTION meteo.f_is_index_exist(text) OWNER TO postgres;

--
-- Name: f_is_type_exist(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.f_is_type_exist(text) RETURNS integer
    LANGUAGE plpgsql
    AS $_$
declare
  itypname alias for $1;
  tcount int4;
begin
  select count(typname) into tcount from pg_type where typname = itypname;
  return tcount;
end
$_$;


ALTER FUNCTION meteo.f_is_type_exist(text) OWNER TO postgres;

--
-- Name: find_descrname_except(jsonb, jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_descrname_except(old_ jsonb, new_ jsonb) RETURNS SETOF text
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY with new_param as (
		select * from jsonb_populate_recordset(null::meteo.meteovalue_type, new_)
	  ), old_param as (
		  select * from jsonb_populate_recordset(null::meteo.meteovalue_type, old_)
	  ) 
	 select descrname from new_param EXCEPT select descrname from old_param;
	  
END;$$;


ALTER FUNCTION meteo.find_descrname_except(old_ jsonb, new_ jsonb) OWNER TO postgres;

--
-- Name: FUNCTION find_descrname_except(old_ jsonb, new_ jsonb); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.find_descrname_except(old_ jsonb, new_ jsonb) IS 'Поиск одинаковых descrname в двух meteodata json';


--
-- Name: find_descrname_intersect(jsonb, jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_descrname_intersect(old_ jsonb, new_ jsonb) RETURNS SETOF text
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY with new_param as (
		select * from jsonb_populate_recordset(null::meteo.meteovalue_type, new_)
	  ), old_param as (
		  select * from jsonb_populate_recordset(null::meteo.meteovalue_type, old_)
	  ) 
	 select new_param.descrname from new_param, old_param	where new_param.cor_number>old_param.cor_number 
	  INTERSECT  select  descrname from old_param;  
END;$$;


ALTER FUNCTION meteo.find_descrname_intersect(old_ jsonb, new_ jsonb) OWNER TO postgres;

--
-- Name: find_file_description(text, text, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_file_description(tablename_ text, filename_ text, id_ bigint) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
 query_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  query_ = FORMAT('SELECT id, filename, filemd5, filesize, dt_write
    FROM meteo.%I WHERE filename = %L OR id = %L', tablename_, filename_, id_ );
	RETURN QUERY EXECUTE query_;
END;$$;


ALTER FUNCTION meteo.find_file_description(tablename_ text, filename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: find_file_list(text, text, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_file_list(tablename_ text, filename_ text, id_ bigint) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
 query_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  query_ = FORMAT('SELECT id, filename, filemd5, filesize, dt_write
    FROM meteo.%I WHERE (%L IS NULL OR filename = %L) AND (%L IS NULL OR id = %L)', tablename_, filename_, filename_, id_, id_ );
	RETURN QUERY EXECUTE query_;
END;$$;


ALTER FUNCTION meteo.find_file_list(tablename_ text, filename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: find_forecast_accuracy_records(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_forecast_accuracy_records(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.find_forecast_accuracy_records_curs(filters, sort_column,sort_order,qskip,qslimit ) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;	
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.find_forecast_accuracy_records(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: find_forecast_accuracy_records_curs(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_forecast_accuracy_records_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$DECLARE
   queryTemplate text := 'SELECT * FROM meteo.forecast_opr WHERE %s ORDER BY %s %s LIMIT %s OFFSET %s';
   sortOrderName text:= '';
   queryString text:='';
   matchString text:= '';
   ref refcursor := 'rescursor';
BEGIN   
  if 1 = sort_order THEN
    sortOrderName = 'ASC';
  ELSE
    sortOrderName = 'DESC';
  END IF;
   
  IF array_length(filters, 1) <> 0
  THEN
        matchString := global.parse_json_condition(filters);
  ELSE
    matchString := 'TRUE';
  END IF;
    
  queryString:= format(queryTemplate, matchString, sort_column, sortOrderName, qslimit, qskip);
  
  OPEN ref FOR EXECUTE queryString;  
  return ref;
END;

$$;


ALTER FUNCTION meteo.find_forecast_accuracy_records_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: find_forecast_records(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_forecast_records(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.find_forecast_records_curs(filters, sort_column,sort_order,qskip,qslimit ) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;	
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.find_forecast_records(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: find_forecast_records_curs(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_forecast_records_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$DECLARE
   queryTemplate text := 'SELECT id, dt, dt_beg, dt_end, station, level, level_type, center, hour,  
               descrname, param->>''code'', param->>''method_title'', (param->>''quality'')::integer, observ, field
	           FROM meteo.forecast WHERE %s ORDER BY %s %s LIMIT %s OFFSET %s';   
   sortOrderName text:= '';
   queryString text:='';
   matchString text:= '';
   ref refcursor := 'rescursor';
BEGIN   
  if 1 = sort_order THEN
    sortOrderName = 'ASC';
  ELSE
    sortOrderName = 'DESC';
  END IF;
  
  IF array_length(filters, 1) <> 0
  THEN
        matchString := global.parse_json_condition(filters);
  ELSE
    matchString := 'TRUE';
  END IF;
    
  queryString:= format(queryTemplate, matchString, sort_column, sortOrderName, qslimit, qskip);
  
  OPEN ref FOR EXECUTE queryString;  
  return ref;
END;

$$;


ALTER FUNCTION meteo.find_forecast_records_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: get_aero_date_list(timestamp without time zone, timestamp without time zone, integer[], text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_aero_date_list(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, data_type_ integer[], station_ text) RETURNS SETOF meteo.aero_date_list
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
return query select distinct (dt) 
from  meteo.report where 
       	  dt BETWEEN dt_start_ AND dt_end_
		   and (data_type_ IS NULL or data_type = ANY (data_type_))
		   and station  = station_ ; 
  

END;$$;


ALTER FUNCTION meteo.get_aero_date_list(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, data_type_ integer[], station_ text) OWNER TO postgres;

--
-- Name: get_available_center_obanal(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer[], integer[], integer[], integer[], integer, integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_center_obanal(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer, descrnum_ integer[]) RETURNS SETOF meteo.center_descr
    LANGUAGE plpgsql
    AS $$DECLARE
result_ meteo.center_descr;
BEGIN
for result_ in 
select DISTINCT ON (cn.center) cn.center, cn.full_name, cn.short_name, cn.priority 
	   from (SELECT * FROM meteo.obanal where  
	       (dt_start_ IS NULL or dt_end_ is NULL or  (dt BETWEEN dt_start_ AND dt_end_))
	       and  (fdt_start_ IS NULL or fdt_end_ is NULL or (forecast_start BETWEEN fdt_start_ AND fdt_end_))
		   and (level_ IS NULL or level = ANY (level_))  
		   and (level_type_ IS NULL or level_type = ANY (level_type_))  
		   and (center_ IS NULL or center = ANY (center_))  
		   and (hour_ IS NULL or hour = ANY (hour_))  
		   and (model_ IS NULL or model = model_)  
		   and (descrnum_ IS NULL or descr = ANY (descrnum_))  
		   ) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   
		   --GROUP BY (cn.center, cn.full_name, cn.short_name, cn.priority )
		   
LOOP
RETURN next result_; 
END LOOP;
		   

END;$$;


ALTER FUNCTION meteo.get_available_center_obanal(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer, descrnum_ integer[]) OWNER TO postgres;

--
-- Name: get_available_data_for_meteosummary(timestamp without time zone, timestamp without time zone, integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_data_for_meteosummary(dt_ timestamp without time zone, fdt_ timestamp without time zone, level_type_ integer[], descrnum_ integer[]) RETURNS meteo.data_for_meteosummary
    LANGUAGE plpgsql
    AS $$DECLARE
result_ meteo.data_for_meteosummary;
BEGIN
result_:=null;
SELECT o.forecast_start,o.center,o.hour, o.model, o.ids, o.descr
	from (SELECT forecast_start,center,hour,model,
		json_agg(fs_id) as ids,
		json_agg(descr) as descr FROM meteo.obanal WHERE  
		(dt_ IS NULL or  (dt = dt_))
		and (fdt_ IS NULL  or (forecast_start = fdt_))
		and (level_type_ IS NULL or level_type = any (level_type_))  
		and (descrnum_ IS NULL or descr = any (descrnum_))  
		group by (center, model,hour, forecast_start))as o
	LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
	ORDER BY cn.priority , o.hour limit 1 into result_;
RETURN result_;
END;$$;


ALTER FUNCTION meteo.get_available_data_for_meteosummary(dt_ timestamp without time zone, fdt_ timestamp without time zone, level_type_ integer[], descrnum_ integer[]) OWNER TO postgres;

--
-- Name: get_data_lumens(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer[], integer[], integer[], integer[], integer[], integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_data_lumens(SensType_ text[],SensNb_ integer[],Time_max_ timestamp with time zone,Time_05_ timestamp with time zone,Time_03_07_ timestamp with time zone,Direction_ integer[],Direction_ex_ integer[],Range_ph_ integer[],Ph_prec_ integer[],Range_ps_ integer[],A1_ integer[],A2_ integer[],Tfr_ integer[],T1_ integer[],T2_ integer[],) RETURNS SETOF meteo.field_data_lumens
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.field_data_lumens;
BEGIN
--raise notice '%',level_;
for result_ in 
select o.id, o.SensType, o.SensNb, o.Time_max, o.Time_05, o.Time_03_07, o.Direction, o.Direction_ex, o.Range_ph, o.Ph_prec, o.Range_ps, o.A1, o.A2, o.Tfr, o.T1, o.T2
	   from (SELECT * FROM meteo.lumens where
            and (SensType_   IS NULL or SensType = ANY (SensType_))
			and (SensNb_   IS NULL or SensNb = ANY (SensNb_))
			and (Time_max_   IS NULL or Time_max = ANY (Time_max_))
			and (Time_05_   IS NULL or Time_05 = ANY (Time_05_))
			and (Time_03_07_   IS NULL or Time_03_07 = ANY (Time_03_07_))
			and (Direction_   IS NULL or Direction = ANY (Direction_))
			and (Direction_ex_   IS NULL or Direction_ex = ANY (Direction_ex_))
			and (Range_ph_   IS NULL or Range_ph = ANY (Range_ph_))
			and (Ph_prec_   IS NULL or Ph_prec = ANY (Ph_prec_))
			and (Range_ps_   IS NULL or Range_ps = ANY (Range_ps_))
			and (A1_   IS NULL or A1 = ANY (A1_))
			and (A2_   IS NULL or A2 = ANY (A2_))
			and (Tfr_   IS NULL or Tfr = ANY (Tfr_))
			and (T1_   IS NULL or T1 = ANY (T1_))
			and (T2_   IS NULL or T2 = ANY (T2_))
LOOP
RETURN next result_; 
END LOOP;
		   

END;
$$;


ALTER FUNCTION meteo.get_available_data_lumens(SensType_ text[],SensNb_ integer[],Time_max_ timestamp with time zone,Time_05_ timestamp with time zone,Time_03_07_ timestamp with time zone,Direction_ integer[],Direction_ex_ integer[],Range_ph_ integer[],Ph_prec_ integer[],Range_ps_ integer[],A1_ integer[],A2_ integer[],Tfr_ integer[],T1_ integer[],T2_ integer[]) OWNER TO postgres;

--
-- Name: get_data_sensors(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer[], integer[], integer[], integer[], integer[], integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_data_sensors(SensNb_ integer[], SensType_ text[], hv_ integer[], sw_ integer[], SensLat_ integer[], SensLong_ integer[], Time_ timestamp with time zone, Thresh_ integer[], NoiseE_ integer[], NoiseХ_ integer[], NoiseY_ integer[], temp_ integer[]) RETURNS SETOF meteo.field_data_lumens
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.field_data_sensors;
BEGIN
--raise notice '%',level_;
for result_ in 
select o.id, o.SensNb, o.SensType, o.hv, o.sw, o.SensLat, o.SensLong, o.Time, o.Thresh, o.NoiseE, o.NoiseХ, o.NoiseY, o.temp
	   from (SELECT * FROM meteo.sensors where
            and (SensNb_  IS NULL or SensNb = ANY (SensNb_))
            and (SensType_  IS NULL or SensType = ANY (SensType_))
            and (hv_  IS NULL or hv = ANY (hv_))
            and (sw_  IS NULL or sw = ANY (sw_))
            and (SensLat_  IS NULL or SensLat = ANY (SensLat_))
            and (SensLong_  IS NULL or SensLong = ANY (SensLong_))
            and (Time_  IS NULL or Time = ANY (Time_))
            and (Thresh_  IS NULL or Thresh = ANY (Thresh_))
            and (NoiseE_  IS NULL or NoiseE = ANY (NoiseE_))
            and (NoiseХ_  IS NULL or NoiseХ = ANY (NoiseХ_))
            and (NoiseY_  IS NULL or NoiseY = ANY (NoiseY_))
            and (temp_  IS NULL or temp = ANY (temp_))
LOOP
RETURN next result_; 
END LOOP;
		   

END;
$$;


ALTER FUNCTION meteo.get_available_data_sensors(SensNb_ integer[], SensType_ text[], hv_ integer[], sw_ integer[], SensLat_ integer[], SensLong_ integer[], Time_ integer[], Thresh_ integer[], NoiseE_ integer[], NoiseХ_ integer[], NoiseY_ integer[], temp_ integer[]) OWNER TO postgres;

--
-- Name: get_available_data_obanal(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer[], integer[], integer[], integer[], integer[], integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_data_obanal(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer[], descrnum_ integer[], net_type_ integer[]) RETURNS SETOF meteo.field_data_descr
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.field_data_descr;
BEGIN
--raise notice '%',level_;
for result_ in 
select o.id, o.dt, o.descr, o.hour, o.model, o.level, o.level_type, o.center, o.net_type,
       o.forecast_start, o.forecast_end, o.fs_id, o.count_points
	   , cn.full_name, cn.short_name integer
	   , lt.full_descr ,cn.priority
	   from (SELECT * FROM meteo.obanal where  
	       (dt_start_ IS NULL or dt_end_ is NULL or  (dt BETWEEN dt_start_ AND dt_end_))
	       and  (fdt_start_ IS NULL or fdt_end_ is NULL or (forecast_start BETWEEN fdt_start_ AND fdt_end_))
		   and (level_ IS NULL or level = ANY (level_))  
		   and (level_type_ IS NULL or level_type = ANY (level_type_))  
		   and (center_ IS NULL or center = ANY (center_))  
		   and (hour_ IS NULL or hour = ANY (hour_))  
		   and (model_ IS NULL or model = ANY (model_))  
		   and (descrnum_ IS NULL or descr = ANY (descrnum_))  
		   and (net_type_ IS NULL or net_type = ANY (net_type_))) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   LEFT OUTER JOIN meteo.level_types lt on o.level_type = lt.type
		   ORDER BY o.dt DESC,o.hour,cn.priority DESC
LOOP
RETURN next result_; 
END LOOP;
		   

END;
$$;


ALTER FUNCTION meteo.get_available_data_obanal(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer[], descrnum_ integer[], net_type_ integer[]) OWNER TO postgres;

--
-- Name: get_available_grib(timestamp without time zone, timestamp without time zone, integer, integer, integer, integer, integer, integer[], integer, integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_grib(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ integer, level_type_ integer, center_ integer, hour_ integer, model_ integer, descrnum_ integer[], step_lat_ integer, step_lon_ integer, forecast_beg_ timestamp without time zone, forecast_end_ timestamp without time zone) RETURNS SETOF meteo.grib_data_descr
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.grib_data_descr;

BEGIN
for result_ in 
SELECT 
--o.id,
o.dt,
jsonb_build_object('d', array_agg(distinct(o.descr))),
--o.descr,
hour,
o."procType",
o.level,
o."levelType",
o.center,
o."stepLat",
o."stepLon",
o.dt1,
o.dt2,
o."timeRange",
cn.full_name,
cn.short_name,
lt.full_descr
	   from (SELECT * FROM meteo.grib where  
	       (dt_start_ IS NULL or dt_end_ is NULL or  (dt BETWEEN dt_start_ AND dt_end_))
		   and (level_ IS NULL or level = level_)  
		   and (level_type_ IS NULL or "levelType" = level_type_)  
		   and (center_ IS NULL or center = center_)  
		   and (hour_ IS NULL or hour = hour_)  
		   and (model_ IS NULL or "procType" = model_)  

		   and (step_lat_ IS NULL or "stepLat" = step_lat_)  
		   and (step_lon_ IS NULL or "stepLon" = step_lon_)  
		   and (forecast_beg_ IS NULL or "dt1" = forecast_beg_)  --TODO может смотреть не на точное совпадение....
		   and (forecast_end_ IS NULL or "dt2" = forecast_end_)  --TODO
    	   and (descrnum_ IS NULL or descr = ANY (descrnum_))) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   LEFT OUTER JOIN meteo.level_types lt on o."levelType" = lt.type
		   GROUP BY dt, hour, o.center, cn.full_name,cn.short_name, lt.full_descr, level, 
		   "levelType", "procType", "stepLat" , "stepLon"
		   , dt1, dt2, "timeRange"
		   ORDER BY o.dt DESC,o.hour
LOOP
RETURN next result_; 
END LOOP;

END;
$$;


ALTER FUNCTION meteo.get_available_grib(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ integer, level_type_ integer, center_ integer, hour_ integer, model_ integer, descrnum_ integer[], step_lat_ integer, step_lon_ integer, forecast_beg_ timestamp without time zone, forecast_end_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_available_hours_obanal(timestamp without time zone, integer[], integer, integer[], integer[], integer[], integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_hours_obanal(dt_ timestamp without time zone, center_ integer[], model_ integer, level_ integer[], level_type_ integer[], descrnum_ integer[], minimum_level_count_ integer DEFAULT NULL::integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
 ref_ refcursor; 
 rec RECORD;
BEGIN
for rec in 
    SELECT dt,center,hour, level FROM (
      SELECT dt,center,hour, array_agg(level) as level, max(descr) as descr FROM (
        SELECT DISTINCT ON (dt,center,hour,level) dt,center,hour, level, array_agg(descr) as descr FROM meteo.obanal 
          where  
  	        dt = dt_
	  	    and (center_ IS NULL or center = ANY (center_))  
		    and (model_ IS NULL or model = model_)  
		    and (level_ IS NULL or level = ANY (level_))  
		    and (level_type_ IS NULL or level_type = ANY (level_type_))  
		    and (descrnum_ IS NULL or descr = ANY(descrnum_))  
            GROUP BY dt,center,hour,level
        ) q WHERE (descrnum_ IS NULL OR descrnum_ <@ descr) 
            GROUP BY dt,center,hour
    ) qq WHERE (minimum_level_count_ IS NULL OR array_length( level, 1 )>= minimum_level_count_ )

LOOP
	RETURN next rec; 
END LOOP;
		   

END;
$$;


ALTER FUNCTION meteo.get_available_hours_obanal(dt_ timestamp without time zone, center_ integer[], model_ integer, level_ integer[], level_type_ integer[], descrnum_ integer[], minimum_level_count_ integer) OWNER TO postgres;

--
-- Name: get_available_hours_obanal_old(timestamp without time zone, integer[], integer, integer[], integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_hours_obanal_old(dt_ timestamp without time zone, center_ integer[], model_ integer, level_ integer[], level_type_ integer[], descrnum_ integer[]) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
 ref_ refcursor; 
 rec RECORD;
BEGIN
for rec in 
    SELECT q.* FROM (
    SELECT DISTINCT ON (dt,center,hour) min(dt) as dt,center,hour, array_agg(descr) as descr FROM meteo.obanal where  
	       dt = dt_
		   and (center_ IS NULL or center = ANY (center_))  
		   and (model_ IS NULL or model = model_)  
		   and (level_ IS NULL or level = ANY (level_))  
		   and (level_type_ IS NULL or level_type = ANY (level_type_))  
		   and (descrnum_ IS NULL or descr = ANY(descrnum_))  
           GROUP BY dt,center,hour) q WHERE (descrnum_ IS NULL or descrnum_ <@ descr)
LOOP
	RETURN next rec; 
END LOOP;
		   

END;
$$;


ALTER FUNCTION meteo.get_available_hours_obanal_old(dt_ timestamp without time zone, center_ integer[], model_ integer, level_ integer[], level_type_ integer[], descrnum_ integer[]) OWNER TO postgres;

--
-- Name: get_available_ocean(timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_ocean(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) RETURNS SETOF meteo.available_ocean
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
return query select * from
(select 
	dt,
	station,
	ST_AsGeoJson(location)::jsonb as coord, 
	max(level) as maxlvl,
	count(id) as count
 from (SELECT *, jsonb_array_elements(param) as d
		   FROM meteo.report 
	   where  dt BETWEEN dt_start_ AND dt_end_
  	   		  and station_type = 6
	          and level_type <> 256
	   		  ) as dd where d::jsonb->'descrname' ?| '{ "Sn", "Tw"}'
	          group by dt,station,location) as foo where count > 10;
	   
END;$$;


ALTER FUNCTION meteo.get_available_ocean(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_available_radar(integer, real, real, integer[], timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_radar(data_type_ integer, h1_ real, h2_ real, product_ integer[], start_dt_ timestamp without time zone, end_dt_ timestamp without time zone) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$DECLARE
 ref_ refcursor; 
 mviews RECORD;
BEGIN

SELECT * FROM meteo.get_available_radar_curs(data_type_, h1_, h2_, product_, start_dt_, end_dt_) into ref_;

LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;

END;$$;


ALTER FUNCTION meteo.get_available_radar(data_type_ integer, h1_ real, h2_ real, product_ integer[], start_dt_ timestamp without time zone, end_dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_available_radar(integer, real, real, integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_radar(data_type_ integer, h1_ real, h2_ real, product_ integer, start_dt_ timestamp without time zone, end_dt_ timestamp without time zone) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$DECLARE
 ref_ refcursor; 
 mviews RECORD;
BEGIN

SELECT * FROM meteo.get_available_radar_curs(data_type_, h1_, h2_, ARRAY[ product_ ], start_dt_, end_dt_) into ref_;

LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;

END;$$;


ALTER FUNCTION meteo.get_available_radar(data_type_ integer, h1_ real, h2_ real, product_ integer, start_dt_ timestamp without time zone, end_dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_available_radar_curs(integer, real, real, integer[], timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_radar_curs(data_type_ integer, h1_ real, h2_ real, product_ integer[], start_dt_ timestamp without time zone, end_dt_ timestamp without time zone) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
 ref refcursor := 'rescursor';
BEGIN
  OPEN ref 	FOR SELECT 	* FROM meteo.radarmap WHERE 
  ( data_type_ is NULL OR data_type_ = "data_type" ) AND
  ( h1_ is NULL OR h1_ = "h1" ) AND
  ( h2_ is NULL OR h2_ = "h2" ) AND
  ( product_ is NULL OR "product" = ANY(product_) ) AND
  ( start_dt_ is NULL OR start_dt_ <= "dt" ) AND
  ( end_dt_ is NULL OR "dt" <= end_dt_ );
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_available_radar_curs(data_type_ integer, h1_ real, h2_ real, product_ integer[], start_dt_ timestamp without time zone, end_dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_available_sigwx(timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_sigwx(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
 
return query select dt, dt_beg, dt_end,
                    center, level_lo, level_hi,
                    count(id), jsonb_agg(fileid) from meteo.sigwx
					where dt BETWEEN dt_start_ AND dt_end_
	                group by dt, dt_beg, dt_end, center, level_lo, level_hi;

END;$$;


ALTER FUNCTION meteo.get_available_sigwx(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_available_stations(timestamp without time zone, timestamp without time zone, integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_stations(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, data_type_ integer[]) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
 
return query select station_type, jsonb_agg(DISTINCT station) from  meteo.report where 
       dt BETWEEN dt_start_ AND dt_end_
       and (data_type_ IS NULL or data_type = ANY (data_type_))  
  group by station_type;

END;$$;


ALTER FUNCTION meteo.get_available_stations(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, data_type_ integer[]) OWNER TO postgres;

--
-- Name: get_available_synmobsea(timestamp without time zone, timestamp without time zone, text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_synmobsea(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, descrname_ text[]) RETURNS SETOF meteo.available_synmobsea
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
return query 
select 
	dt,
	station,
	ST_AsGeoJson(location)::jsonb as coord, 
	count(id) as count
 from (SELECT *, jsonb_array_elements(param) as d
		   FROM meteo.report 
	   where  dt BETWEEN dt_start_ AND dt_end_
  	   		  and station_type = 1
	   		  ) as dd where d::jsonb->'descrname' ?| descrname_
	          group by dt,station,location;
	   
END;$$;


ALTER FUNCTION meteo.get_available_synmobsea(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, descrname_ text[]) OWNER TO postgres;

--
-- Name: get_available_zond(timestamp without time zone, integer[], integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_zond(dt_ timestamp without time zone, data_type_ integer[], level_type_ integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
return query select * from (select meteo.report.station_type,
        meteo.report.station, 
		ST_AsGeoJson(meteo.report.location)::jsonb as location, 
       meteo.stations.name as station_info,
	    count(meteo.report.id) as count 
from  meteo.report left join meteo.stations using (station, station_type) where 
       dt = dt_
  	   and data_type = ANY (data_type_)
	   and level_type = level_type_
	   group by station_type,station,meteo.report.location, meteo.stations.name ) as foo 
	   where count > 5;
END;$$;


ALTER FUNCTION meteo.get_available_zond(dt_ timestamp without time zone, data_type_ integer[], level_type_ integer) OWNER TO postgres;

--
-- Name: get_bufr_code(text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_bufr_code(bufr_code_ text[]) RETURNS TABLE(bufr_code text, bufr_description text, "values" jsonb)
    LANGUAGE plpgsql
    AS $$
DECLARE
    rec meteo.bufr_code%rowtype;
    bint_codes bigint[];
    bint_code bigint;
    code text;
BEGIN
	FOREACH code IN ARRAY bufr_code_
	LOOP
		bint_code := to_number(code, 'FM9999999999999999999');
		bint_codes := array_append(bint_codes, bint_code);
	END LOOP;
	FOR rec IN
        SELECT * FROM meteo.bufr_code WHERE bufr_code_ IS NULL OR meteo.bufr_code.bufr_code = ANY (bint_codes)
	LOOP
		bufr_code = rec.bufr_code;
		bufr_description = rec.description;
		values = rec.code_values;
		RETURN NEXT; -- возвращается текущая строка запроса
	END LOOP;
    RETURN;
END
$$;


ALTER FUNCTION meteo.get_bufr_code(bufr_code_ text[]) OWNER TO postgres;

--
-- Name: get_country(integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_country(priority_ integer) RETURNS TABLE(number integer, code text, icao text[], name text, name_en text, name_ru text, wmocode text, wmonumber integer)
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
RETURN QUERY
        SELECT 
		countries.world_number,
		countries.code,
		countries.icao,
		countries.name_common,
		countries.name_en,
		countries.name_ru,
		countries.wmocode,
		countries.wmonumber
		
        FROM 	meteo.countries AS countries
        
        WHERE 	priority_ IS NULL OR countries.priority = priority_;
END
$$;


ALTER FUNCTION meteo.get_country(priority_ integer) OWNER TO postgres;

--
-- Name: get_data_cover(timestamp without time zone, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_data_cover(dt_ timestamp without time zone, station_type_ integer) RETURNS real
    LANGUAGE plpgsql
    AS $$DECLARE
 count_total_ numeric;
 count_exist_ numeric;
 cover_ numeric;
BEGIN
	SELECT count(distinct(station)) FROM meteo.stations
	where station_type = station_type_ and operational = true into count_total_;
	
	SELECT count(distinct(station)) FROM meteo.stations_history
	where station_type = station_type_ and dt = dt_ into count_exist_;
	--SELECT count(distinct(station)) FROM meteo.report
	--where station_type = station_type_ and dt = dt_ into count_exist_;
	cover_ = 0;
   IF (count_total_ <> 0) THEN
     cover_ = round(count_exist_/count_total_ ,2);
   END IF;
   
return cover_; 
END;$$;


ALTER FUNCTION meteo.get_data_cover(dt_ timestamp without time zone, station_type_ integer) OWNER TO postgres;

--
-- Name: get_data_obanal(timestamp without time zone, integer[], integer[], integer[], integer[], integer, integer[], integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_data_obanal(dt_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer, descrnum_ integer[], net_type_ integer) RETURNS SETOF meteo.field_data_descr
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.field_data_descr;
BEGIN
for result_ in 
select o.id, o.dt, o.descr, o.hour, o.model, o.level, o.level_type, o.center, o.net_type,
       o.forecast_start, o.forecast_end, o.fs_id, o.count_points
	   , cn.full_name, cn.short_name 
	   , lt.full_descr ,cn.priority
	   from (SELECT * FROM meteo.obanal where  
	       (dt_ IS NULL  or  (dt = dt_ ))
		   and (level_ IS NULL or level = ANY (level_))  
		   and (level_type_ IS NULL or level_type = ANY (level_type_))  
		   and (center_ IS NULL or center = ANY (center_))  
		   and (hour_ IS NULL or hour = ANY (hour_))  
		   and (model_ IS NULL or model = model_)  
		   and (descrnum_ IS NULL or descr = ANY (descrnum_))  
		   and (net_type_ IS NULL or net_type = net_type_)) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   LEFT OUTER JOIN meteo.level_types lt on o.level_type = lt.type
		   
LOOP
RETURN next result_; 
END LOOP;
		   

END;
$$;


ALTER FUNCTION meteo.get_data_obanal(dt_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer, descrnum_ integer[], net_type_ integer) OWNER TO postgres;

--
-- Name: get_field_descr_po_id(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_field_descr_po_id(id_ bigint) RETURNS meteo.field_data_descr
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.field_data_descr;
BEGIN
SELECT o.id, o.dt, o.descr, o.hour, o.model, o.level, o.level_type, o.center, o.net_type,
       o.forecast_start, o.forecast_end, o.fs_id, o.count_points
	   , cn.full_name, cn.short_name 
	   , lt.full_descr, cn.priority 
	   from (SELECT * FROM meteo.obanal WHERE  
	       id = id_	        
		   ) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   LEFT OUTER JOIN meteo.level_types lt on o.level_type = lt.type
           limit 1 into result_;
		   --GROUP BY (cn.center, cn.full_name, cn.short_name, cn.priority )
RETURN result_;
END;
$$;


ALTER FUNCTION meteo.get_field_descr_po_id(id_ bigint) OWNER TO postgres;

--
-- Name: get_field_for_zond(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer[], integer[], integer[], integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_field_for_zond(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], descrnum_ integer[]) RETURNS SETOF meteo.field_for_zond
    LANGUAGE plpgsql
    AS $$DECLARE
result_ meteo.field_for_zond;
BEGIN
FOR result_ IN
WITH step1 AS (select 
       o.center, cn.priority, cn.full_name, cn.short_name, o.model, o.hour, o.level, o.forecast_start,
      jsonb_agg(json_build_object('descr', o.descr,'id', o.id,'fs_id', o.fs_id,'level_type', o.level_type)) AS descrs,
	  count(o.descr) AS count		   
	   FROM (
		   SELECT * FROM meteo.obanal WHERE  
	       (dt_start_ IS NULL OR dt_end_ IS NULL OR  (dt BETWEEN dt_start_ AND dt_end_))
	       and  (fdt_start_ IS NULL OR fdt_end_ IS NULL OR (forecast_start BETWEEN fdt_start_ AND fdt_end_))
		   and (level_ IS NULL OR level = ANY (level_))  
		   and (level_type_ IS NULL OR level_type = ANY (level_type_))  
		   and (center_ IS NULL OR center = ANY (center_))  
		   and (hour_ IS NULL OR hour = ANY (hour_))  
		   and (descrnum_ IS NULL OR descr = ANY (descrnum_))  
		   ) AS o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   GROUP BY (o.forecast_start, o.center, cn.priority, cn.full_name, cn.short_name, o.model, o.hour, o.level) ),
     step2 AS (
	   SELECT 
       count(*) AS count,		   
       center, priority, full_name, short_name, model, hour, forecast_start,
       json_agg(json_build_object('level', level,'descrs',descrs )) AS levels
	   FROM step1
	   GROUP BY (forecast_start, center, priority, full_name, short_name, model, hour)
	   ORDER BY count DESC, hour,priority 
	 ),
	 step3 AS (
	   SELECT 
       center, priority, full_name, short_name,  forecast_start,
       jsonb_agg(json_build_object('hour', hour,'count',step2.count,'model',model,'levels',levels ))->0 AS hours
   	   --count(*) AS count		   
	   FROM step2
	   GROUP BY (forecast_start, center, priority, full_name, short_name )
	 ) 
	 ,
	 step4 AS (
	   SELECT 
       forecast_start,
       jsonb_agg(json_build_object('center', step3.center,'priority', step3.priority,'full_name',
								  step3.full_name,'short_name',step3.short_name,
								   'model',hours->'model','hour',hours->'hour',
								  'count',hours->'count','levels',hours->'levels')) AS zonds
	   FROM step3
		   GROUP BY (forecast_start) 
	 )
	 SELECT * FROM step4
	 LOOP
RETURN next result_; 
END LOOP;
		   

END;$$;


ALTER FUNCTION meteo.get_field_for_zond(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], descrnum_ integer[]) OWNER TO postgres;

--
-- Name: get_field_for_zond_test(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer[], integer[], integer[], integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_field_for_zond_test(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], descrnum_ integer[]) RETURNS SETOF meteo.field_for_zond
    LANGUAGE plpgsql
    AS $$DECLARE
result_ meteo.field_for_zond;
BEGIN
for result_ in 
with step1 as (select 
       o.center, cn.priority, cn.full_name, cn.short_name, o.model, o.hour, o.level, o.forecast_start,
      jsonb_agg(json_build_object('descr', o.descr,'id', o.id,'path', o.path,'level_type', o.level_type)) as descrs,
	  count(o.descr) as count		   
	   from (
		   SELECT * FROM meteo.obanal where  
	       (dt_start_ IS NULL or dt_end_ is NULL or  (dt BETWEEN dt_start_ AND dt_end_))
	       and  (fdt_start_ IS NULL or fdt_end_ is NULL or (forecast_start BETWEEN fdt_start_ AND fdt_end_))
		   and (level_ IS NULL or level = ANY (level_))  
		   and (level_type_ IS NULL or level_type = ANY (level_type_))  
		   and (center_ IS NULL or center = ANY (center_))  
		   and (hour_ IS NULL or hour = ANY (hour_))  
		   and (descrnum_ IS NULL or descr = ANY (descrnum_))  
		   ) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   group by (o.forecast_start, o.center, cn.priority, cn.full_name, cn.short_name, o.model, o.hour, o.level) ),
     step2 as (
	   select 
       count(level)*step1.count as count,		   
       center, priority, full_name, short_name, model, hour, forecast_start,
       json_agg(json_build_object('level', level,'descrs',descrs )) as levels
	   from step1
	   group by (forecast_start, center, priority, full_name, short_name, model, hour, step1.count)
	   ORDER BY count DESC, hour,priority 
	 ),
	 step3 as (
	 		 select 
       center, priority, full_name, short_name,  forecast_start,
       jsonb_agg(json_build_object('hour', hour,'count',count,'model',model,'levels',levels ))->0 as hours,
   	   count(hour)*step2.count as count		   
	   from step2
		   GROUP BY (forecast_start, center, priority, full_name, short_name, step2.count )
           ORDER BY count 

	 )	 select * from step3

	 /*,
	 step4 as (
	 		 select 
       step3.forecast_start,
       jsonb_agg(json_build_object('center', step3.center,'priority', step3.priority,'full_name',
								  step3.full_name,'short_name',step3.short_name,
								   'model',hours->'model','hour',hours->'hour',
								  'count',hours->'count','levels',step2.levels)) as zonds,
	   count(step3.center)*step3.count as count		   
	   from step3,step2
		   group by (step3.forecast_start,step3.count ) 
	   ORDER BY count 
	 )
	 select * from step4*/
LOOP
RETURN next result_; 
END LOOP;
		   

END;$$;


ALTER FUNCTION meteo.get_field_for_zond_test(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], descrnum_ integer[]) OWNER TO postgres;

--
-- Name: get_gmi_types(integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_gmi_types(type_ integer[]) RETURNS TABLE(type integer, description text, parent integer, integrated boolean)
    LANGUAGE plpgsql
    AS $$
DECLARE
rec meteo.gmi_types%rowtype;
test_rec RECORD;
BEGIN
IF type_ IS NULL THEN
	FOR rec IN
	SELECT * FROM meteo.gmi_types
	LOOP
		type := rec.gmitype;
		description := rec.description;
		parent := rec.gmitype;
		integrated := true;
		RETURN NEXT;
		FOR test_rec IN SELECT * FROM jsonb_to_recordset(rec.subtype) AS X("type" integer, description text)
		LOOP
			type := test_rec.type;
			description := test_rec.description;
			parent := rec.gmitype;
			integrated := false;
			RETURN NEXT;
		END LOOP;
	END LOOP;
ELSE
	FOR rec IN
	SELECT * FROM meteo.gmi_types
	LOOP
	IF rec.gmitype = ANY (type_) THEN
		type := rec.gmitype;
		description := rec.description;
		parent := rec.gmitype;
		integrated := true;
		RETURN NEXT;
	ELSE
		FOR test_rec IN SELECT * FROM jsonb_to_recordset(rec.subtype) AS X("type" integer, description text) WHERE X.type = ANY(type_)
		LOOP
			type := test_rec.type;
			description := test_rec.description;
			parent := rec.gmitype;
			integrated := false;
			RETURN NEXT;
		END LOOP;
	END IF;
	END LOOP;
END IF;
RETURN;
END
$$;


ALTER FUNCTION meteo.get_gmi_types(type_ integer[]) OWNER TO postgres;

--
-- Name: get_last_meteo_data(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_last_meteo_data(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer, country_ integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$DECLARE
 ref_ refcursor; 
 mviews RECORD;
 mviews1 RECORD;
BEGIN
--get_last_meteo_data_curs????
SELECT * FROM meteo.get_last_meteo_data_curs(dt_start_ ,dt_end_ ,level_,
  lev_type_,data_type_,station_,stat_type_,descrname_,coord_,max_distace_, country_) into ref_;

LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;

END;$$;


ALTER FUNCTION meteo.get_last_meteo_data(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer, country_ integer) OWNER TO postgres;

--
-- Name: get_last_meteo_data_all_descr_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_last_meteo_data_all_descr_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[]) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor := '_result';
BEGIN

OPEN ref FOR select DISTINCT ON (station)
    id, 
 	dt,
	station,
	station_type,
	data_type,
	level,
	level_type,
	param,
	ST_AsGeoJson(location)::jsonb as location
   ,0.0::double precision as dist
    FROM meteo.report 
where  dt BETWEEN dt_start_ AND dt_end_
  	   and (level_ IS NULL or level =level_)  
	   and level_type = lev_type_
	   and (station_ IS NULL or station = ANY (station_))  
	   and (stat_type_ IS NULL or station_type = ANY (stat_type_))  
	   and (data_type_ IS NULL or data_type = ANY (data_type_))  
	   ORDER BY station, dt desc;
return ref; 

END;$$;


ALTER FUNCTION meteo.get_last_meteo_data_all_descr_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[]) OWNER TO postgres;

--
-- Name: get_last_meteo_data_by_coords_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_last_meteo_data_by_coords_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor := '_result';
BEGIN
--select  from meteo.report limit 1
OPEN ref FOR select DISTINCT ON (station)
    id, 
 	dt,
	station,
	station_type,
	data_type,
	level,
	level_type,
	jsonb_agg(d) as param,
	ST_AsGeoJson(location)::jsonb as location
   ,ST_DistanceSphere(coord_,location) as dist
 from (SELECT *, jsonb_array_elements(param) as d
	   FROM meteo.report 
where  dt BETWEEN dt_start_ AND dt_end_
  	   and (level_ IS NULL or level =level_)  
	   and level_type = lev_type_
	   and (station_ IS NULL or station = ANY (station_))  
	   and (stat_type_ IS NULL or station_type = ANY (stat_type_))  
	   and (data_type_ IS NULL or data_type = ANY (data_type_))  
	   and (max_distace_ IS NULL or ST_DistanceSphere(coord_,location) <= max_distace_)  
	  ) as dd where (descrname_ is null or d::jsonb->'descrname' ?| descrname_)
	   group by id, dt,	station, station_type, data_type, level, level_type,location,dist ORDER BY station, dt desc, dist;
 
	--coord_ geometry,
	--max_distace_ integer)
return ref; 
 
END;$$;


ALTER FUNCTION meteo.get_last_meteo_data_by_coords_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) OWNER TO postgres;

--
-- Name: get_last_meteo_data_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_last_meteo_data_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer, country_ integer) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
 ref refcursor := '_result';
BEGIN
	OPEN ref FOR select distinct on (station)
		id, dt,	station,station_info, station_type, data_type, level, level_type, 
		  jsonb_agg(d) as param,
		ST_AsGeoJson(location)::jsonb||jsonb_build_object('alt',alt) as location
	   ,0.0::double precision as dist
 from (SELECT  meteo.report.*, jsonb_array_elements(meteo.report.param) as d, meteo.stations.alt,
--	   '{}'::jsonb AS station_info
	   meteo.stations.name AS station_info,
	   meteo.stations.country
	   FROM meteo.report left join meteo.stations using (station, station_type)
	where  dt BETWEEN dt_start_ AND dt_end_
		   and (level_ IS NULL or level =level_)  
		   and level_type = lev_type_
		   and (station_ IS NULL or station = ANY (station_))  
		   and (stat_type_ IS NULL or station_type = ANY (stat_type_))  
		   and (data_type_ IS NULL or data_type = ANY (data_type_))  
		   and ( country_ IS NULL OR country = country_)
		  ) as dd where ( descrname_ IS NULL OR d::jsonb->'descrname' ?| descrname_ )
		    group by id, dt,	station,alt,station_info, station_type, data_type, level, level_type,location,dist ORDER BY station asc, dt desc;
    return ref;
	--coord_ geometry,
	--max_distace_ integer)

 
END;
$$;


ALTER FUNCTION meteo.get_last_meteo_data_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer, country_ integer) OWNER TO postgres;

--
-- Name: get_last_obanal(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer[], integer[], integer[], integer[], integer, integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_last_obanal(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer, descrnum_ integer[]) RETURNS SETOF meteo.field_data_descr
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.field_data_descr;
BEGIN
for result_ in 
select o.id, o.dt, o.descr, o.hour, o.model, o.level, o.level_type, o.center, o.net_type,
       o.forecast_start, o.forecast_end, o.fs_id, o.count_points
	   , cn.full_name, cn.short_name 
	   , lt.full_descr , cn.priority
	   from (SELECT * FROM meteo.obanal where  
	       (dt_start_ IS NULL or dt_end_ is NULL or  (dt BETWEEN dt_start_ AND dt_end_))
	       and  (fdt_start_ IS NULL or fdt_end_ is NULL or (forecast_start BETWEEN fdt_start_ AND fdt_end_))
		   and (level_ IS NULL or level = ANY (level_))  
		   and (level_type_ IS NULL or level_type = ANY (level_type_))  
		   and (center_ IS NULL or center = ANY (center_))  
		   and (hour_ IS NULL or hour = ANY (hour_))  
		   and (model_ IS NULL or model = model_)  
		   and (descrnum_ IS NULL or descr = ANY (descrnum_))  
		   ) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   LEFT OUTER JOIN meteo.level_types lt on o.level_type = lt.type
           ORDER BY o.dt DESC, o.forecast_start DESC, o.hour
		   --GROUP BY (cn.center, cn.full_name, cn.short_name, cn.priority )
		   
LOOP
RETURN next result_; 
END LOOP;
		   

END;
$$;


ALTER FUNCTION meteo.get_last_obanal(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer[], level_type_ integer[], center_ integer[], hour_ integer[], model_ integer, descrnum_ integer[]) OWNER TO postgres;

--
-- Name: get_level_types(integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_level_types(type_ integer[]) RETURNS TABLE(code_num integer, mean text, standart_value integer[])
    LANGUAGE plpgsql
    AS $$
DECLARE
query_ varchar;
BEGIN
RETURN QUERY
        SELECT 
		level_types.type,
		level_types.description,
		level_types.standard_value::integer[]
        FROM 	meteo.level_types AS level_types
        WHERE 	type_ IS NULL OR level_types.type = ANY (type_);
END
$$;


ALTER FUNCTION meteo.get_level_types(type_ integer[]) OWNER TO postgres;

--
-- Name: get_manual_forecast_result(bigint, text[], bigint[], bigint[], text, timestamp without time zone, timestamp without time zone, timestamp without time zone, bigint, timestamp without time zone, timestamp without time zone, boolean); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_manual_forecast_result(id_ bigint, stations_ text[], stations_types_ bigint[], region_id_ bigint[], user_id_ text, dt_ timestamp without time zone, dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, forecast_hour_ bigint, from_dt_ timestamp without time zone, to_dt_ timestamp without time zone, only_last_ boolean) RETURNS TABLE(id bigint, user_id text, dt timestamp without time zone, dt_start timestamp without time zone, dt_end timestamp without time zone, forecast_hour integer, forecast_text text, forecast_code text, region_id bigint, station jsonb, forecast_params jsonb, station_name text, region_name text)
    LANGUAGE plpgsql
    AS $$DECLARE
    stations_id bigint[];
BEGIN 

	IF stations_ IS NOT NULL THEN
        stations_id := array (SELECT s.id FROM unnest(  stations_, stations_types_ ) x(code,sttype)
        left join meteo.stations as s on s.station = code and s.station_type=sttype )  ;
    END IF;
   
	IF only_last_ IS NOT null and only_last_ is true THEN
        return QUERY SELECT  fm.id,
    		fm.user_id,
    		fm.dt,
    		fm.dt_start,
    		fm.dt_end,
    		fm.forecast_hour,
    		fm.forecast_text,
    		fm.forecast_code,
    		fm.region_id,
    		jsonb_set( to_jsonb( s ), '{location}', jsonb_set(ST_AsGeoJson(s.location)::jsonb,'{alt}', to_jsonb(0)) , true )#-'{hydro_levels}'  as station,
    		fm.forecast_params,
            s.name->>'ru' as station_name,  
            meteo.forecast_region.title as region_name
            FROM meteo.forecast_manual fm
            LEFT JOIN meteo.stations as s   ON s.id        = fm.station_id
            LEFT JOIN meteo.forecast_region ON meteo.forecast_region.id = fm.region_id
            WHERE   ( id_            IS NULL OR fm.id = id_ )
	                AND (  
	                        ( stations_    IS NULL AND  region_id_    IS NULL ) OR
	                        fm.region_id     = ANY (region_id_) OR fm.station_id    = ANY (stations_id)    
	                    )
            ORDER BY fm.dt_start DESC 
            limit 1;
    ELSE
    

	    return QUERY SELECT  fm.id,
	    		fm.user_id,
	    		fm.dt,
	    		fm.dt_start,
	    		fm.dt_end,
	    		fm.forecast_hour,
	    		fm.forecast_text,
	    		fm.forecast_code,
	    		fm.region_id,
	    		jsonb_set( to_jsonb( s ), '{location}', jsonb_set(ST_AsGeoJson(s.location)::jsonb,'{alt}', to_jsonb(0)) , true )#-'{hydro_levels}'  as station,
	    		fm.forecast_params,
	            s.name->>'ru' as station_name,  
	            meteo.forecast_region.title as region_name
	            FROM meteo.forecast_manual fm
	            LEFT JOIN meteo.stations as s   ON s.id        = fm.station_id
	            LEFT JOIN meteo.forecast_region ON meteo.forecast_region.id = fm.region_id
	            WHERE   ( id_            IS NULL OR fm.id = id_ )
	                AND (  
	                        ( stations_    IS NULL AND  region_id_    IS NULL ) OR
	                        fm.region_id     = ANY (region_id_) OR fm.station_id    = ANY (stations_id)    
	                    ) 
	                AND ( user_id_       IS NULL OR fm.user_id       = user_id_       )
	                AND ( dt_            IS NULL OR fm.dt            = dt_            )
	                AND ( dt_start_      IS NULL OR fm.dt_start      = dt_start_      )
	                AND ( dt_end_        IS NULL OR fm.dt_end        = dt_end_        )
	                AND ( forecast_hour_ IS NULL OR fm.forecast_hour = forecast_hour_ )            
	                AND ( from_dt_       IS NULL OR fm.dt_start     >= from_dt_       )
	                AND ( to_dt_         IS NULL OR fm.dt_start     <= to_dt_         )
	            ORDER BY fm.dt_start;
    END IF;
END;$$;


ALTER FUNCTION meteo.get_manual_forecast_result(id_ bigint, stations_ text[], stations_types_ bigint[], region_id_ bigint[], user_id_ text, dt_ timestamp without time zone, dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, forecast_hour_ bigint, from_dt_ timestamp without time zone, to_dt_ timestamp without time zone, only_last_ boolean) OWNER TO postgres;

--
-- Name: get_many_ocean_data(timestamp without time zone, real, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_many_ocean_data(dt_ timestamp without time zone, level_ real, level_type_ integer, descrname_ text) RETURNS SETOF meteo.many_ocean_data
    LANGUAGE plpgsql
    AS $$DECLARE
 ref_ refcursor; 
 mviews meteo.many_ocean_data;
BEGIN
SELECT * FROM meteo.get_many_ocean_data_curs(dt_, level_ , level_type_, descrname_) into ref_;

LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;$$;


ALTER FUNCTION meteo.get_many_ocean_data(dt_ timestamp without time zone, level_ real, level_type_ integer, descrname_ text) OWNER TO postgres;

--
-- Name: get_many_ocean_data_curs(timestamp without time zone, real, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_many_ocean_data_curs(dt_ timestamp without time zone, level_ real, level_type_ integer, descrname_ text) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor := '_result';

BEGIN

	OPEN ref FOR select 
        ST_AsGeoJson(location)::jsonb as coords,
        station,
        jsonb_agg(d) as param,
		id   
	 from (SELECT *, jsonb_array_elements(param) as d
		   FROM meteo.report 
	where  dt = dt_
		   and (level_ IS NULL or level = level_)  
		   and level_type = level_type_
		  ) as dd where d::jsonb->'descrname' ? descrname_
		    group by id, station, location;
    return ref;

END;$$;


ALTER FUNCTION meteo.get_many_ocean_data_curs(dt_ timestamp without time zone, level_ real, level_type_ integer, descrname_ text) OWNER TO postgres;

--
-- Name: get_meteo_centers(integer[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_centers(center_ integer[], subcenter_ integer[]) RETURNS TABLE(center integer, name text, short_name text, priority integer, subcenters jsonb)
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
RETURN QUERY 
SELECT 	meteo.centers.center,
	meteo.centers.full_name,
	meteo.centers.short_name,
	meteo.centers.priority,
	meteo.centers.subcenters
		FROM meteo.centers WHERE (center_ IS NULL OR meteo.centers.center = ANY (center_))
		AND (subcenter_ IS NULL OR EXISTS(SELECT 1 FROM jsonb_to_recordset(meteo.centers.subcenters) AS X(id integer, "name" text) WHERE x.id = ANY(subcenter_)));
RETURN;
END
$$;


ALTER FUNCTION meteo.get_meteo_centers(center_ integer[], subcenter_ integer[]) OWNER TO postgres;

--
-- Name: get_meteo_data(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
 ref_ refcursor; 
 mviews RECORD;
BEGIN
SELECT * FROM meteo.get_meteo_data_curs(dt_start_ ,dt_end_ ,level_,
  lev_type_,data_type_,station_,stat_type_,descrname_,coord_,max_distace_) into ref_;

LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;

END;$$;


ALTER FUNCTION meteo.get_meteo_data(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) OWNER TO postgres;

--
-- Name: get_meteo_data_all_descr_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_all_descr_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[]) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor := '_result';
BEGIN

OPEN ref FOR select 
    meteo.report.id, 
 	meteo.report.dt,
	meteo.report.station,
	meteo.report.station_type,
	meteo.stations.name AS station_info,
	meteo.report.data_type,
	meteo.report.level,
	meteo.report.level_type,
	meteo.report.param,
	ST_AsGeoJson(meteo.report.location)::jsonb||jsonb_build_object('alt',alt) as location,
    0.0::double precision as dist
    FROM meteo.report left join meteo.stations using (station,station_type)
where  dt BETWEEN dt_start_ AND dt_end_
  	   and (level_ IS NULL or level =level_)  
	   and level_type = lev_type_
	   and (station_ IS NULL or station = ANY (station_))  
	   and (stat_type_ IS NULL or station_type = ANY (stat_type_))  
	   and (data_type_ IS NULL or data_type = ANY (data_type_))  
	   ORDER BY dt;
return ref; 

END;$$;


ALTER FUNCTION meteo.get_meteo_data_all_descr_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[]) OWNER TO postgres;

--
-- Name: get_meteo_data_by_coords(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_by_coords(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
 ref_ refcursor; 
 mviews RECORD;
BEGIN
SELECT * FROM meteo.get_meteo_data_by_coords_curs(dt_start_ ,dt_end_ ,level_,
  lev_type_,data_type_,station_,stat_type_,descrname_,coord_,max_distace_) into ref_;

LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
CLOSE ref_;
END;
$$;


ALTER FUNCTION meteo.get_meteo_data_by_coords(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) OWNER TO postgres;

--
-- Name: get_meteo_data_by_coords_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_by_coords_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
 ref refcursor := '_result';
 circle_ geometry;
 point_center_ geometry;
 flag_search int := 0;
BEGIN
--select  from meteo.report limit 1
 IF max_distace_ IS NOT NULL AND ST_GeometryType(coord_) = 'ST_Point' THEN
	flag_search := 1;
	point_center_ = coord_;
 END IF;
 IF max_distace_ IS NULL AND ST_GeometryType(coord_) = 'ST_Polygon' THEN
    circle_ := coord_;
	flag_search := 2;
	point_center_ := null;
 END IF;

OPEN ref FOR select
    id,
        dt,
        station,
        station_type,
        station_info,
        data_type,
        level,
        level_type,
        jsonb_agg(d) as param,
        ST_AsGeoJson(location)::jsonb||jsonb_build_object('alt', alt) as location
        ,ST_DistanceSphere(point_center_,location) as dist
 from (SELECT meteo.report.*, jsonb_array_elements(meteo.report.param) as d, meteo.stations.alt,
           meteo.stations.name AS station_info
           FROM meteo.report left join meteo.stations using (station, station_type)
where  dt BETWEEN dt_start_ AND dt_end_
           and (level_ IS NULL or level =level_)
           and level_type = lev_type_
           and (station_ IS NULL or station = ANY (station_))
           and (stat_type_ IS NULL or station_type = ANY (stat_type_))
           and (data_type_ IS NULL or data_type = ANY (data_type_))
	       and (flag_search <>2 or ST_Within(meteo.report.location, circle_) )
           and (flag_search <>1 or max_distace_ IS NULL or ST_DistanceSphere(coord_,meteo.report.location) <= max_distace_)
          ) as dd where (descrname_ is null or d::jsonb->'descrname' ?| descrname_)
           group by id, dt, station,alt, station_info, station_type, data_type, level, level_type,location,dist ORDER BY dt, dist;

    RAISE notice ' end';
     --coord_ geometry,
        --max_distace_ integer)
return ref;

END;
$$;


ALTER FUNCTION meteo.get_meteo_data_by_coords_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) OWNER TO postgres;

--
-- Name: get_meteo_data_by_day_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_by_day_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[]) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor := '_result';
 ref1 refcursor;
 mviews RECORD;
 mviews1 RECORD;
BEGIN
	OPEN ref FOR select 
		station, station_type, data_type, level, level_type, 
		min((d->>'value')::real) as  param
   from (SELECT *, jsonb_array_elements(param) as d
		   FROM meteo.report 
	where  dt BETWEEN dt_start_ AND dt_end_
		   and (level_ IS NULL or level =level_)  
		   and level_type = lev_type_
		   and (station_ IS NULL or station = ANY (station_))  
		   and (stat_type_ IS NULL or station_type = ANY (stat_type_))  
		   and (data_type_ IS NULL or data_type = ANY (data_type_))  
		  ) as dd where d::jsonb->'descrname' ?| descrname_
		   group by station, station_type, data_type, level, level_type
		   ORDER BY station;
    return ref;
 
END;$$;


ALTER FUNCTION meteo.get_meteo_data_by_day_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[]) OWNER TO postgres;

--
-- Name: get_meteo_data_count(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_count(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], descrname_ text[]) RETURNS SETOF meteo.meteodata_count
    LANGUAGE plpgsql
    AS $$
DECLARE

	count_total_ numeric;
	station_types_ smallint[];
	adata_type_ integer;
BEGIN
    FOREACH adata_type_ IN ARRAY data_type_
	loop
	station_types_ = array_append(station_types_, (select type from meteo.station_types where adata_type_ between min_data_type and  max_data_type));
	end loop;
	SELECT count(distinct(station)) FROM meteo.stations
	where station_type = ANY (station_types_) and operational = true into count_total_;
	IF (0 = count_total_ ) THEN
     count_total_ = 1;
    END IF;
if(level_ IS NULL )then
return query select dt, count(id) ,round(count(id)/count_total_ ,2),-1.0::real from meteo.report where 
       dt BETWEEN dt_start_ AND dt_end_
	   and level_type = lev_type_
	   and (data_type_ IS NULL or data_type = ANY (data_type_))  
       and (descrname_ && (meteo.json2arr(param,'descrname' ))) 
	   group by dt, level_type;

ELSE

return query select dt, count(id) ,round(count(id)/count_total_ ,2), level from meteo.report where 
       dt BETWEEN dt_start_ AND dt_end_
  	   and level =level_
	   and level_type = lev_type_
	   and (data_type_ IS NULL or data_type = ANY (data_type_))  
       and (descrname_ && (meteo.json2arr(param,'descrname' ))) 
	   group by dt, level;
END IF;
	
 
END;
$$;


ALTER FUNCTION meteo.get_meteo_data_count(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], descrname_ text[]) OWNER TO postgres;

--
-- Name: get_meteo_data_count_max_min(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_count_max_min(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], descrname_ text[]) RETURNS SETOF meteo.meteodata_max_min_count
    LANGUAGE plpgsql
    AS $$DECLARE

	count_total_ numeric;
	station_types_ smallint[];
	adata_type_ integer;
BEGIN
    FOREACH adata_type_ IN ARRAY data_type_
	loop
	station_types_ = array_append(station_types_, (select type from meteo.station_types where adata_type_ between min_data_type and  max_data_type));
	end loop;
	SELECT count(distinct(station)) FROM meteo.stations
	where station_type = ANY (station_types_) and operational = true into count_total_;
	IF (0 = count_total_ ) THEN
     count_total_ = 1;
    END IF;
  --  RAISE NOTICE ' % stype= % dtype= %',count_total_,station_types_,data_type_;

return query select date(dt) as dateu, count(distinct(station)) ,round(count(distinct(station))/count_total_ ,2) from meteo.report where 
       dt BETWEEN dt_start_ AND dt_end_
  	   and (level_ IS NULL or level =level_)  
  	   and (lev_type_ IS NULL or level_type =lev_type_)  
	   and (data_type_ IS NULL or data_type = ANY (data_type_))  
       and (descrname_ && (meteo.json2arr(param,'descrname' ))) 
	   group by dateu;
 
END;$$;


ALTER FUNCTION meteo.get_meteo_data_count_max_min(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], descrname_ text[]) OWNER TO postgres;

--
-- Name: get_meteo_data_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor := '_result';
BEGIN
  CASE 
      WHEN descrname_ is null THEN
            select * from meteo.get_meteo_data_all_descr_curs(dt_start_ ,dt_end_ ,level_,
						                					  lev_type_,data_type_,station_,stat_type_) into ref;
		    return ref;
													
      WHEN coord_ is not null THEN
			select * from meteo.get_meteo_data_by_coords_curs(dt_start_ ,dt_end_ ,level_,
							                					  lev_type_,data_type_,station_,stat_type_,
																	  descrname_,coord_,max_distace_) into ref;
			return ref;
	ELSE
	OPEN ref FOR select 
		id, dt,	station, station_type, station_info, data_type, level, level_type, 
		  jsonb_agg(d) as param,
		ST_AsGeoJson(location)::jsonb||jsonb_build_object('alt',alt) as location
	   ,0.0::double precision as dist
 from (SELECT meteo.report.*, jsonb_array_elements(meteo.report.param) as d, meteo.stations.alt,
	   meteo.stations.name AS station_info
	   FROM meteo.report left join meteo.stations using (station, station_type)

--		from (SELECT *, jsonb_array_elements(param) as d
--		   FROM meteo.report 
	where  dt BETWEEN dt_start_ AND dt_end_
		   and (level_ IS NULL or level =level_)  
		   and level_type = lev_type_
		   and (station_ IS NULL or station = ANY (station_))  
		   and (stat_type_ IS NULL or station_type = ANY (stat_type_))  
		   and (data_type_ IS NULL or data_type = ANY (data_type_))  
		  ) as dd where d::jsonb->'descrname' ?| descrname_
		    group by id, dt,	station,alt, station_info, station_type, data_type, level, level_type,location,dist ORDER BY dt;
    return ref;
	--coord_ geometry,
	--max_distace_ integer)
END CASE;
 
END;$$;


ALTER FUNCTION meteo.get_meteo_data_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, max_distace_ integer) OWNER TO postgres;

--
-- Name: get_meteo_data_nearest_time(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_nearest_time(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, term_ timestamp without time zone) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor;
 mviews RECORD;
BEGIN
SELECT * FROM meteo.get_meteo_data_nearest_time_curs(dt_start_ ,dt_end_ ,level_,
  lev_type_,data_type_,station_,stat_type_,descrname_,coord_,term_) into ref;

LOOP
	FETCH NEXT FROM ref INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
 
END;$$;


ALTER FUNCTION meteo.get_meteo_data_nearest_time(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, term_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_meteo_data_nearest_time_curs(timestamp without time zone, timestamp without time zone, real, integer, integer[], text[], integer[], text[], public.geometry, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_meteo_data_nearest_time_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, term_ timestamp without time zone) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
 ref refcursor := '_result';
BEGIN
OPEN ref FOR select DISTINCT ON (station)
                id,
                dt,
                station,
                station_type,
                station_info,
                data_type,
                level,
                level_type,
                jsonb_agg(d) as param,
                ST_AsGeoJson(location)::jsonb||jsonb_build_object('alt', alt) as location
           ,0.0::double precision as dist
         from (SELECT meteo.report.*, jsonb_array_elements(meteo.report.param) as d, meteo.stations.alt ,
           meteo.stations.name AS station_info
                   FROM meteo.report left join meteo.stations using (station, station_type)
        where  dt BETWEEN dt_start_ AND dt_end_
                   and (level_ IS NULL or level =level_)
                   and level_type = lev_type_
                   and (station_ IS NULL or station = ANY (station_))
                   and (stat_type_ IS NULL or station_type = ANY (stat_type_))
                   and (data_type_ IS NULL or data_type = ANY (data_type_))
                   and (coord_ IS NULL or ST_Contains(coord_,meteo.stations.location))
                  ) as dd where d::jsonb->'descrname' ?| descrname_
                    group by id, dt, station,station_info,alt,  station_type, data_type, level, level_type,location,dist ORDER BY station, abs(EXTRACT(hours from(dt-term_))) ;
    return ref;

END;$$;


ALTER FUNCTION meteo.get_meteo_data_nearest_time_curs(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, level_ real, lev_type_ integer, data_type_ integer[], station_ text[], stat_type_ integer[], descrname_ text[], coord_ public.geometry, term_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_military_district(integer[], text[], text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_military_district(id_ integer[], title_ text[], query_ text) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_military_district_curs(id_, title_, query_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_military_district(id_ integer[], title_ text[], query_ text) OWNER TO postgres;

--
-- Name: get_military_district_curs(integer[], text[], text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_military_district_curs(id_ integer[], title_ text[], query_ text) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref refcursor := 'rescursor';
  like_str_ TEXT = CONCAT('%', query_, '%');
BEGIN
  OPEN ref FOR SELECT a.id,
                      a.title as name,
					  jsonb_set(ST_AsGeoJson(a.region_center_point)::jsonb,'{alt}', to_jsonb(0)) as center,
					    jsonb_agg(ST_AsGeoJSON(a.region_coords)::jsonb) as region_coords,
					  a.descr
                      FROM (select id, title, region_center_point, (ST_DUMP(region_coords)).geom as region_coords, descr from  meteo.forecast_region 
                  WHERE is_okrug =True
                    and  (id_   IS NULL OR id = ANY(id_) )
				  	AND  (title_ IS NULL OR title = ANY(title_) )
					AND  (query_ IS NULL OR title ILIKE like_str_ )) as a group by a.id, a.title, center,a.descr  ;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_military_district_curs(id_ integer[], title_ text[], query_ text) OWNER TO postgres;

--
-- Name: get_near_date(timestamp without time zone, timestamp without time zone, integer[], text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_near_date(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, data_type_ integer[], station_ text) RETURNS SETOF meteo.aero_date_list
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
return query select distinct (dt) 
from  meteo.report where 
       	  dt BETWEEN dt_start_ AND dt_end_
		   and (data_type_ IS NULL or data_type = ANY (data_type_))
		   and station  = station_ 
     	   order by dt desc limit 1; 
END;$$;


ALTER FUNCTION meteo.get_near_date(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, data_type_ integer[], station_ text) OWNER TO postgres;

--
-- Name: get_ocean_by_station(timestamp without time zone, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_ocean_by_station(dt_ timestamp without time zone, station_ text) RETURNS SETOF meteo.ocean_by_station_data
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
return query 
select 
	station,  ST_AsGeoJson(location)::jsonb as coord, level,
	jsonb_agg(d) as param
 from (SELECT *, jsonb_array_elements(param) as d
	   FROM meteo.report where 
	              dt = dt_
  	   		  and station_type = 6
	          and station = station_
	          --FIXME  and level_type <> 256
	   		  ) as dd /*where d::jsonb->'descrname' ?| '{ "Sn", "Tw"}'*/
       group by station, coord, level
       order by station, coord, level;
	   
END;$$;


ALTER FUNCTION meteo.get_ocean_by_station(dt_ timestamp without time zone, station_ text) OWNER TO postgres;

--
-- Name: get_one_field(timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer, integer, integer, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_one_field(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer, level_type_ integer, center_ integer, hour_ integer, model_ integer, descrnum_ integer) RETURNS meteo.field_data_descr
    LANGUAGE plpgsql
    AS $$
DECLARE
result_ meteo.field_data_descr;
BEGIN
SELECT o.id, o.dt, o.descr, o.hour, o.model, o.level, o.level_type, o.center, o.net_type,
       o.forecast_start, o.forecast_end, o.fs_id, o.count_points
	   , cn.full_name, cn.short_name 
	   , lt.full_descr, cn.priority
	   from (SELECT * FROM meteo.obanal WHERE  
	       (dt_start_ IS NULL or dt_end_ is NULL or  (dt BETWEEN dt_start_ AND dt_end_))
	       and  (fdt_start_ IS NULL or fdt_end_ is NULL or (forecast_start BETWEEN fdt_start_ AND fdt_end_))
		   and (level_ IS NULL or level = level_)  
		   and (level_type_ IS NULL or level_type = level_type_)  
		   and (center_ IS NULL or center = center_)  
		   and (hour_ IS NULL or hour = hour_) 
		   and (model_ IS NULL or model = model_)  
		   and (descrnum_ IS NULL or descr = descrnum_)  
		   ) as o 
		   LEFT OUTER JOIN meteo.centers cn on o.center = cn.center
		   LEFT OUTER JOIN meteo.level_types lt on o.level_type = lt.type
           ORDER BY o.forecast_start DESC, o.hour limit 1 into result_;
		   --GROUP BY (cn.center, cn.full_name, cn.short_name, cn.priority )
RETURN result_;
END;
$$;


ALTER FUNCTION meteo.get_one_field(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, fdt_start_ timestamp without time zone, fdt_end_ timestamp without time zone, level_ integer, level_type_ integer, center_ integer, hour_ integer, model_ integer, descrnum_ integer) OWNER TO postgres;

--
-- Name: get_one_zond_data(timestamp without time zone, integer[], text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_one_zond_data(dt_ timestamp without time zone, data_type_ integer[], station_ text) RETURNS SETOF meteo.one_zond_data
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
 
return query 
select level,level_type,
ST_AsGeoJson(location)::jsonb as location,
jsonb_array_elements(jsonb_agg(param)) 
from  meteo.report where 
       station = station_ 
	   and dt = dt_ 
	   and (data_type_ IS NULL or data_type = ANY (data_type_))
 -- FIXME данные по ветру попадают в зонд со странным типом уровня и ломают зонд
	   and level_type != 112
	   group by location,level,level_type;

END;
$$;


ALTER FUNCTION meteo.get_one_zond_data(dt_ timestamp without time zone, data_type_ integer[], station_ text) OWNER TO postgres;

--
-- Name: get_parameter_by_parameter(text, integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_parameter_by_parameter(paramname text, values_ integer[]) RETURNS TABLE(_id integer, name text, unit text, grib1_parameter integer, bufr_descr integer, discipline integer, category integer, unit_en text)
    LANGUAGE plpgsql
    AS $$DECLARE
query_ text;
parampam text;
BEGIN
IF paramname = 'grib1_parameter' THEN
    paramname := 'grib1';
ELSIF paramname = 'parameter' THEN
    paramname := 'grib2';
ELSIF paramname = 'discipline' THEN
    paramname := 'grb2discipline';
ELSIF paramname = 'category' THEN
    paramname := 'grb2category';
ELSIF paramname = 'name' THEN
    paramname := 'fullname';
END IF;
		--parameters.id,--если подключение к MDB, то запрашивается _id, который назначает сама БД
query_ = 'SELECT 
		parameters.id,
		parameters.fullname,
		parameters.unit,
		parameters.grib1,
		parameters.bufr_descr,
		parameters.grb2discipline,
		parameters.grb2category,
		parameters.unit_en
	FROM 	meteo.parameters AS parameters ';
IF values_ IS NOT NULL  AND array_length(values_, 1) > 0 THEN
	query_ := query_ || 'WHERE parameters.' || paramname || ' = ANY (''{' || array_to_string(values_, ',', 'null') || '}'');';
ELSE
	query_ := query_ || ';';
END IF;
RETURN QUERY EXECUTE query_;
END
$$;


ALTER FUNCTION meteo.get_parameter_by_parameter(paramname text, values_ integer[]) OWNER TO postgres;

--
-- Name: get_regions(integer[], text[], integer, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_regions(id_ integer[], title_ text[], skip_ integer, limit_ integer, query_ text) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_regions_curs(id_, title_, skip_, limit_, query_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_regions(id_ integer[], title_ text[], skip_ integer, limit_ integer, query_ text) OWNER TO postgres;

--
-- Name: get_regions_curs(integer[], text[], integer, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_regions_curs(id_ integer[], title_ text[], skip_ integer, limit_ integer, query_ text) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref refcursor := 'rescursor';
  like_str_ TEXT = CONCAT('%', query_, '%');
BEGIN
  OPEN ref FOR SELECT id,
                      title,
					  jsonb_set(ST_AsGeoJson(region_center_point)::jsonb,'{alt}', to_jsonb(0)) as region_center_point,
					  ST_AsGeoJSON(region_coords)::jsonb as region_coords
                      FROM  meteo.forecast_region 
                  WHERE  (id_   IS NULL OR id = ANY(id_) )
				  	AND (title_ IS NULL OR title = ANY(title_)      )
					AND (query_ IS NULL OR title ILIKE like_str_   )
					LIMIT limit_
					OFFSET skip_;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_regions_curs(id_ integer[], title_ text[], skip_ integer, limit_ integer, query_ text) OWNER TO postgres;

--
-- Name: get_station_by_parameter(text, integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_by_parameter(paramname text, values_ integer[]) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
SELECT * FROM meteo.get_station_by_parameter_curs(paramname, values_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
RETURN;
END;
$$;


ALTER FUNCTION meteo.get_station_by_parameter(paramname text, values_ integer[]) OWNER TO postgres;

--
-- Name: get_station_by_parameter_curs(text, integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_by_parameter_curs(paramname text, values_ integer[]) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
ref refcursor := 'rescursor';
main_query text;
array_to_text text;
where_text text;
BEGIN
IF values_ IS NOT NULL AND array_length(values_, 1) > 0 THEN
	array_to_text := '''{' || array_to_string(values_, ',', 'null') || '}''';
	where_text := format('WHERE (stations.' || quote_ident(paramname) || '= ANY('|| array_to_text ||') ) )');
ELSE
	where_text := ')';
END IF;
main_query := format('SELECT id as _id,
	station,
	st_index as index,
	st_type as station_type,
	fullname as name,
	jsonb_set(ST_AsGeoJson(coord)::jsonb,' || quote_literal('{alt}') || ', to_jsonb(alt)) as coord,
	country,
	water_depth as hydro_levels,
	countryinfo
	FROM ( SELECT * FROM meteo.stations ' || where_text ||
	' AS t1 INNER JOIN ( SELECT jsonb_build_object(	' || quote_literal('_id') || ', id, ' || quote_literal('number') || ', world_number,' ||
							quote_literal('name_en') || ', name_en, ' || quote_literal('name') || ', name_common,' ||
							quote_literal('name_ru') || ', name_ru, ' || quote_literal('wmocode') || ', wmocode,' ||
							quote_literal('wmonumber') || ', wmonumber, ' || quote_literal('priority') || ', priority,' ||
							quote_literal('code') || ', code) AS countryinfo FROM meteo.countries  ) AS b
                                                                             ON t1.country = (b.countryinfo::jsonb->>''number'')::integer ;');

OPEN ref FOR EXECUTE main_query;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_station_by_parameter_curs(paramname text, values_ integer[]) OWNER TO postgres;

--
-- Name: get_station_by_position(double precision[], double precision, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_by_position(coordinates_ double precision[], maxdistance_ double precision, station_type_ integer DEFAULT NULL::integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_station_by_position_curs(coordinates_, maxdistance_, station_type_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_station_by_position(coordinates_ double precision[], maxdistance_ double precision, station_type_ integer) OWNER TO postgres;

--
-- Name: get_station_by_position_curs(double precision[], double precision, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_by_position_curs(coordinates_ double precision[], maxdistance_ double precision, station_type_ integer DEFAULT NULL::integer) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
center geometry := ST_MakePoint(coordinates_[1], coordinates_[2]);
circle geometry := ST_Buffer(center::geography, maxdistance_)::geometry;
ref refcursor := 'rescursor';
BEGIN
  OPEN ref FOR SELECT jsonb_build_object('calculated', to_jsonb(ST_DistanceSphere(t1.location, center)::integer)) as dist,
		      id as _id,
                      station,
                      index,
                      station_type,
                      name,
                      jsonb_set(ST_AsGeoJson(location)::jsonb,'{alt}', to_jsonb(alt)) as location,
                      country,
                      hydro_levels,
                      countryinfo,
                      airstrip_direction
                      FROM ( SELECT * FROM meteo.stations WHERE (station_type_ IS NULL OR meteo.stations.station_type = station_type_) 
							AND ST_Within(meteo.stations.location, circle) )
                                                           AS t1 INNER JOIN ( SELECT jsonb_build_object('_id', id, 'number', world_number,
                                                                                                        'name_en', name_en, 'name', name_common,
                                                                                                        'name_ru', name_ru, 'wmocode', wmocode,
                                                                                                        'wmonumber', wmonumber, 'priority', priority,
                                                                                                        'code', code, 'icao', icao) AS
																			 countryinfo FROM meteo.countries  ) AS b
                                                                             ON t1.country = (b.countryinfo::jsonb->>'number')::integer ;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_station_by_position_curs(coordinates_ double precision[], maxdistance_ double precision, station_type_ integer) OWNER TO postgres;

--
-- Name: get_station_in_polygon(public.geometry); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_in_polygon(polygon_ public.geometry) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_station_in_polygon_curs(polygon_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_station_in_polygon(polygon_ public.geometry) OWNER TO postgres;

--
-- Name: get_station_in_polygon_curs(public.geometry); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_in_polygon_curs(polygon_ public.geometry) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref refcursor := 'rescursor';
BEGIN
  OPEN ref FOR SELECT id as _id,
                      station,
                      st_index as index,
                      st_type as station_type,
                      fullname as name,
                      jsonb_set(ST_AsGeoJson(coord)::jsonb,'{alt}', to_jsonb(alt)) as coord,
                      country,
                      water_depth as hydro_levels,
                      countryinfo
                      FROM ( SELECT * FROM meteo.stations WHERE ST_IsValid(meteo.stations.coord) AND ST_IsValid(polygon_) AND ST_Within(meteo.stations.coord, polygon_) )
                                                           AS t1 INNER JOIN ( SELECT jsonb_build_object('_id', id, 'number', world_number,
                                                                                                        'name_en', name_en, 'name', name_common,
                                                                                                        'name_ru', name_ru, 'wmocode', wmocode,
                                                                                                        'wmonumber', wmonumber, 'priority', priority,
                                                                                                        'code', code) AS countryinfo FROM meteo.countries  ) AS b
                                                                             ON t1.country = (b.countryinfo::jsonb->>'number')::integer ;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_station_in_polygon_curs(polygon_ public.geometry) OWNER TO postgres;

--
-- Name: get_station_type(integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_type(station_type_ integer[] DEFAULT NULL::integer[]) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_station_type_curs(station_type_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_station_type(station_type_ integer[]) OWNER TO postgres;

--
-- Name: FUNCTION get_station_type(station_type_ integer[]); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.get_station_type(station_type_ integer[]) IS 'Получить тип(ы) станции';


--
-- Name: get_station_type_curs(integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_station_type_curs(station_type_ integer[] DEFAULT NULL::integer[]) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$DECLARE
  ref refcursor := 'rescursor';
BEGIN
  OPEN ref 	FOR SELECT 	meteo.station_types.type,
				meteo.station_types.description,
				meteo.station_types.max_data_type,
				meteo.station_types.min_data_type
		FROM meteo.station_types WHERE station_type_ IS NULL OR type = ANY(station_type_);
		--FROM meteo.station_types WHERE station_type_ IS NULL OR array_length(station_type_, 1) > 0 OR type = ANY(station_type_);
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_station_type_curs(station_type_ integer[]) OWNER TO postgres;

--
-- Name: get_stations(integer[], text[], integer[], integer[], text[], text[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_stations_curs(id_, station_, station_type_, station_index_,
                                       station_cccc_, name_, country_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]) OWNER TO postgres;

--
-- Name: FUNCTION get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]) IS 'Получить станции';


--
-- Name: get_stations(integer[], text[], integer[], integer[], text[], text[], integer[], public.geometry); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], region_ public.geometry) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_stations_curs(id_, station_, station_type_, station_index_,
                                       station_cccc_, name_, country_,region_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], region_ public.geometry) OWNER TO postgres;

--
-- Name: get_stations(integer[], text[], integer[], integer[], text[], text[], integer[], integer, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_stations_curs(id_, station_, station_type_, station_index_,
                                       station_cccc_, name_, country_, skip_, limit_, query_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text) OWNER TO postgres;

--
-- Name: FUNCTION get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text) IS 'Получить станции';


--
-- Name: get_stations(integer[], text[], integer[], integer[], text[], text[], integer[], integer, integer, text, public.geometry); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text, region_ public.geometry) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.get_stations_curs(id_, station_, station_type_, station_index_,
                                       station_cccc_, name_, country_, skip_, limit_, query_,region_) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.get_stations(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text, region_ public.geometry) OWNER TO postgres;

--
-- Name: get_stations_curs(integer[], text[], integer[], integer[], text[], text[], integer[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref refcursor := 'rescursor';
BEGIN
  OPEN ref FOR SELECT id as _id,
                      station,
                      index,
                      station_type,
                      name,
                      jsonb_set(ST_AsGeoJson(location)::jsonb,'{alt}', to_jsonb(alt)) as location,
                      country,
                      hydro_levels,
                      countryinfo,
                      airstrip_direction
                      FROM  meteo.stations INNER JOIN ( SELECT jsonb_build_object('_id', id, 'number', world_number,
                                                                                                        'name_en', name_en, 'name', name_common,
                                                                                                        'name_ru', name_ru, 'wmocode', wmocode,
                                                                                                        'wmonumber', wmonumber, 'priority', priority,
                                                                                                        'code', code, 'icao', icao) AS countryinfo FROM meteo.countries  ) AS b
                                                                                                        ON country = (b.countryinfo::jsonb->>'number')::integer 
							                                  WHERE (id_ IS NULL OR id = ANY(id_))
                                                              AND (station_ IS NULL OR station = ANY(station_))
                                                              AND (station_type_ IS NULL OR station_type = ANY(station_type_))
                                                              AND (station_index_ IS NULL OR  ( (index::jsonb->>'station_index')::integer = ANY(station_index_) ))
                                                              AND (station_cccc_ IS NULL OR index::jsonb->'cccc' ?| station_cccc_
                                                                                     OR index::jsonb->'station_name' ?| station_cccc_)
                                                              AND (country_ IS NULL OR country = ANY(country_))
                                                              AND ( (name_ IS NULL)
                                                                 OR ( (name::jsonb->'international' ?| name_)
                                                                   OR (name::jsonb->'ru' ?| name_)
                                                                   OR (name::jsonb->'en' ?| name_)));
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]) OWNER TO postgres;

--
-- Name: FUNCTION get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[]) IS 'Получить станции';


--
-- Name: get_stations_curs(integer[], text[], integer[], integer[], text[], text[], integer[], public.geometry); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], region_ public.geometry) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref refcursor := 'rescursor';
BEGIN
  OPEN ref FOR SELECT id as _id,
                      station,
                      index,
                      station_type,
                      name,
                      jsonb_set(ST_AsGeoJson(location)::jsonb,'{alt}', to_jsonb(alt)) as location,
                      country,
                      hydro_levels,
                      countryinfo,
                      airstrip_direction
                      FROM  meteo.stations INNER JOIN ( SELECT jsonb_build_object('_id', id, 'number', world_number,
                                                                                                        'name_en', name_en, 'name', name_common,
                                                                                                        'name_ru', name_ru, 'wmocode', wmocode,
                                                                                                        'wmonumber', wmonumber, 'priority', priority,
                                                                                                        'code', code, 'icao', icao) AS countryinfo FROM meteo.countries  ) AS b
                                                                                                        ON country = (b.countryinfo::jsonb->>'number')::integer 
							                                  WHERE (id_ IS NULL OR id = ANY(id_))
                                                              AND (station_ IS NULL OR station = ANY(station_))
                                                              AND (station_type_ IS NULL OR station_type = ANY(station_type_))
                                                              AND (station_index_ IS NULL OR  ( (index::jsonb->>'station_index')::integer = ANY(station_index_) ))
                                                              AND (station_cccc_ IS NULL OR index::jsonb->'cccc' ?| station_cccc_
                                                                                     OR index::jsonb->'station_name' ?| station_cccc_)
                                                              AND (country_ IS NULL OR country = ANY(country_))
                                                              AND ( (name_ IS NULL)
                                                                 OR ( (name::jsonb->'international' ?| name_)
                                                                   OR (name::jsonb->'ru' ?| name_)
                                                                   OR (name::jsonb->'en' ?| name_)))
															and (region_ IS NULL OR ST_Within(meteo.stations.location, region_))  
															;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], region_ public.geometry) OWNER TO postgres;

--
-- Name: get_stations_curs(integer[], text[], integer[], integer[], text[], text[], integer[], integer, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref refcursor := 'rescursor';
  like_str_ TEXT = CONCAT('%', query_, '%');
BEGIN
  OPEN ref FOR SELECT id as _id,
                      station,
                      index,
                      station_type,
                      name,
                      jsonb_set(ST_AsGeoJson(location)::jsonb,'{alt}', to_jsonb(alt)) as location,
                      country,
                      hydro_levels,
                      countryinfo,
                      airstrip_direction
                      FROM  meteo.stations INNER JOIN ( SELECT jsonb_build_object('_id', id, 'number', world_number,
                                                                                  'name_en', name_en, 'name', name_common,
                                                                                  'name_ru', name_ru, 'wmocode', wmocode,
                                                                                  'wmonumber', wmonumber, 'priority', priority,
                                                                                  'code', code, 'icao', icao) AS countryinfo FROM meteo.countries  ) AS b
                                                                                  ON country = (b.countryinfo::jsonb->>'number')::integer 
                  WHERE (id_ IS NULL OR id = ANY(id_))
                                AND (station_ IS NULL OR station = ANY(station_))
                                AND (station_type_ IS NULL OR station_type = ANY(station_type_))
                                AND (station_index_ IS NULL OR  ( (index::jsonb->>'station_index')::integer = ANY(station_index_) ))
                                AND (station_cccc_ IS NULL OR index::jsonb->'cccc' ?| station_cccc_
                                                        OR index::jsonb->'station_name' ?| station_cccc_)
                                AND (country_ IS NULL OR country = ANY(country_))
                                AND ( (name_ IS NULL)
                                    OR ( (name::jsonb->'international' ?| name_)
                                      OR (name::jsonb->'ru' ?| name_)
                                      OR (name::jsonb->'en' ?| name_)))
															  AND ( (query_ IS NULL) 
																    OR  ( name::jsonb->>'ru'             ILIKE like_str_ )
																   OR  ( name::jsonb->>'en'             ILIKE like_str_ )
																   OR  ( name::jsonb->>'international'  ILIKE like_str_ )
																   OR  ( index::jsonb->>'cccc'          ILIKE like_str_ )
																   OR  ( index::jsonb->>'station_name'  ILIKE like_str_ )
																   OR  ( index::jsonb->>'station_index' ILIKE like_str_ )
																   OR  station ILIKE like_str_
                                )
															  LIMIT limit_
															  OFFSET skip_;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text) OWNER TO postgres;

--
-- Name: FUNCTION get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text) IS 'Получить станции';


--
-- Name: get_stations_curs(integer[], text[], integer[], integer[], text[], text[], integer[], integer, integer, text, public.geometry); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text, region_ public.geometry) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
DECLARE
  ref refcursor := 'rescursor';
  like_str_ TEXT = CONCAT('%', query_, '%');
BEGIN
  OPEN ref FOR SELECT id as _id,
                      station,
                      index,
                      station_type,
                      name,
                      jsonb_set(ST_AsGeoJson(location)::jsonb,'{alt}', to_jsonb(alt)) as location,
                      country,
                      hydro_levels,
                      countryinfo,
                      airstrip_direction
                      FROM  meteo.stations INNER JOIN ( SELECT jsonb_build_object('_id', id, 'number', world_number,
                                                                                  'name_en', name_en, 'name', name_common,
                                                                                  'name_ru', name_ru, 'wmocode', wmocode,
                                                                                  'wmonumber', wmonumber, 'priority', priority,
                                                                                  'code', code, 'icao', icao) AS countryinfo FROM meteo.countries  ) AS b
                                                                                  ON country = (b.countryinfo::jsonb->>'number')::integer 
                  WHERE (id_ IS NULL OR id = ANY(id_))
                                AND (station_ IS NULL OR station = ANY(station_))
                                AND (station_type_ IS NULL OR station_type = ANY(station_type_))
                                AND (station_index_ IS NULL OR  ( (index::jsonb->>'station_index')::integer = ANY(station_index_) ))
                                AND (station_cccc_ IS NULL OR index::jsonb->'cccc' ?| station_cccc_
                                                        OR index::jsonb->'station_name' ?| station_cccc_)
                                AND (country_ IS NULL OR country = ANY(country_))
                                AND ( (name_ IS NULL)
                                    OR ( (name::jsonb->'international' ?| name_)
                                      OR (name::jsonb->'ru' ?| name_)
                                      OR (name::jsonb->'en' ?| name_)))
															  AND ( (query_ IS NULL) 
																    OR  ( name::jsonb->>'ru'             ILIKE like_str_ )
																   OR  ( name::jsonb->>'en'             ILIKE like_str_ )
																   OR  ( name::jsonb->>'international'  ILIKE like_str_ )
																   OR  ( index::jsonb->>'cccc'          ILIKE like_str_ )
																   OR  ( index::jsonb->>'station_name'  ILIKE like_str_ )
																   OR  ( index::jsonb->>'station_index' ILIKE like_str_ )
																   OR  station ILIKE like_str_)
																   and (region_ IS NULL OR ST_Within(meteo.stations.location, region_))
															  LIMIT limit_
															  OFFSET skip_;
  RETURN ref;
END;
$$;


ALTER FUNCTION meteo.get_stations_curs(id_ integer[], station_ text[], station_type_ integer[], station_index_ integer[], station_cccc_ text[], name_ text[], country_ integer[], skip_ integer, limit_ integer, query_ text, region_ public.geometry) OWNER TO postgres;

--
-- Name: index_partition(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.index_partition(partition_name text) RETURNS void
    LANGUAGE plpgsql
    AS $$
BEGIN
 -- Ensure we have all the necessary indices in this partition;
 -- More indexes here...
END;
$$;


ALTER FUNCTION meteo.index_partition(partition_name text) OWNER TO postgres;

--
-- Name: insert_forecast(jsonb[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_forecast(fdata_ jsonb[]) RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE
 cur_ jsonb;
BEGIN
 
 FOREACH cur_ in ARRAY fdata_
  loop
  
  perform meteo.insert_forecast(cur_);
  end loop;
   				    
 RETURN 1;
END;
$$;


ALTER FUNCTION meteo.insert_forecast(fdata_ jsonb[]) OWNER TO postgres;

--
-- Name: insert_forecast(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_forecast(fdata_ jsonb) RETURNS integer
    LANGUAGE plpgsql
    AS $_$DECLARE
 station_id text;
 station text;
 station_type int;
 descrname text;
 dt timestamp;
 dt_beg timestamp;
 dt_end timestamp;
 model smallint;
BEGIN
 --station_id := fdata_->'q'->'station_info._id';
 descrname := trim(both '"' from fdata_->'u'->'$addToSet'->'param'->>'descrname');
 station   := trim(both '"' from fdata_->'u'->'$setOnInsert'->'station_info'->>'station');
 station_type := (fdata_->'u'->'$setOnInsert'->'station_info'->>'station_type')::integer;
 dt = fdata_->'q'->'dt'->'$date';
 dt_beg = fdata_->'q'->'dt_beg'->'$date';
 dt_end = fdata_->'q'->'dt_end'->'$date';
 
 IF fdata_->'q' ? 'model' THEN
 	model = fdata_->'q'->>'model';
 END IF;
 

 EXECUTE FORMAT('INSERT INTO meteo.forecast("dt", "dt_beg", "dt_end", "station", "station_type", "level", 
				"level_type", "center", "hour", "model", "station_info", "descrname", "param")
				 VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
				 ON CONFLICT ON CONSTRAINT insert_forecast_uniq DO NOTHING')
				 --UPDATE SET param = jsonb_insert(meteo.forecast.param, ''{-1}'', $12, true)') 
				 USING 			
				 dt, dt_beg, dt_end, station, station_type,
				 (fdata_->'q'->>'level')::integer, (fdata_->'q'->>'level_type')::smallint, (fdata_->'q'->>'center')::integer, 
				 (fdata_->'q'->>'hour')::integer, model, 
				 fdata_->'u'->'$setOnInsert'->'station_info', descrname, fdata_->'u'->'$addToSet'->'param';
   				    
 RETURN 1;
END;
$_$;


ALTER FUNCTION meteo.insert_forecast(fdata_ jsonb) OWNER TO postgres;

--
-- Name: insert_forecast_accuracy(integer, timestamp without time zone, timestamp without time zone, integer, integer, text, integer, text, real, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_forecast_accuracy(center_ integer, dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone, level_ integer, level_type_ integer, station_ text, station_type_ integer, descrname_ text, field_value_ real, dt_ timestamp without time zone) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
	id_ bigint;
	dt_text_ jsonb;
	field_ jsonb;
BEGIN
	dt_text_ = jsonb_build_object('dt', dt_);
	
	SELECT id INTO id_ FROM ( SELECT id, jsonb_array_elements(field) AS lst FROM meteo.forecast 
							    WHERE center = center_ AND dt_beg <= dt_beg_ AND dt_end >= dt_end_ AND
				                       level = level_ AND level_type = level_type_ AND 
							           station = station_ AND station_type = station_type_ AND
							           descrname = descrname_) t
              WHERE lst ->'dt' = dt_text_->'dt'; 
	
 	IF FOUND THEN	
		--UPDATE 
	ELSE
		UPDATE meteo.forecast SET field = jsonb_insert(meteo.forecast.field, '{-1}', jsonb_build_object('field_value', field_value_, 'dt', dt_), true)
		--, flag_obr = meteo.forecast.flag_obr + 1
			    WHERE center = center_ AND dt_beg <= dt_beg_ AND dt_end >= dt_end_ AND
				   level = level_ AND level_type = level_type_ AND station = station_ AND station_type = station_type_ AND descrname = descrname_;
	END IF;
	
 
	
   				    
 RETURN 1;
END;
$$;


ALTER FUNCTION meteo.insert_forecast_accuracy(center_ integer, dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone, level_ integer, level_type_ integer, station_ text, station_type_ integer, descrname_ text, field_value_ real, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: insert_forecast_accuracy_observed(timestamp without time zone, timestamp without time zone, integer, integer, text, integer, text, real, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_forecast_accuracy_observed(dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone, level_ integer, level_type_ integer, station_ text, station_type_ integer, descrname_ text, observ_value_ real, dt_ timestamp without time zone) RETURNS void
    LANGUAGE plpgsql
    AS $$DECLARE
	id_ bigint;
	dt_text jsonb;
BEGIN
	dt_text = jsonb_build_object('dt', dt_); 
	
												  
	SELECT id INTO id_ FROM ( SELECT id, jsonb_array_elements(observ) AS lst FROM meteo.forecast WHERE dt_beg <= dt_beg_ AND dt_end >= dt_end_ AND
				                                          level = level_ AND level_type = level_type_ AND 
							                              station = station_ AND station_type = station_type_ AND descrname = descrname_) t
              WHERE lst ->'dt' = dt_text->'dt'; 
	
 	IF FOUND THEN	
		--UPDATE 
	ELSE
		UPDATE meteo.forecast SET observ = jsonb_insert(meteo.forecast.observ, '{-1}', jsonb_build_object('observ_value', observ_value_, 'dt', dt_), true)				
			    WHERE dt_beg <= dt_beg_ AND dt_end >= dt_end_ AND level = level_ AND level_type = level_type_ AND 
				      station = station_ AND station_type = station_type_ AND descrname = descrname_;
	END IF;
	
   				    

END;
$$;


ALTER FUNCTION meteo.insert_forecast_accuracy_observed(dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone, level_ integer, level_type_ integer, station_ text, station_type_ integer, descrname_ text, observ_value_ real, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: insert_forecast_flag_opravd(bigint[], bigint[], bigint[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_forecast_flag_opravd(id_ bigint[], opr_id_ bigint[], not_opr_id_ bigint[]) RETURNS void
    LANGUAGE plpgsql
    AS $$DECLARE

BEGIN
	UPDATE meteo.forecast SET flag_opravd = true WHERE id = ANY(id_);
	UPDATE meteo.forecast SET opr = true WHERE id = ANY(opr_id_);
	UPDATE meteo.forecast SET opr = false WHERE id = ANY(not_opr_id_);				 
   	--RAISE NOTICE '%', id_;
END;
$$;


ALTER FUNCTION meteo.insert_forecast_flag_opravd(id_ bigint[], opr_id_ bigint[], not_opr_id_ bigint[]) OWNER TO postgres;

--
-- Name: insert_forecast_opravd(text, integer, integer, integer, text, integer, text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_forecast_opravd(punkt_id_ text, level_ integer, type_level_ integer, hour_ integer, descrname_ text, center_ integer, ftype_method_ text, opr_ integer, not_opr_ integer, total_ integer) RETURNS integer
    LANGUAGE plpgsql
    AS $_$DECLARE

BEGIN
  
 EXECUTE FORMAT('INSERT INTO meteo.forecast_opr("punkt_id", "level", "type_level", "hour", "descrname", 
				                                 "center", "ftype_method", "opr", "not_opr", "total")
				 VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
				 ON CONFLICT ON CONSTRAINT forecast_opr_uniq DO 
				 UPDATE SET opr = meteo.forecast_opr.opr + $8, not_opr = meteo.forecast_opr.not_opr + $9, total = meteo.forecast_opr.total + $10') USING 			
				 punkt_id_, level_, type_level_, hour_, descrname_, center_, ftype_method_, opr_, not_opr_, total_;
				 
   				    
 RETURN 1;
END;
$_$;


ALTER FUNCTION meteo.insert_forecast_opravd(punkt_id_ text, level_ integer, type_level_ integer, hour_ integer, descrname_ text, center_ integer, ftype_method_ text, opr_ integer, not_opr_ integer, total_ integer) OWNER TO postgres;

--
-- Name: insert_grib(timestamp without time zone, bigint, integer, integer, integer, integer, bigint, real, integer, integer, integer, integer, timestamp without time zone, timestamp without time zone, bigint, integer, integer, integer, integer, integer, integer, bigint, text, integer, boolean, timestamp without time zone, boolean); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_grib(dt_ timestamp without time zone, hour_ bigint, edition_ integer, center_ integer, subcenter_ integer, product_ integer, descr_ bigint, level_ real, "levelType_" integer, "procType_" integer, "timeRange_" integer, sign_dt_ integer, dt1_ timestamp without time zone, dt2_ timestamp without time zone, status_ bigint, data_type_ integer, discipline_ integer, category_ integer, "defType_" integer, "stepLat_" integer, "stepLon_" integer, fileid_ bigint, tlgid_ text, crc_ integer, analysed_ boolean, dt_write_ timestamp without time zone DEFAULT now(), error_ boolean DEFAULT false) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
    err_context_ text;
	table_name_ text;
	query_ text;
BEGIN
    select meteo.try_create_grib_partition('grib',dt_) into table_name_;
 query_ :=  format('INSERT INTO %s
	       (dt, edition, center, subcenter, product, descr, level, "levelType", "procType", "timeRange", sign_dt, dt1, dt2,status,hour,
			data_type, discipline, category, "defType", "stepLat", "stepLon", fileid, tlgid, crc, analysed, dt_write, error) 
	VALUES (%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L,%L);'
			,table_name_,dt_,edition_,center_,subcenter_,product_,descr_,level_,"levelType_","procType_","timeRange_",sign_dt_,dt1_,dt2_,status_,hour_,
			data_type_,discipline_,category_,"defType_","stepLat_","stepLon_",fileid_,tlgid_,crc_,analysed_,dt_write_,error_);
	execute query_ ;
	RETURN TRUE;					 
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context_ = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context_;
	        RETURN FALSE;
END;$$;


ALTER FUNCTION meteo.insert_grib(dt_ timestamp without time zone, hour_ bigint, edition_ integer, center_ integer, subcenter_ integer, product_ integer, descr_ bigint, level_ real, "levelType_" integer, "procType_" integer, "timeRange_" integer, sign_dt_ integer, dt1_ timestamp without time zone, dt2_ timestamp without time zone, status_ bigint, data_type_ integer, discipline_ integer, category_ integer, "defType_" integer, "stepLat_" integer, "stepLon_" integer, fileid_ bigint, tlgid_ text, crc_ integer, analysed_ boolean, dt_write_ timestamp without time zone, error_ boolean) OWNER TO postgres;

--
-- Name: insert_into_report(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_into_report() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_main        TEXT       = TG_RELNAME;--самая главная таблица
    table_part_yyyy_mm_dd          TEXT       = '';--таблица по дням (партиция за сутки)
    rec_                RECORD;
    table_part_yyyy_mm_dd_only text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||TG_RELNAME; --самая главная таблица
  table_part_yyyy_mm_dd_only :=   TG_RELNAME  || '_' || to_char(NEW.dt, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main  || '_' || to_char(NEW.dt, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
          
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
          EXECUTE  'CREATE TABLE ' || table_part_yyyy_mm_dd || '
          (
		  CONSTRAINT '||table_part_yyyy_mm_dd_only||'_oninsert UNIQUE (dt, station, station_type, data_type, level, level_type),
           CONSTRAINT ' || table_part_yyyy_mm_dd_only || '_created_check
                CHECK ( dt BETWEEN ''' || date_trunc('day', NEW.dt) || '''
                                AND ''' || date_trunc('day', NEW.dt) + interval '1 day' - interval '1 sec' || ''')
          )
          INHERITS (' || table_main || ')
          WITH (OIDS = FALSE, FILLFACTOR = 90)';

          -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
          FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                                FROM information_schema.role_table_grants
                                WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
            EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part_yyyy_mm_dd || ' TO ' || rec_.grantee;
          END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_part_yyyy_mm_dd || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';

    -- создаём индексы
   EXECUTE ' CREATE INDEX  '||table_part_yyyy_mm_dd_only ||'_gin_idx
    ON '|| table_part_yyyy_mm_dd||' USING gin
    (param jsonb_path_ops)
    TABLESPACE pg_default;';
    
     IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_dt_station_station_type_data_type_level_level_type_idx' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'dt', 'station', 'station_type', 'data_type', 'level', 'level_type']);
    END IF;
 END IF;

  -- Вставляем данные в партицию ----------------------------------------
  EXECUTE 'INSERT INTO ' || table_part_yyyy_mm_dd || ' SELECT ((' || quote_literal(NEW) || ')::' || table_main || ').*';
  --raise notice 'query = %','INSERT INTO ' || table_part_hh || ' SELECT ((' || quote_literal(NEW) || ')::' || table_part_yyyy_mm_dd || ').*';

  RETURN NULL;
END;
$$;


ALTER FUNCTION meteo.insert_into_report() OWNER TO postgres;

--
-- Name: FUNCTION insert_into_report(); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.insert_into_report() IS 'Функция создаёт дочернюю таблицу, отнаследованную от таблицы, на которой сработал триггер (table_master), с именем table_master_yyyy_mm_dd, если таковая не существует. 
	Данные будут вставлены в дочернюю таблицу.

NB: Триггер должен быть установлен на событие BEFORE INSERT FOR EACH ROW';


--
-- Name: insert_manual_forecast(bigint, integer, integer, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer, text, text, jsonb, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_manual_forecast(id_ bigint, region_id_ integer, user_id_ integer, dt_ timestamp without time zone, dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, forecast_hour_ integer, forecast_code_ text, forecast_text_ text, forecast_params_ jsonb, station_id_ integer) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE
    err_context text;
	ret_id bigint;
BEGIN
	RAISE INFO 'before update:%', id_;
	UPDATE meteo.forecast_manual SET dt               = dt_,
			dt_start         = dt_start_,
			dt_end           = dt_end_,
			forecast_hour    = forecast_hour_,
			forecast_code    = forecast_code_,
			forecast_text    = forecast_text_,
			forecast_params  = forecast_params_
		WHERE id = id_; 
	RAISE INFO 'after update:%', id_;
	IF NOT FOUND THEN 
		INSERT INTO meteo.forecast_manual( region_id, user_id, dt, dt_start, dt_end, forecast_hour, forecast_code, forecast_text, forecast_params, station_id ) 
	                     VALUES (  region_id_, user_id_, dt_, dt_start_, dt_end_, forecast_hour_, forecast_code_, forecast_text_, forecast_params_, station_id_ ) 
						 RETURNING id INTO ret_id;
		RAISE INFO 'insert:%', ret_id;
		RETURN ret_id;
	END IF; 
	RAISE INFO 'RETURN:%', id_;
	RETURN id_;
	EXCEPTION
		WHEN OTHERS THEN
	        GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
	        RAISE INFO 'Error Name:%',SQLERRM;
	        RAISE INFO 'Error State:%', SQLSTATE;
	        RAISE INFO 'Error Context:%', err_context;
	        RETURN 0;
END;
$$;


ALTER FUNCTION meteo.insert_manual_forecast(id_ bigint, region_id_ integer, user_id_ integer, dt_ timestamp without time zone, dt_start_ timestamp without time zone, dt_end_ timestamp without time zone, forecast_hour_ integer, forecast_code_ text, forecast_text_ text, forecast_params_ jsonb, station_id_ integer) OWNER TO postgres;

--
-- Name: insert_radar(text, timestamp without time zone, timestamp without time zone, text, real, real, integer, integer, bigint, text, integer, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_radar(collection text, dt_write_ timestamp without time zone, dt_ timestamp without time zone, station_ text, h1_ real, h2_ real, product_ integer, count_ integer, fileid_ bigint, tlgid_ text, data_type_ integer, source_ text, ii_ integer) RETURNS real
    LANGUAGE plpgsql
    AS $$
DECLARE
    insertedId text;
	tablePart text;
BEGIN
--Создаем партицию
  tablePart := global.try_create_partition_by_dt( collection, 'dt', dt_);
  
--Создаем индексы
  IF ( global.f_is_index_exist( tablePart, ARRAY[ 'dt' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( tablePart, ARRAY[ 'dt' ] );
  END IF;
  
  IF ( global.f_is_index_exist( tablePart, ARRAY[ 'product' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( tablePart, ARRAY[ 'product' ] );
  END IF;
  
  IF ( global.f_is_index_exist( tablePart, ARRAY[ 'fileid' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( tablePart, ARRAY[ 'fileid' ] );
  END IF;
  
  IF ( global.f_is_index_exist( tablePart, ARRAY[ 'data_type', 'dt', 'h1', 'h2', 'product' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( tablePart, ARRAY[ 'data_type', 'dt', 'h1', 'h2', 'product' ] );
  END IF;
  
  EXECUTE FORMAT('INSERT INTO %s ( dt_write, dt, station, h1, h2, product, count, fileid, tlgid, data_type, source, ii )
				 VALUES (  %L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L )  RETURNING "id"',
				 tablePart, dt_write_, dt_, station_, h1_, h2_, product_, count_, fileid_, tlgid_, data_type_, source_, ii_) INTO insertedId;
  IF insertedId = Null
  THEN
    return 0;
  ELSE
    return 1;
  END IF;	

END;
$$;


ALTER FUNCTION meteo.insert_radar(collection text, dt_write_ timestamp without time zone, dt_ timestamp without time zone, station_ text, h1_ real, h2_ real, product_ integer, count_ integer, fileid_ bigint, tlgid_ text, data_type_ integer, source_ text, ii_ integer) OWNER TO postgres;

--
-- Name: insert_sigwx(timestamp without time zone, smallint, smallint, integer, integer, integer, integer, smallint, text, smallint, bigint, bigint, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_sigwx(dt_ timestamp without time zone, center_ smallint, level_type_ smallint, level_lo_ integer, level_hi_ integer, descriptor_ integer, value_ integer, data_type_ smallint, source_ text, ii_ smallint, fileid_ bigint, tlgid_ bigint, dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
  tablename TEXT;
BEGIN
    tablename := meteo.try_create_sigwx_partition(dt_);
	
	
	EXECUTE FORMAT( 'INSERT INTO %s ( "dt", "center", "level_type", "level_lo", "level_hi",
				                      "descriptor", "value", "data_type", "source", "ii", "dt_write",
				                      "fileid", "tlgid", "dt_beg", "dt_end" )
					 VALUES (%L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L );', 
				     tablename,
				     dt_, center_, level_type_, level_lo_, level_hi_, descriptor_, value_,
				     data_type_, source_, ii_, now() at time zone 'utc', fileid_, tlgid_, dt_beg_, dt_end_ );

  RETURN TRUE;
END;$$;


ALTER FUNCTION meteo.insert_sigwx(dt_ timestamp without time zone, center_ smallint, level_type_ smallint, level_lo_ integer, level_hi_ integer, descriptor_ integer, value_ integer, data_type_ smallint, source_ text, ii_ smallint, fileid_ bigint, tlgid_ bigint, dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone) OWNER TO postgres;

--
-- Name: FUNCTION insert_sigwx(dt_ timestamp without time zone, center_ smallint, level_type_ smallint, level_lo_ integer, level_hi_ integer, descriptor_ integer, value_ integer, data_type_ smallint, source_ text, ii_ smallint, fileid_ bigint, tlgid_ bigint, dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.insert_sigwx(dt_ timestamp without time zone, center_ smallint, level_type_ smallint, level_lo_ integer, level_hi_ integer, descriptor_ integer, value_ integer, data_type_ smallint, source_ text, ii_ smallint, fileid_ bigint, tlgid_ bigint, dt_beg_ timestamp without time zone, dt_end_ timestamp without time zone) IS 'Вставка информации о SIGWX';


--
-- Name: json2arr(jsonb, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.json2arr(_j jsonb, _key text) RETURNS text[]
    LANGUAGE sql IMMUTABLE
    AS $$SELECT ARRAY(SELECT elem->>_key FROM jsonb_array_elements(_j) elem)$$;


ALTER FUNCTION meteo.json2arr(_j jsonb, _key text) OWNER TO postgres;

--
-- Name: put_file_description(text, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.put_file_description(tablename_ text, filename_ text, filemd5_ text, filesize_ integer) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE
  err_context TEXT;
  idrec_ BIGINT;
  query_ TEXT;
  rec_ RECORD;
  dup_md5_ boolean;
  tablepart_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  SELECT * FROM meteo.try_create_fs_part(tablename_)
    AS ( tablepart TEXT, id BIGINT ) LIMIT 1 INTO rec_;

  tablepart_ = rec_.tablepart;

--      raise notice 'part =%', rec_;
--      raise notice 'partition =%', rec_.tablepart;


  query_ = FORMAT('WITH descr AS (
          INSERT INTO meteo.%s ( id, filename, filemd5, filesize )
          VALUES ( %s, %L, %L, %L )
          ON CONFLICT (filemd5) DO NOTHING 
                                  RETURNING * )
      SELECT * FROM descr;', tablepart_, rec_.id, filename_, filemd5_, filesize_ );
  EXECUTE query_ INTO rec_;
  dup_md5_ := false;
  IF ( rec_.id IS NULL ) THEN
    query_ = FORMAT( 'SELECT * FROM meteo.%s WHERE filemd5 = %L LIMIT 1', tablepart_, filemd5_);
        EXECUTE query_ INTO rec_;
        dup_md5_ := TRUE;
  END IF;
  RETURN (rec_.id, rec_.filename, rec_.filemd5, rec_.filesize, dup_md5_);
  EXCEPTION
    WHEN OTHERS THEN
          GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
          RAISE INFO 'Error Name:%',SQLERRM;
          RAISE INFO 'Error State:%', SQLSTATE;
          RAISE INFO 'Error Context:%', err_context;
          RETURN idrec_;
END;$$;


ALTER FUNCTION meteo.put_file_description(tablename_ text, filename_ text, filemd5_ text, filesize_ integer) OWNER TO postgres;

--
-- Name: remove_file_description(text, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.remove_file_description(tablename_ text, id_ bigint) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
  query_ TEXT;
  delcount_ INTEGER;
BEGIN
  delcount_ = 0;
  PERFORM meteo.check_fs_table(tablename_);
  IF id_ IS NULL THEN
    RETURN FALSE;
  END IF;
  query_ = FORMAT( 'WITH del AS (
				      DELETE FROM meteo.%I WHERE id=%L RETURNING id)
				    SELECT count(*) FROM del;', tablename_, id_ );
  EXECUTE query_ INTO delcount_;
  IF ( 0 >= delcount_ ) THEN
	RETURN FALSE;
  END IF;
  RETURN TRUE;
END;$$;


ALTER FUNCTION meteo.remove_file_description(tablename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: FUNCTION remove_file_description(tablename_ text, id_ bigint); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.remove_file_description(tablename_ text, id_ bigint) IS 'Удалить описание файла';


--
-- Name: reset_meteodata_quality(timestamp without time zone, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.reset_meteodata_quality(dt_ timestamp without time zone, descrname_ text) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
 param_to_update_ meteo.id_param;
 new_param_ jsonb;
 cur_value_ jsonb;
 new_descr_ jsonb;
 upd_params_ jsonb;
 query_ text;
 rrr timestamp without time zone;
 count_value_ bigint; 
 has_new_ boolean;
 res_ boolean;
 table_name_  text;
 i_ integer;
 cur_control_type_ integer;
BEGIN
 table_name_:='meteo.report';
 has_new_ := FALSE; 
 --rrr :=  clock_timestamp();
 for param_to_update_ in 
	 SELECT id, param from meteo.report WHERE (dt_ IS NULL or dt = dt_ ) 
    LOOP 
	IF param_to_update_.param is NULL then continue; end if;
    SELECT  jsonb_array_length(param_to_update_.param) into count_value_;
	IF 0 = count_value_ then continue; end if;

	SELECT json_build_object('descrname',descrname_) into new_descr_;
	 i_ := 0;
	  new_param_ := '[]'::jsonb;
	 count_value_:= count_value_-1;
	FOR i_ IN 0..count_value_ -- удаляем записи
		 LOOP 
		 cur_value_ := param_to_update_.param->i_;
		 SELECT cur_value_ @> new_descr_ into res_;
		 IF TRUE = res_ THEN
			cur_control_type_:= cur_value_->'control';
			SELECT jsonb_set(cur_value_,'{quality}',to_jsonb(9)) into cur_value_;
			SELECT jsonb_set(cur_value_,'{control}',to_jsonb(0)) into cur_value_;
			has_new_ := TRUE;
		 END IF;
		 SELECT new_param_ || cur_value_ into new_param_;
	 END LOOP;
	 IF TRUE = has_new_ THEN -- если были изменения то добавляем
	  EXECUTE format('update %s SET dt_update = now(), "param" = %L::jsonb
		where
		   (%L IS NULL or "dt" = %L::timestamp ) and
		   "id" = %L;',
			table_name_,new_param_,dt_,dt_,param_to_update_.id);	  
	 END IF;
 END LOOP;
-- RAISE NOTICE ' -time operation: %', clock_timestamp()-rrr;
 RETURN 1;
END;$$;


ALTER FUNCTION meteo.reset_meteodata_quality(dt_ timestamp without time zone, descrname_ text) OWNER TO postgres;

--
-- Name: FUNCTION reset_meteodata_quality(dt_ timestamp without time zone, descrname_ text); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.reset_meteodata_quality(dt_ timestamp without time zone, descrname_ text) IS 'обновление показателя качества данных наблюдений в meteo.report';


--
-- Name: try_create_fs_part(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_fs_part(tablemain_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
  tablepart_ TEXT;
  tablepart_only_ TEXT;
  id_ BIGINT;
  minval_ BIGINT;
  maxval_ BIGINT;
  query_ TEXT;
BEGIN
  query_ = FORMAT('SELECT nextval(''meteo.%s_id_seq''::regclass);', tablemain_ );
  EXECUTE query_ INTO id_; 
  minval_ := id_ - id_%100000;
  maxval_ := minval_ + 100000;
  tablepart_ = 'meteo.' || tablemain_ || '_' || CAST( minval_ AS TEXT ) || '_' || CAST( maxval_ AS TEXT );
  tablepart_only_ = tablemain_ || '_' || CAST( minval_ AS TEXT ) || '_' || CAST( maxval_ AS TEXT );
  PERFORM 1 FROM pg_tables WHERE tablename = tablepart_only_ LIMIT 1;
  IF NOT FOUND
  THEN
    query_ := format(
      -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
      'CREATE UNLOGGED TABLE %s
          (
		  CONSTRAINT %I_oninsert UNIQUE (filemd5),
          CONSTRAINT %I_id_check
                CHECK ( id BETWEEN %L::BIGINT
                                AND %L::BIGINT - 1)
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
		 tablepart_, tablepart_only_, tablepart_only_,
         minval_, maxval_,
		 'meteo.' || tablemain_);
    EXECUTE query_  ;
	    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || tablepart_ || ' ADD CONSTRAINT pkey_' || tablepart_only_ || ' PRIMARY KEY (id)';

    -- создаём индексы
	IF ( meteo.f_is_index_exist(tablepart_only_ || '_id' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(tablepart_, ARRAY[ 'id'] );
	END IF;
	IF ( meteo.f_is_index_exist(tablepart_only_ || '_filemd5' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(tablepart_, ARRAY[ 'filemd5'] );
	END IF;
	IF ( meteo.f_is_index_exist(tablepart_only_ || '_filename' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(tablepart_, ARRAY[ 'filename'] );
	END IF;
  END IF;
  RETURN (tablepart_only_, id_);
END;
$$;


ALTER FUNCTION meteo.try_create_fs_part(tablemain_ text) OWNER TO postgres;

--
-- Name: try_create_grib_partition(text, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_grib_partition(table_main_ text, dt_ timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd          TEXT       = '';--таблица по дням (партиция за сутки)
    rec_                RECORD;
    table_part_yyyy_mm_dd_only text;
	query_ text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_main_; --самая главная таблица
  table_part_yyyy_mm_dd_only :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
          query_ := format(
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
            'CREATE UNLOGGED TABLE %s
          (
		     CONSTRAINT %I_created_check
             CHECK ( dt BETWEEN %L::timestamp
                               AND %L::timestamp + interval ''1 day'' - interval ''1 sec'')
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
			 table_part_yyyy_mm_dd,table_part_yyyy_mm_dd_only,
			  date_trunc('day', dt_),date_trunc('day', dt_),
			  table_main);
          EXECUTE query_  ;
          -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
          FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                                FROM information_schema.role_table_grants
                                WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
            EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part_yyyy_mm_dd || ' TO ' || rec_.grantee;
          END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_part_yyyy_mm_dd || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';

    -- создаём индексы
    /* IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only ||
		'_dt_descr_hour_model_level_level_type_center_forecast_start_forecast_end' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, 
	  ARRAY[ 'dt', 'descr', 'hour', 'model', 'level', 'level_type', 'center', 'forecast_start', 'forecast_end']);
    END IF;*/
 END IF;
 RETURN table_part_yyyy_mm_dd;
END;
$$;


ALTER FUNCTION meteo.try_create_grib_partition(table_main_ text, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: try_create_obanal_partition(text, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_obanal_partition(table_main_ text, dt_ timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd          TEXT       = '';--таблица по дням (партиция за сутки)
    rec_                RECORD;
    table_part_yyyy_mm_dd_only text;
	query_ text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_main_; --самая главная таблица
  table_part_yyyy_mm_dd_only :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
          query_ := format(
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
            'CREATE UNLOGGED TABLE %s
          (
		  CONSTRAINT %I_oninsert UNIQUE (
		  dt, descr, hour, model, level, level_type, center, net_type,forecast_start, forecast_end),
           CONSTRAINT %I_created_check
                CHECK ( dt BETWEEN %L::timestamp
                                AND %L::timestamp + interval ''1 day'' - interval ''1 sec'')
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
			 table_part_yyyy_mm_dd,table_part_yyyy_mm_dd_only,table_part_yyyy_mm_dd_only,
			  date_trunc('day', dt_),date_trunc('day', dt_),
			  table_main);
          EXECUTE query_  ;
          -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
          FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                                FROM information_schema.role_table_grants
                                WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
            EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part_yyyy_mm_dd || ' TO ' || rec_.grantee;
          END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_part_yyyy_mm_dd || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';

    -- создаём индексы
     IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only ||
		'_dt_descr_hour_model_level_level_type_center_forecast_start_forecast_end' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, 
	  ARRAY[ 'dt', 'descr', 'hour', 'model', 'level', 'level_type', 'center', 'forecast_start', 'forecast_end']);
    END IF;
 END IF;

  -- Вставляем данные в партицию ----------------------------------------
  -- EXECUTE 'INSERT INTO ' || table_part_yyyy_mm_dd || ' SELECT ((' || quote_literal(NEW) || ')::' || table_main || ').*';
  --raise notice 'query = %','INSERT INTO ' || table_part_hh || ' SELECT ((' || quote_literal(NEW) || ')::' || table_part_yyyy_mm_dd || ').*';

  RETURN table_part_yyyy_mm_dd;
END;
$$;


ALTER FUNCTION meteo.try_create_obanal_partition(table_main_ text, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: try_create_partition(text, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_partition(table_main_ text, dt_ timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd          TEXT       = '';--таблица по дням (партиция за сутки)
    rec_                RECORD;
    table_part_yyyy_mm_dd_only text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_main_; --самая главная таблица
  table_part_yyyy_mm_dd_only :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
          
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
          EXECUTE  'CREATE UNLOGGED TABLE ' || table_part_yyyy_mm_dd || '
          (
		  CONSTRAINT '||table_part_yyyy_mm_dd_only||'_oninsert UNIQUE (dt, station, station_type, data_type, level, level_type),
           CONSTRAINT ' || table_part_yyyy_mm_dd_only || '_created_check
                CHECK ( dt BETWEEN ''' || date_trunc('day', dt_) || '''
                                AND ''' || date_trunc('day', dt_) + interval '1 day' - interval '1 sec' || ''')
          )
          INHERITS (' || table_main || ')
          WITH (OIDS = FALSE, FILLFACTOR = 90)';

          -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
          FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                                FROM information_schema.role_table_grants
                                WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
            EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part_yyyy_mm_dd || ' TO ' || rec_.grantee;
          END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_part_yyyy_mm_dd || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';

    -- создаём индексы
   EXECUTE ' CREATE INDEX  '||table_part_yyyy_mm_dd_only ||'_gin_idx
    ON '|| table_part_yyyy_mm_dd||' USING gin
    (param jsonb_path_ops)
    TABLESPACE pg_default;';
    
     IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_dt_station_station_type_data_type_level_level_type_idx' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'dt', 'station', 'station_type', 'data_type', 'level', 'level_type']);
    END IF;
 END IF;

  -- Вставляем данные в партицию ----------------------------------------
  -- EXECUTE 'INSERT INTO ' || table_part_yyyy_mm_dd || ' SELECT ((' || quote_literal(NEW) || ')::' || table_main || ').*';
  --raise notice 'query = %','INSERT INTO ' || table_part_hh || ' SELECT ((' || quote_literal(NEW) || ')::' || table_part_yyyy_mm_dd || ').*';

  RETURN table_part_yyyy_mm_dd;
END;
$$;


ALTER FUNCTION meteo.try_create_partition(table_main_ text, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: try_create_sigwx_partition(timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_sigwx_partition(dt timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE  
  table_part text;
BEGIN
  table_part := global.try_create_partition_by_dt( 'sigwx', 'dt', dt);
  
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  END IF;
  	
 RETURN table_part;
END;
$$;


ALTER FUNCTION meteo.try_create_sigwx_partition(dt timestamp without time zone) OWNER TO postgres;

--
-- Name: try_create_station_history_partition(text, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_station_history_partition(table_main_ text, dt_ timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd          TEXT       = '';--таблица по дням (партиция за сутки)
    rec_                RECORD;
    table_part_yyyy_mm_dd_only text;
	query_ text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_main_; --самая главная таблица
  table_part_yyyy_mm_dd_only :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
	      --актуализируем состояние stations
	      --PERFORM FROM meteo.actualize_stations_state(); 
          query_ := format(
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
            'CREATE UNLOGGED TABLE %s
          (
			 CONSTRAINT %Istation_date UNIQUE (station, data_type, dt),
		     CONSTRAINT %I_created_check
             CHECK ( dt BETWEEN %L::timestamp
                               AND %L::timestamp + interval ''1 day'' - interval ''1 sec'')
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
			 table_part_yyyy_mm_dd,table_part_yyyy_mm_dd_only,
			  table_part_yyyy_mm_dd_only,
			  date_trunc('day', dt_),date_trunc('day', dt_),
			  table_main);
          EXECUTE query_  ;
          -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
          FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                                FROM information_schema.role_table_grants
                                WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
            EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part_yyyy_mm_dd || ' TO ' || rec_.grantee;
          END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_part_yyyy_mm_dd || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';

    -- создаём индексы
     IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only ||
		'_data_type_dt' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, 
	  ARRAY[ 'data_type','dt']);
    END IF;
 END IF;
 RETURN table_part_yyyy_mm_dd;
END;
$$;


ALTER FUNCTION meteo.try_create_station_history_partition(table_main_ text, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: udalit(date, integer[], integer, integer, integer, text[], integer[], timestamp without time zone, integer, real, real); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.udalit(_dt date, _data_type integer[], _level integer, _type_level integer, _type_query integer, _station text[] DEFAULT NULL::integer[], _descriptor integer[] DEFAULT NULL::integer[], _term timestamp without time zone DEFAULT now(), _max_secs integer DEFAULT 86400, _min_lo real DEFAULT NULL::real, _max_lo real DEFAULT NULL::real) RETURNS TABLE(station_index text, la real, lo real, descr integer[], add_descr integer[], val real[], qual integer[], gruppa integer[], code text[], alt double precision, ptkpp bigint[], dt timestamp without time zone, dt_type integer[], dt1 timestamp without time zone[])
    LANGUAGE plpgsql
    AS $$
DECLARE
 rec RECORD;
 q varchar;
 termloc_ TIMESTAMP WITHOUT TIME ZONE;
 max_secsloc_ INTEGER;

BEGIN
/*
_type_query - входной параметр - 2 - отбор последнего значения, 3 - отбор ближайшего к _term значения
*/
IF ( _term IS NULL ) THEN
  termloc_ = now();
ELSE
  termloc_ = _term;
END IF;
IF ( _max_secs IS NULL ) THEN
  max_secsloc_ = 86400;
ELSE
  max_secsloc_ = _max_secs;
END IF;
q = '
 SELECT
   DISTINCT ON (stnumber)
   stnumber, max(coords) AS coords, max(stheight) AS stheight,dt,
   array_agg( value) AS val,
   array_agg(descriptor) AS descr,
   array_agg(add_descr) AS add_descr,
   array_agg( data_group) AS gruppa,
   array_agg( quality) AS qual,
   array_agg( var) AS code,
   array_agg(ptkpp_id) AS ptkpp,
   array_agg( dt_type) AS dt_type,
   array_agg( dt1) AS dt1';

    IF ( 3 = _type_query ) THEN
      q = q || ',  ABS( extract(epoch FROM dt - '''|| termloc_ ||''' ))  AS secs';
    END IF;

    q = q || '   FROM meteo_data ';


      q = q || '
       WHERE (
            data_type IN (' || array_to_string(_data_type, ',') || ') AND
            dt::date = ''' ||  _dt || ''' AND
            stnumber IS NOT NULL';
  IF ( _level IS NOT NULL ) THEN
    q = q || ' AND level = '|| _level ||' ';
  END IF;
  IF ( _type_level IS NOT NULL ) THEN
    q = q || ' AND level_type = '|| _type_level ||' ';
  END IF;

  q = q || ' AND
            coords IS NOT NULL';

 IF ( _descriptor  IS NOT NULL ) THEN
   q = q || '
   AND descriptor IN ( ' || array_to_string(_descriptor,',') || ' )';
 END IF;
IF ( _station  IS NOT NULL ) THEN
   q = q || '
            AND stnumber = ANY (''{' || array_to_string(_station,',') || '}'') ';
 END IF;
 IF ( 3 = _type_query ) THEN
   q = q || '
     AND ABS( extract (epoch FROM dt - '''|| termloc_ ||'''))  <= ' || max_secsloc_ ||  ' ';
 END IF;
 IF ( _min_lo IS NOT NULL ) THEN
   q = q || '
   AND ST_X(coords)>=' || _min_lo || ' ';
 END IF;
 IF ( _max_lo IS NOT NULL ) THEN
   q = q || '
   AND ST_X(coords)<=' || _max_lo || ' ';
 END IF;

 q = q || '
   )  ';
   q = q || '
 GROUP BY stnumber, dt ';
 IF ( 0 = _type_query OR 1 = _type_query ) THEN
   q = q || ',value ';
 END IF;
     q = q || '
      ORDER BY stnumber ASC ';

   IF ( 2 = _type_query ) THEN
     q = q || ',meteo_data.dt DESC';
   ELSE
     q = q || ', secs ASC';
   END IF;


   q = q || ' ;';

--raise notice '%', q;
FOR rec IN EXECUTE q

  LOOP
        station_index = rec.stnumber;
        la = ST_Y(rec.coords);
        lo =  ST_X(rec.coords);
        alt = rec.stheight;
        descr = rec.descr;
        add_descr = rec.add_descr;
        val = rec.val;
        add_descr = rec.add_descr;
        qual = rec.qual;
        gruppa = rec.gruppa;
        code = rec.code;
        ptkpp = rec.ptkpp;
        dt = rec.dt;
        dt_type = rec.dt_type;
        dt1 = rec.dt1;
    RETURN next;
  END LOOP;

END;
$$;


ALTER FUNCTION meteo.udalit(_dt date, _data_type integer[], _level integer, _type_level integer, _type_query integer, _station text[], _descriptor integer[], _term timestamp without time zone, _max_secs integer, _min_lo real, _max_lo real) OWNER TO postgres;

--
-- Name: update_file_description(text, bigint, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_file_description(tablename_ text, id_ bigint, filename_ text, filemd5_ text, filesize_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
  query_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  query_ = FORMAT('UPDATE meteo.%I SET filename = %L,
				                       filemd5 = %L,
				                       filesize = %L WHERE id = %L;', tablename_, filename_, filemd5_, filesize_, id_ );

  EXECUTE query_;
  RETURN TRUE;
END;$$;


ALTER FUNCTION meteo.update_file_description(tablename_ text, id_ bigint, filename_ text, filemd5_ text, filesize_ integer) OWNER TO postgres;

--
-- Name: update_grib_set_analysed_true(bigint[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_grib_set_analysed_true(ids_ bigint[]) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
  UPDATE meteo.grib SET "analysed" = TRUE WHERE id = ANY(ids_);
  RETURN TRUE;
END;$$;


ALTER FUNCTION meteo.update_grib_set_analysed_true(ids_ bigint[]) OWNER TO postgres;

--
-- Name: update_meteodata(meteo.update_data_quality[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_meteodata(udata_ meteo.update_data_quality[]) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
 cur_udata_ meteo.update_data_quality;
BEGIN
  FOREACH cur_udata_ in ARRAY udata_
  loop
  RAISE NOTICE '  %', cur_udata_;

  perform meteo.update_meteodata(cur_udata_);
  end loop;
 RETURN 1;
END;$$;


ALTER FUNCTION meteo.update_meteodata(udata_ meteo.update_data_quality[]) OWNER TO postgres;

--
-- Name: FUNCTION update_meteodata(udata_ meteo.update_data_quality[]); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.update_meteodata(udata_ meteo.update_data_quality[]) IS 'обновление показателя качества данных наблюдений в meteo.report';


--
-- Name: update_meteodata(meteo.update_data_quality); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_meteodata(udata_ meteo.update_data_quality) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
 param_to_update_ meteo.id_param;
 new_param_ jsonb;
 cur_value_ jsonb;
 new_descr_ jsonb;
 upd_params_ jsonb;
 query_ text;
 rrr timestamp without time zone;
 count_value_ bigint; 
 has_new_ boolean;
 res_ boolean;
 table_name_  text;
 i_ integer;
 cur_control_type_ integer;
BEGIN
 table_name_:='meteo.report';
 has_new_ := FALSE; 
 --rrr :=  clock_timestamp();
-- SELECT id,param from meteo.report WHERE (udata_.dt IS NULL or dt = udata_.dt ) 
--	and id = udata_.id into param_to_update_;
	 
 for param_to_update_ in 
	 SELECT id, param from meteo.report WHERE (udata_.dt IS NULL or dt = udata_.dt ) 
	 and id = udata_.id --into param_to_update_; 
    LOOP 
	IF param_to_update_.param is NULL then continue; end if;
    SELECT  jsonb_array_length(param_to_update_.param) into count_value_;
	IF 0 = count_value_ then continue; end if;

	SELECT json_build_object('descrname',udata_.descrname) into new_descr_;
	 i_ := 0;
	  new_param_ := '[]'::jsonb;
	 count_value_:= count_value_-1;
	FOR i_ IN 0..count_value_ -- удаляем записи
		 LOOP 
		 cur_value_ := param_to_update_.param->i_;
		 SELECT cur_value_ @> new_descr_ into res_;
		 IF TRUE = res_ THEN
			cur_control_type_:= cur_value_->'control';
			SELECT jsonb_set(cur_value_,'{quality}',to_jsonb(udata_.quality)) into cur_value_;
			SELECT jsonb_set(cur_value_,'{control}',to_jsonb(udata_.control_type|cur_control_type_)) into cur_value_;
			has_new_ := TRUE;
		 END IF;
		 SELECT new_param_ || cur_value_ into new_param_;
	 END LOOP;
	 IF TRUE = has_new_ THEN -- если были изменения то добавляем
	  EXECUTE format('update %s SET dt_update = now(), "param" = %L::jsonb
		where
		   (%L IS NULL or "dt" = %L::timestamp ) and
		   "id" = %L;',
			table_name_,new_param_,udata_.dt,udata_.dt,param_to_update_.id);	  
	 END IF;
 END LOOP;
-- RAISE NOTICE ' -time operation: %', clock_timestamp()-rrr;
 RETURN 1;
END;$$;


ALTER FUNCTION meteo.update_meteodata(udata_ meteo.update_data_quality) OWNER TO postgres;

--
-- Name: FUNCTION update_meteodata(udata_ meteo.update_data_quality); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.update_meteodata(udata_ meteo.update_data_quality) IS 'обновление показателя качества данных наблюдений в meteo.report';


--
-- Name: update_meteodata(bigint[], timestamp without time zone, text, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_meteodata(id_ bigint[], dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
 param_to_update_ meteo.id_param;
 new_param_ jsonb;
 cur_value_ jsonb;
 new_descr_ jsonb;
 upd_params_ jsonb;
 query_ text;
 rrr timestamp without time zone;
 count_value_ bigint; 
 has_new_ boolean;
 res_ boolean;
 table_name_  text;
 i_ integer;
 cur_control_type_ integer;
BEGIN
 table_name_:='meteo.report';
 has_new_ := FALSE; 
 --rrr :=  clock_timestamp();
 SELECT id,param from meteo.report WHERE (dt_ IS NULL or dt = dt_ ) 
	and id = ANY (id_) into param_to_update_;
	 
 for param_to_update_ in 
	 SELECT id, param from meteo.report WHERE (dt_ IS NULL or dt = dt_ ) 
	 and id = ANY (id_) --into param_to_update_; 
    LOOP 
	IF param_to_update_.param is NULL then continue; end if;
    SELECT  jsonb_array_length(param_to_update_.param) into count_value_;
	IF 0 = count_value_ then continue; end if;

	SELECT json_build_object('descrname',descrname_) into new_descr_;
	 i_ := 0;
	  new_param_ := '[]'::jsonb;
	 count_value_:= count_value_-1;
	FOR i_ IN 0..count_value_ -- удаляем записи
		 LOOP 
		 cur_value_ := param_to_update_.param->i_;
		 SELECT cur_value_ @> new_descr_ into res_;
		 IF TRUE = res_ THEN
			cur_control_type_:= cur_value_->'control';
			SELECT jsonb_set(cur_value_,'{quality}',to_jsonb(quality_)) into cur_value_;
			SELECT jsonb_set(cur_value_,'{control}',to_jsonb(control_type_|cur_control_type_)) into cur_value_;
			has_new_ := TRUE;
		 END IF;
		 SELECT new_param_ || cur_value_ into new_param_;
	 END LOOP;
	 IF TRUE = has_new_ THEN -- если были изменения то добавляем
	  EXECUTE format('update %s SET dt_update = now(), "param" = %L::jsonb
		where
		   (%L IS NULL or "dt" = %L::timestamp ) and
		   "id" = %L;',
			table_name_,new_param_,dt_,dt_,param_to_update_.id);	  
	 END IF;
 END LOOP;
-- RAISE NOTICE ' -time operation: %', clock_timestamp()-rrr;
 RETURN 1;
END;$$;


ALTER FUNCTION meteo.update_meteodata(id_ bigint[], dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer) OWNER TO postgres;

--
-- Name: FUNCTION update_meteodata(id_ bigint[], dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.update_meteodata(id_ bigint[], dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer) IS 'обновление показателя качества данных наблюдений в meteo.report';


--
-- Name: update_meteodata(bigint, timestamp without time zone, text, integer, integer, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_meteodata(id_ bigint, dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer, dt_write_ timestamp without time zone DEFAULT now()) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
 param_to_update_ jsonb;
 new_param_ jsonb;
 cur_value_ jsonb;
 new_descr_ jsonb;
 upd_params_ jsonb;
 query_ text;
 rrr timestamp without time zone;
 count_value_ bigint; 
 has_new_ boolean;
 res_ boolean;
 table_name_  text;
 i_ integer;
BEGIN
 table_name_:='meteo.report';
 has_new_ := FALSE; 
 --rrr :=  clock_timestamp();
 select param from meteo.report where (dt_ IS NULL or dt = dt_ ) and id = id_ into param_to_update_; 
	-- RAISE notice '  count_value_ %',param_to_update_;

IF param_to_update_ is NULL then return 0; end if;

 select  jsonb_array_length(param_to_update_) into count_value_;
 IF 0 = count_value_ then return 0; end if;

 select json_build_object('descrname',descrname_) into new_descr_;
 select json_build_object('quality',quality_,'control',control_type_) into upd_params_;
-- RAISE notice ' old_param_.param: %', old_param_.param;
--RAISE notice ' ur_descr_ %',cur_descr_; ,'quality',quality_,'control',control_type_
 i_ := 0;
  new_param_ := '[]'::jsonb;
 count_value_:= count_value_-1;
 FOR i_ IN 0..count_value_ -- удаляем записи
	 LOOP 
	 cur_value_ := param_to_update_->i_;
	 select cur_value_ @> new_descr_ into res_;
	 IF TRUE = res_ THEN
	    select cur_value_||upd_params_ into cur_value_;
		--select jsonb_set(cur_value_,'{quality}',to_jsonb(quality_)) into cur_value_;
		--select jsonb_set(cur_value_,'{control}',to_jsonb(control_type_)) into cur_value_;
		has_new_ := TRUE;
	   -- RAISE notice ' были изменения v %',cur_value_;
	 END IF;
	 select new_param_ || cur_value_ into new_param_;
   -- RAISE notice ' new_param_ % %',new_param_,cur_value_;

 END LOOP;

-- RAISE notice ' find.param: %', param_to_update_;
 IF TRUE = has_new_ THEN -- если были изменения то добавляем
  query_ := format('update %s SET dt_update = now(), "param" = %L::jsonb
	where
       (%L IS NULL or "dt" = %L::timestamp ) and
	   "id" = %L;',
		table_name_,new_param_,dt_,dt_,id_);	  
	execute query_;
	--RAISE notice ' обновили %', param_to_update_;
	 RETURN 1;
 END IF;
-- RAISE NOTICE ' -time operation: %', clock_timestamp()-rrr;
 RETURN 0;
END;$$;


ALTER FUNCTION meteo.update_meteodata(id_ bigint, dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer, dt_write_ timestamp without time zone) OWNER TO postgres;

--
-- Name: FUNCTION update_meteodata(id_ bigint, dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer, dt_write_ timestamp without time zone); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.update_meteodata(id_ bigint, dt_ timestamp without time zone, descrname_ text, quality_ integer, control_type_ integer, dt_write_ timestamp without time zone) IS 'обновление показателя качества данных наблюдений в meteo.report';


--
-- Name: update_station(bigint, text, integer, text, text, text, text, integer, public.geometry, double precision, double precision, double precision, double precision, double precision, integer, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_station(id_ bigint, cccc_ text, index_ integer, station_ text, name_ru_ text, name_short_ text, name_en_ text, station_type_ integer, location_ public.geometry, alt_ double precision, poima_ double precision, zatop_ double precision, dangerlevel_ double precision, airstrip_direction_ double precision, country_ integer DEFAULT '-1'::integer, last_update_ timestamp without time zone DEFAULT timezone('utc'::text, now())) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE
 station_index_ jsonb;
 name_ jsonb;
 hydro_levels_ jsonb;
 ret_id_ bigint;
 BEGIN
	IF (index_ IS NOT NULL OR cccc_ IS NOT NULL) THEN
	station_index_ := '{}';
	END IF;
	IF index_ IS NOT NULL THEN
		station_index_ := jsonb_set(station_index_, '{station_index}', to_jsonb(index_));
	END IF;
	IF cccc_ IS NOT NULL THEN
		station_index_ := jsonb_set(station_index_, '{cccc}', to_jsonb(cccc_));
	END IF;
	IF (name_ru_ IS NOT NULL OR name_en_ IS NOT NULL OR name_short_ IS NOT NULL) THEN
	name_ := '{}';
	END IF;
	IF name_ru_ IS NOT NULL THEN
		name_ := jsonb_set(name_, '{ru}', to_jsonb(name_ru_));
	END IF;
	IF name_en_ IS NOT NULL THEN
		name_ := jsonb_set(name_, '{international}', to_jsonb(name_en_));
	END IF;
	IF name_short_ IS NOT NULL THEN
		name_ := jsonb_set(name_, '{short}', to_jsonb(name_short_));
	END IF;
	IF (poima_ IS NOT NULL OR zatop_ IS NOT NULL OR dangerlevel_ IS NOT NULL) THEN
	hydro_levels_ := '{}';
	END IF;
	IF poima_ IS NOT NULL THEN
		hydro_levels_ := jsonb_set(hydro_levels_, '{poima}', to_jsonb(poima_));
	END IF;
	IF zatop_ IS NOT NULL THEN
		hydro_levels_ := jsonb_set(hydro_levels_, '{zatop}', to_jsonb(zatop_));
	END IF;
	IF dangerlevel_ IS NOT NULL THEN
		hydro_levels_ := jsonb_set(hydro_levels_, '{dangerlevel}', to_jsonb(dangerlevel_));
	END IF;
	ret_id_ := -1;
	IF last_update_  IS NULL THEN
		SELECT now() at time zone 'utc'  into last_update_ ;
	END IF;
	IF id_ IS NULL THEN
	INSERT INTO meteo.stations(station, station_type, index, name, source, alt, last_update, location, country, hydro_levels, airstrip_direction)
	                   VALUES (station_, station_type_, station_index_, name_, 'user-modified', alt_, last_update_, location_, country_, hydro_levels_,airstrip_direction_)
	        ON CONFLICT(station_type, index) DO UPDATE SET station = station_, station_type = station_type_, index = station_index_, name = name_, source = 'user-modified',
	         alt = alt_, last_update = last_update_, location = location_, country = country_, hydro_levels = hydro_levels_, airstrip_direction = airstrip_direction_ 
	          WHERE (meteo.stations.station_type = station_type_ AND meteo.stations.index = station_index_)
	          RETURNING id INTO ret_id_;
	ELSE
	UPDATE meteo.stations SET station = station_, station_type = station_type_, index = station_index_, name = name_, source = 'user-modified',
	         alt = alt_, last_update = last_update_, location = location_, country = country_, hydro_levels = hydro_levels_, airstrip_direction = airstrip_direction_ 
	          WHERE (meteo.stations.id = id_) RETURNING id INTO ret_id_;
	END IF;
  RETURN ret_id_;
END;
$$;


ALTER FUNCTION meteo.update_station(id_ bigint, cccc_ text, index_ integer, station_ text, name_ru_ text, name_short_ text, name_en_ text, station_type_ integer, location_ public.geometry, alt_ double precision, poima_ double precision, zatop_ double precision, dangerlevel_ double precision, airstrip_direction_ double precision, country_ integer, last_update_ timestamp without time zone) OWNER TO postgres;

--
-- Name: update_stations_history(text, integer, integer, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_stations_history(station_ text, station_type_ integer, data_type_ integer, used_ timestamp without time zone) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  asql_ text;
  table_name_ text;

BEGIN
IF station_ IS NULL THEN 
  RETURN;
END IF;
select meteo.try_create_station_history_partition('stations_history',used_) into table_name_;

select format('INSERT INTO %s (station, station_type,data_type, dt) 
			  VALUES (%L::text, %L::integer,%L::integer, %L::timestamp) ON CONFLICT DO NOTHING;',
              table_name_,station_, station_type_, data_type_, used_) into asql_;
 EXECUTE asql_;
RETURN;
END;
$$;


ALTER FUNCTION meteo.update_stations_history(station_ text, station_type_ integer, data_type_ integer, used_ timestamp without time zone) OWNER TO postgres;

--
-- Name: upsert_obanal(timestamp without time zone, integer, integer, integer, integer, integer, integer, integer, timestamp without time zone, timestamp without time zone, bigint, integer, integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.upsert_obanal(dt_ timestamp without time zone, descr_ integer, hour_ integer, model_ integer, level_ integer, level_type_ integer, center_ integer, net_type_ integer, forecast_start_ timestamp without time zone, forecast_end_ timestamp without time zone, fs_id_ bigint, count_points_ integer, time_range_ integer, dates_ timestamp without time zone DEFAULT NULL::timestamp without time zone, dt_write_ timestamp without time zone DEFAULT now()) RETURNS integer
    LANGUAGE plpgsql
    AS $_$DECLARE
 table_name_ text;
 query_ text;
 BEGIN
 select meteo.try_create_obanal_partition('obanal',dt_) into table_name_;
 query_ :=  format('INSERT INTO %s
	 ("dt", "descr", "hour", "model", "level", "level_type", "center", "net_type",
				  "forecast_start", "forecast_end", "fs_id", "count_points", "time_range", "dt_write")
    VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14)
	ON CONFLICT (dt, descr, hour, model, level, level_type, center, net_type, forecast_start, forecast_end) 
				   DO UPDATE SET 
				   fs_id = EXCLUDED.fs_id,
				   count_points = EXCLUDED.count_points,
				   time_range = EXCLUDED.time_range,
				   dt_write = EXCLUDED.dt_write;			   
				   ',table_name_) ;
				   
	execute query_ 
	USING dt_, descr_, hour_, model_, level_, level_type_, center_, net_type_,
				  forecast_start_, forecast_end_, fs_id_, count_points_, time_range_, dt_write_  ;
RETURN 1;
END;$_$;


ALTER FUNCTION meteo.upsert_obanal(dt_ timestamp without time zone, descr_ integer, hour_ integer, model_ integer, level_ integer, level_type_ integer, center_ integer, net_type_ integer, forecast_start_ timestamp without time zone, forecast_end_ timestamp without time zone, fs_id_ bigint, count_points_ integer, time_range_ integer, dates_ timestamp without time zone, dt_write_ timestamp without time zone) OWNER TO postgres;

--
-- Name: upsert_punkt(text, real, real, real, text, boolean, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.upsert_punkt(name_ text, fi_ real, la_ real, height_ real, station_id_ text, is_active_ boolean, station_type_ integer, OUT ok integer) RETURNS integer
    LANGUAGE plpgsql
    AS $_$
DECLARE

 BEGIN
	
 EXECUTE FORMAT('INSERT INTO meteo.punkts("name", "fi", "la", "height", "station_id", "is_active", "station_type")
   			      VALUES ($1, $2, $3, $4, $5, $6, $7) 
			      ON CONFLICT ("name", "fi", "la", "height", "station_id", "station_type") DO 
   	              UPDATE SET 
				   name = $1, fi = $2, la = $3, height = $4, station_id = $5, 
				   is_active = $6, station_type = $7 RETURNING 1') USING 
			      name_, fi_, la_, height_, station_id_, is_active_, station_type_ INTO ok;
				  
END;
$_$;


ALTER FUNCTION meteo.upsert_punkt(name_ text, fi_ real, la_ real, height_ real, station_id_ text, is_active_ boolean, station_type_ integer, OUT ok integer) OWNER TO postgres;

--
-- Name: upsert_report(timestamp without time zone, smallint, real, smallint, text, smallint, public.geometry, jsonb, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.upsert_report(dt_ timestamp without time zone, data_type_ smallint, level_ real, lev_type_ smallint, station_ text, stat_type_ smallint, coord_ public.geometry, param_ jsonb, dt_write_ timestamp without time zone DEFAULT now()) RETURNS meteo.upsert_report_result
    LANGUAGE plpgsql
    AS $$DECLARE
 result_param_ meteo.upsert_report_result;
 table_name_ text;
 query_ text;
 BEGIN
 select meteo.try_create_partition('report',dt_) into table_name_;
 query_ := format('WITH input_rows("data_type", "dt", "level", "level_type", "station", "station_type", "location", "param") AS (
   VALUES
      (%L::smallint, %L::timestamp, %L::real, %L::smallint, %L, %L::smallint, %L, %L::jsonb) 
   )
,ins AS (
	INSERT INTO %s
	 ("data_type", "dt", "level", "level_type", "station", "station_type", "location", "param")
	SELECT * FROM input_rows
	ON CONFLICT DO NOTHING RETURNING "id","param"
	)
	SELECT id, true AS inserted, param
	FROM   ins                                  
	UNION  ALL
	SELECT id, false AS inserted, c.param     
	FROM   input_rows
	JOIN   %s c USING (data_type, dt, level, level_type, station, station_type)
	LIMIT 1 ;',data_type_, dt_, level_, lev_type_, station_, stat_type_, coord_, param_,table_name_,table_name_);
	execute query_ into result_param_;
 RETURN result_param_;
END;$$;


ALTER FUNCTION meteo.upsert_report(dt_ timestamp without time zone, data_type_ smallint, level_ real, lev_type_ smallint, station_ text, stat_type_ smallint, coord_ public.geometry, param_ jsonb, dt_write_ timestamp without time zone) OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

