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
-- Name: add_aero(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_aero(param_ jsonb) RETURNS void
    LANGUAGE plpgsql
    AS $_$DECLARE

DECLARE
 idx integer;
 one jsonb;
 adt timestamp;
 adtw timestamp;
 table_part text;
 
BEGIN

idx := 0;
FOR idx IN 0..jsonb_array_length(param_)-1 LOOP
	one := param_->idx->'_id';
	adt := one->'dt'->'$date';
	adtw := param_->idx->'dt_write'->'$date';

    --RAISE notice '- данные %', adt;
	
    table_part := global.try_create_partition_by_year( 'aero', 'dt', adt);
	
	-- фиг знает, надо ли, dt в составе primary
  	--IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    --	PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  	--END IF;
	
    --table_part = 'meteo.aero';
				   
	EXECUTE FORMAT( 'INSERT INTO %s ("station", "station_type", "dt", "level", "level_type", "descrname", "value", "dt_write")
   				      VALUES (%L, %L::smallint, %L, %L::real, %L::smallint, %L, %L::real, %L) 
				      ON CONFLICT ON CONSTRAINT %s DO 
   		              UPDATE SET value = %L::real', 
				      table_part,
				      one->>'station', one->'station_type', adt, 
			 	      one->'level', one->'level_type', 
			 	      one->>'descrname', param_->idx->'value', adtw, 
				      replace(table_part,'meteo.','')||'_pkey',
				      param_->idx->'value');
   		


--	INSERT INTO meteo.aero(station, station_type, dt, level, level_type, descrname, value, dt_write)
 --  	VALUES (one->'station', (one->'station_type')::smallint, adt, 
--			 (one->'level')::real, (one->'level_type')::smallint, 
--			 one->'descrname', (param_->idx->'value')::real, adtw)
 --  	ON CONFLICT ON CONSTRAINT aero_pkey DO 
  -- 		UPDATE SET value = (param_->idx->'value')::real;
		
END LOOP;
 
END;

$_$;


ALTER FUNCTION meteo.add_aero(param_ jsonb) OWNER TO postgres;

--
-- Name: add_surf_accumulation(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_surf_accumulation(param_ jsonb) RETURNS void
    LANGUAGE plpgsql
    AS $_$DECLARE

DECLARE
 idx integer;
 one jsonb;
 adt timestamp;
 adtend timestamp;
 adtw timestamp;
 table_part text;
 tp text;
  
BEGIN
adtend = NULL;

idx := 0;
FOR idx IN 0..jsonb_array_length(param_)-1 LOOP
	one := param_->idx->'_id';
	adt := one->'dt'->'$date';
	adtw := param_->idx->'dt_write'->'$date';
	IF  one ? 'dtend' THEN
		adtend := one->'dtend'->'$date';
	END IF;
	
	--RAISE notice '- данные %', one ? 'dtend' ;
	-- таблицы понадобятся при обработке собранных в surf_accumulation
	tp := global.try_create_partition_by_year( 'surf_day', 'date', adt);
	tp := global.try_create_partition_by_year( 'surf_month', 'year', date_part('year', adt)::smallint);
	
	table_part := global.try_create_partition_by_year( 'surf_accumulation', 'dt', adt);
	
	
	-- фиг знает, надо ли, dt в составе primary
  	--IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    --	PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  	--END IF;
	
	EXECUTE FORMAT( 'INSERT INTO %s ("station", "station_type", "dt", "dtend", "descrname", "value", "dt_write")
   				      VALUES (%L, %L::smallint, %L, %L, %L, %L::real, %L) 
				      ON CONFLICT ON CONSTRAINT %s DO 
   		              UPDATE SET value = %L::real', 
				      table_part,
				      one->>'station', one->'station_type', adt, 
			 	      adtend, 
			 	      one->>'descrname', param_->idx->'value', adtw, 
				      replace(table_part,'meteo.','')||'_pkey',
				      param_->idx->'value');
   		
	
	--INSERT INTO meteo.surf_accumulation(station, station_type, dt, dtend, descrname, value, dt_write)
   	--VALUES (one->'station', (one->'station_type')::smallint, adt, adtend,			 
--			 one->'descrname', (param_->idx->'value')::real, adtw)
--   	ON CONFLICT ON CONSTRAINT surf_accumulation_pkey DO 
--   		UPDATE SET value = (param_->idx->'value')::real;
		
END LOOP;
 
END;

$_$;


ALTER FUNCTION meteo.add_surf_accumulation(param_ jsonb) OWNER TO postgres;

--
-- Name: add_surf_day(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_surf_day(param_ jsonb) RETURNS void
    LANGUAGE plpgsql
    AS $_$DECLARE

DECLARE
 idx integer;
 one jsonb;
 adt timestamp;
 adtw timestamp;
 table_part text;
 
BEGIN


idx := 0;
FOR idx IN 0..jsonb_array_length(param_)-1 LOOP
	one := param_->idx->'_id';
	adt := one->'dt'->'$date';
	adtw := param_->idx->'dt_write'->'$date';
	
	--RAISE notice '- данные %', one  ;
	table_part := global.try_create_partition_by_year( 'surf_day', 'date', adt);
	
	-- фиг знает, надо ли, dt в составе primary
  	--IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    --	PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  	--END IF;
	
	EXECUTE FORMAT( 'INSERT INTO %s ("station", "station_type", "date", "descrname", "value", "dt_write")
   				      VALUES (%L, %L::smallint, %L, %L, %L::real, %L) 
				      ON CONFLICT ON CONSTRAINT %s DO 
   		              UPDATE SET value = %L::real', 
				      table_part,
				      one->>'station', one->'station_type', adt::date, 			 	      
			 	      one->>'descrname', param_->idx->'value', adtw, 
				      replace(table_part,'meteo.','')||'_pkey',
				      param_->idx->'value');
					  
	
--	INSERT INTO meteo.surf_day(station, station_type, date, descrname, value, dt_write)
--   	VALUES (one->'station', (one->'station_type')::smallint, adt::date, 			 
--			 one->'descrname', (param_->idx->'value')::real, adtw)
--   	ON CONFLICT ON CONSTRAINT surf_day_pkey DO 
--   		UPDATE SET value = (param_->idx->'value')::real;
		
END LOOP;
 
END;

$_$;


ALTER FUNCTION meteo.add_surf_day(param_ jsonb) OWNER TO postgres;

--
-- Name: add_surf_hour(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.add_surf_hour(param_ jsonb) RETURNS void
    LANGUAGE plpgsql
    AS $_$DECLARE

DECLARE
 idx integer;
 one jsonb;
 adt timestamp;
 adtw timestamp;
 table_part text;
 ahour smallint;
   
BEGIN


idx := 0;
FOR idx IN 0..jsonb_array_length(param_)-1 LOOP
	one := param_->idx->'_id';
	adt := one->'dt'->'$date';
	adtw := param_->idx->'dt_write'->'$date';
	ahour := one->'hour';
	
	--RAISE notice '- данные %', one  ;
	table_part := global.try_create_partition_by_year( 'surf_hour', 'date', adt);
	-- фиг знает, надо ли, dt в составе primary
  	--IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    --	PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  	--END IF;
	
	EXECUTE FORMAT( 'INSERT INTO %s ("station", "station_type", "date", "hour", "descrname", "value", "dt_write")
   				      VALUES (%L, %L::smallint, %L, %L, %L, %L::real, %L) 
				      ON CONFLICT ON CONSTRAINT %s DO 
   		              UPDATE SET value = %L::real', 
				      table_part,
				      one->>'station', one->'station_type', adt::date, 
			 	      ahour, 
			 	      one->>'descrname', param_->idx->'value', adtw, 
				      replace(table_part,'meteo.','')||'_pkey',
				      param_->idx->'value');
					  
--	INSERT INTO meteo.surf_hour(station, station_type, date, hour, descrname, value, dt_write)
--   	VALUES (one->'station', (one->'station_type')::smallint, adt::date, (one->'hour')::smallint,			 
--			 one->'descrname', (param_->idx->'value')::real, adtw)
--   	ON CONFLICT ON CONSTRAINT surf_hour_pkey DO 
--   		UPDATE SET value = (param_->idx->'value')::real;
		
END LOOP;
 
END;

$_$;


ALTER FUNCTION meteo.add_surf_hour(param_ jsonb) OWNER TO postgres;

--
-- Name: clear_data(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.clear_data() RETURNS void
    LANGUAGE sql
    AS $$DELETE FROM meteo.aero;
DELETE FROM meteo.surf_accumulation;
DELETE FROM meteo.surf_day;
DELETE FROM meteo.surf_decade;
DELETE FROM meteo.surf_hour;
DELETE FROM meteo.surf_month;$$;


ALTER FUNCTION meteo.clear_data() OWNER TO postgres;

--
-- Name: get_aero(text, integer, integer, text[], integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_aero(station_ text, station_type_ integer, level_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) RETURNS TABLE(level real, dt timestamp without time zone, descrname text, value real)
    LANGUAGE plpgsql
    AS $$DECLARE

  query_ text;
  
BEGIN

  query_ = FORMAT( 'SELECT level, dt, descrname, value FROM meteo.aero WHERE 
                           station = %L AND station_type = %s 
						   AND level_type = %s
                           AND descrname = ANY(%L)',
				    station_, station_type_, level_type_, descrname_);
					
 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' and dt >= '' ' || startdt_ || ' '' ';
  END IF;

  IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' and dt <= '' ' || enddt_ || ' '' ';
  END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' and date_part(''month'', dt) = ' || month_;
 END IF;
    
 --RAISE notice '- %', query_ ;
RETURN QUERY EXECUTE query_ || 'ORDER BY level, dt, descrname';

END;
$$;


ALTER FUNCTION meteo.get_aero(station_ text, station_type_ integer, level_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_date(text, text, integer, text[], integer, timestamp without time zone, timestamp without time zone, text, real, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_date(tablename_ text, station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone, condition_ text, vlimit_ real, month_start_ integer, month_end_ integer, OUT first jsonb, OUT last jsonb) RETURNS record
    LANGUAGE plpgsql
    AS $_$

DECLARE

  query_ text;
  squery_ text;
  equery_ text;
  --start_ jsonb;
  --end_ jsonb;
  res_ record;
  
BEGIN

-- with s as
--  (select  date_part('year', date)::integer as year, MIN(date)::timestamp 
--         from meteo.surf_hour where
--           station = '26063' AND station_type = 0					
--                             AND descrname=ANY('{T}')							
--  						    AND date_part('month', date) > 7							
-- 						    AND value < 20						  
--  						  AND date >= '2019-01-01 00:00:00'
--  						  AND date <= '2021-09-06 00:00:00'
--         group by year) 		
--  select json_agg(json_build_object('year', year, 'date', min)) as start from s

--  with p as
-- (select date_part('year', date)::integer as year, MAX(date)::timestamp 
--          from meteo.surf_hour where
--           station = '26063' AND station_type = 0					
--                             AND descrname=ANY('{T}')							
--  						    AND date_part('month', date) < 8							
-- 						    AND value < 20						  
--  						  AND date >= '2019-01-01 00:00:00'
--  						  AND date <= '2021-09-06 00:00:00'
--           group by year)	  
-- select json_agg(json_build_object('year', year, 'date', max)) as end from p


 -- дата начала, окончания явления (заморозки, снег)

 -- общие условия выборки
 query_ = FORMAT('AND station = %L 
				  AND station_type = %s 						
                  AND descrname = ANY(%L)', 
				  station_, station_type_, descrname_);
				  
 IF (condition_ = '$lt') THEN
 	query_ = query_ || 'AND value <' || vlimit_;
 ELSE
    query_ = query_ || 'AND value >' || vlimit_;
 END IF;

 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' AND date >= '' ' || startdt_ || ' '' ';
 END IF;

 IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' AND date <= '' ' || enddt_ || ' '' ';
 END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' AND date_part(''month'', date) = ' || month_;   
 END IF;
 ----

 -- выборка даты начала явления 
  squery_ = FORMAT( 'WITH s AS 
					(SELECT date_part(''year'', date)::integer as year, MIN(date)::timestamp 
				      FROM meteo.%s
				      WHERE 
				           date_part(''month'', date) > %s',
				     tablename_, month_start_);

  squery_ = squery_ || query_ || 'GROUP BY year ) 		
		             				SELECT json_agg(json_build_object(''year'', year, ''date'', min)) as start from s';
	
 --RAISE notice '- %', query_ ;
 
-- выборка даты окончания явления
  equery_ = FORMAT( 'WITH p AS 
					(SELECT date_part(''year'', date)::integer as year, MAX(date)::timestamp 
				      FROM meteo.%s
				      WHERE 
				           date_part(''month'', date) < %s',
				     tablename_, month_end_);

  equery_ = equery_ || query_ || 'GROUP BY year ) 		
		              SELECT json_agg(json_build_object(''year'', year, ''date'', max)) as end from p';

 EXECUTE squery_ INTO first;
 EXECUTE equery_ INTO last;
 

--RETURN res_ ;

END;
$_$;


ALTER FUNCTION meteo.get_date(tablename_ text, station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone, condition_ text, vlimit_ real, month_start_ integer, month_end_ integer, OUT first jsonb, OUT last jsonb) OWNER TO postgres;

--
-- Name: get_lastdate_aero(text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_lastdate_aero(station_ text, station_type_ integer, OUT dt timestamp without time zone) RETURNS timestamp without time zone
    LANGUAGE plpgsql
    AS $$
DECLARE
 
 
BEGIN
  select meteo.aero.dt INTO dt from meteo.aero where station = station_ and station_type = station_type_ order by dt desc limit 1;

END;
$$;


ALTER FUNCTION meteo.get_lastdate_aero(station_ text, station_type_ integer, OUT dt timestamp without time zone) OWNER TO postgres;

--
-- Name: get_lastdate_surf_hour(text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_lastdate_surf_hour(station_ text, station_type_ integer, OUT dt timestamp without time zone) RETURNS timestamp without time zone
    LANGUAGE plpgsql
    AS $$DECLARE
 
 
BEGIN
  select date INTO dt as dt from meteo.surf_hour where station = station_ and station_type = station_type_ order by date desc limit 1;


END;
$$;


ALTER FUNCTION meteo.get_lastdate_surf_hour(station_ text, station_type_ integer, OUT dt timestamp without time zone) OWNER TO postgres;

--
-- Name: get_surf_all(text, integer, text[], integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_surf_all(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) RETURNS TABLE(descrname text, month integer, value jsonb)
    LANGUAGE plpgsql
    AS $$DECLARE

  query_ text;
  
BEGIN

-- 	SELECT descrname, date_part('month', date)::integer as month, 
--            jsonb_agg(json_build_object('value', value, 'year', date_part('year', date)::integer, 
-- 									   'day', date_part('day', date)::integer, 'hour', hour)) as value                
--                     FROM meteo.surf_hour WHERE 
--                            station = '26063' AND station_type = 0					
--                            AND descrname=ANY('{ff, R, dd}')
-- 						  -- AND date_part('month', date) = '7'
-- 						  AND date >= '2020-07-01 00:00:00'
-- 						  AND date <= '2020-08-06 00:00:00'
-- 					GROUP BY descrname, month
-- 			        ORDER BY descrname, month

  query_ = FORMAT( 'SELECT descrname, date_part(''month'', date)::integer as month, 
				    jsonb_agg(json_build_object(''value'', value, ''year'', date_part(''year'', date)::integer, 
								   ''day'', date_part(''day'', date)::integer, ''hour'', hour)) as value                       
                    FROM meteo.surf_hour
				    WHERE 
                           station = %L AND station_type = %s 						
                           AND descrname = ANY(%L)',
				    station_, station_type_, descrname_);
					
 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' and date >= '' ' || startdt_ || ' '' ';
  END IF;

  IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' and date <= '' ' || enddt_ || ' '' ';
  END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' and date_part(''month'', date) = ' || month_;
 END IF;
    
 --RAISE notice '- %', query_ ;
RETURN QUERY EXECUTE query_ || 'GROUP BY descrname, month 
								  ORDER BY descrname, month';

END;
$$;


ALTER FUNCTION meteo.get_surf_all(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_surf_avg_month(text, integer, text[], integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_surf_avg_month(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) RETURNS TABLE(descrname text, month integer, value real[])
    LANGUAGE plpgsql
    AS $$DECLARE

  query_ text;
  
BEGIN

  query_ = FORMAT( 'SELECT descrname, date_part(''month'', date)::integer as month, 				     
                      	array_agg(value) as value 				      
                    FROM meteo.surf_hour
				    WHERE 
                           station = %L AND station_type = %s 						
                           AND descrname = ANY(%L)',
				    station_, station_type_, descrname_);
					
 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' and date >= '' ' || startdt_ || ' '' ';
  END IF;

  IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' and date <= '' ' || enddt_ || ' '' ';
  END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' and date_part(''month'', date) = ' || month_;
 END IF;
    
 --RAISE notice '- %', query_ ;
RETURN QUERY EXECUTE query_ || 'GROUP BY descrname, month 
								  ORDER BY descrname, month';

END;
$$;


ALTER FUNCTION meteo.get_surf_avg_month(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_surf_day(text, integer, text[], integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_surf_day(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) RETURNS TABLE(descrname text, month integer, year integer, value real[], day real[])
    LANGUAGE plpgsql
    AS $$
DECLARE

  query_ text;
  
BEGIN

--SELECT descrname, date_part('month', date)::integer as month, date_part('year', date)::integer as year,
--        array_agg(value) as value, array_agg(date_part('day', date)::integer) as day 
 --                   FROM meteo.surf_day WHERE 
  --                         station = '26063' AND station_type = 0					
  --                         AND descrname=ANY('{ff, R}')
--						   AND date_part('month', date) = '7'
--						   AND date >= '2020-07-01 00:00:00'
--						   AND date <= '2020-08-06 00:00:00'
--					GROUP BY descrname, month, year
--			        ORDER BY descrname, month, year
					

  query_ = FORMAT( 'SELECT descrname, date_part(''month'', date)::integer as month, 
				      date_part(''year'', date)::integer as year,
                      array_agg(value) as value, 
				      array_agg(date_part(''day'', date)::real) as day 
                    FROM meteo.surf_day
				    WHERE 
                           station = %L AND station_type = %s 						
                           AND descrname = ANY(%L)',
				    station_, station_type_, descrname_);
					
 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' and date >= '' ' || startdt_ || ' '' ';
  END IF;

  IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' and date <= '' ' || enddt_ || ' '' ';
  END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' and date_part(''month'', date) = ' || month_;
 END IF;
    
 --RAISE notice '- %', query_ ;
RETURN QUERY EXECUTE query_ || 'GROUP BY descrname, month, year 
								  ORDER BY descrname, month, year';

END;
$$;


ALTER FUNCTION meteo.get_surf_day(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_surf_decade(text, integer, text[], integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_surf_decade(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) RETURNS TABLE(descrname text, month smallint, value jsonb)
    LANGUAGE plpgsql
    AS $$
DECLARE

  query_ text;
  
BEGIN

							
-- SELECT descrname, month, 
--            jsonb_agg(json_build_object(''value'', value, ''decade'', decade, ''year'', year)) as value
                
--                     FROM meteo.surf_month WHERE 
--                            station = '26063' AND station_type = 0					
--                            AND descrname=ANY('{ff, R}')
-- 						  -- AND date_part('month', date) = '7'
-- 						  AND to_date(CONCAT(year, '/', month), 'YYYY/MM') >= '2020-07-01 00:00:00'
-- 						  AND to_date(CONCAT(year, '/', month), 'YYYY/MM') <= '2020-08-06 00:00:00'
-- 					GROUP BY descrname, month
-- 			        ORDER BY descrname, month
					

  query_ = FORMAT( 'SELECT descrname, month, 
				 jsonb_agg(json_build_object(''value'', value, ''decade'', decade, ''year'', year)) as value
                   
                    FROM meteo.surf_decade
				    WHERE 
                           station = %L AND station_type = %s 						
                           AND descrname = ANY(%L)',
				    station_, station_type_, descrname_);
					
 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' and to_date(CONCAT(year, ''/'', month), ''YYYY/MM'') >= '' ' || startdt_ || ' '' ';
  END IF;

  IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' and to_date(CONCAT(year, ''/'', month), ''YYYY/MM'') <= '' ' || enddt_ || ' '' ';
  END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' and month = ' || month_;
 END IF;
    
 --RAISE notice '- %', query_ ;
RETURN QUERY EXECUTE query_ || 'GROUP BY descrname, month 
								  ORDER BY descrname, month';

END;
$$;


ALTER FUNCTION meteo.get_surf_decade(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_surf_month(text, integer, text[], integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_surf_month(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) RETURNS TABLE(descrname text, month integer, hour smallint, value real[])
    LANGUAGE plpgsql
    AS $$DECLARE

  query_ text;
  
BEGIN

  query_ = FORMAT( 'SELECT descrname, date_part(''month'', date)::integer as month, 
				      hour,
                      array_agg(value) as value 				      
                    FROM meteo.surf_hour
				    WHERE 
                           station = %L AND station_type = %s 						
                           AND descrname = ANY(%L)',
				    station_, station_type_, descrname_);
					
 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' and date >= '' ' || startdt_ || ' '' ';
  END IF;

  IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' and date <= '' ' || enddt_ || ' '' ';
  END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' and date_part(''month'', date) = ' || month_;
 END IF;
    
 --RAISE notice '- %', query_ ;
RETURN QUERY EXECUTE query_ || 'GROUP BY descrname, month, hour 
								  ORDER BY descrname, month, hour';

END;
$$;


ALTER FUNCTION meteo.get_surf_month(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_surf_year(text, integer, text[], integer, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_surf_year(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) RETURNS TABLE(descrname text, month smallint, value jsonb)
    LANGUAGE plpgsql
    AS $$
DECLARE

  query_ text;
  
BEGIN

							
-- SELECT descrname, month, 
--           json_agg(json_build_object('value', value, 'year', year, 'hour', hour)) as value
--                     FROM meteo.surf_month WHERE 
--                            station = '26063' AND station_type = 0					
--                            AND descrname=ANY('{ff, R}')
-- 						  -- AND date_part('month', date) = '7'
-- 						  AND to_date(CONCAT(year, '/', month), 'YYYY/MM') >= '2020-07-01 00:00:00'
-- 						  AND to_date(CONCAT(year, '/', month), 'YYYY/MM') <= '2020-08-06 00:00:00'
-- 					GROUP BY descrname, month
-- 			        ORDER BY descrname, month
					

  query_ = FORMAT( 'SELECT descrname, month, 
				 
                    jsonb_agg(json_build_object(''value'', value, ''year'', year, ''hour'', hour)) as value
                    FROM meteo.surf_month
				    WHERE 
                           station = %L AND station_type = %s 						
                           AND descrname = ANY(%L)',
				    station_, station_type_, descrname_);
					
 IF (startdt_ IS NOT NULL) THEN
    query_ = query_ || ' and to_date(CONCAT(year, ''/'', month), ''YYYY/MM'') >= '' ' || startdt_ || ' '' ';
  END IF;

  IF (enddt_ IS NOT NULL) THEN
    query_ = query_ || ' and to_date(CONCAT(year, ''/'', month), ''YYYY/MM'') <= '' ' || enddt_ || ' '' ';
  END IF;
					
 IF (month_ IS NOT NULL) THEN
   query_ = query_ || ' and month = ' || month_;
 END IF;
    
 --RAISE notice '- %', query_ ;
RETURN QUERY EXECUTE query_ || 'GROUP BY descrname, month 
								  ORDER BY descrname, month';

END;
$$;


ALTER FUNCTION meteo.get_surf_year(station_ text, station_type_ integer, descrname_ text[], month_ integer, startdt_ timestamp without time zone, enddt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: station_update(text, integer, integer, timestamp without time zone, timestamp without time zone, real, public.geometry, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.station_update(station_ text, station_type_ integer, cltype_ integer, dtbegin_ timestamp without time zone, dtend_ timestamp without time zone, utc_ real, location_ public.geometry, name_ru_ text, name_en_ text, country_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
 
 
BEGIN
 
	INSERT INTO meteo.stations(station, type, cltype,  dtbegin, dtend, utc, location, name_ru, name_en, country)
   	VALUES (station_, station_type_, cltype_, dtbegin_, dtend_, utc_, location_, name_ru_, name_en_, country_)
   	ON CONFLICT ON CONSTRAINT stations_pkey 
		DO UPDATE SET dtbegin = LEAST(excluded.dtbegin, meteo.stations.dtbegin),
				 	   dtend = GREATEST(excluded.dtend, meteo.stations.dtend);

END;
$$;


ALTER FUNCTION meteo.station_update(station_ text, station_type_ integer, cltype_ integer, dtbegin_ timestamp without time zone, dtend_ timestamp without time zone, utc_ real, location_ public.geometry, name_ru_ text, name_en_ text, country_ integer) OWNER TO postgres;

--
-- Name: update_surf_day_bitsum(text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_surf_day_bitsum(descrname_ text[]) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  rec RECORD;
  insert_table_name_ TEXT;
  select_table_name_ TEXT;
  constraint_pkey_ TEXT;
  query_ TEXT;

BEGIN

  -- объединить явления для Wvko
  FOR rec IN SELECT date_part('year', dt) AS dt_year FROM meteo.surf_accumulation GROUP BY dt_year
  LOOP
    SELECT FORMAT('meteo.surf_day_%s', rec.dt_year),FORMAT('meteo.surf_accumulation_%s', rec.dt_year), FORMAT('surf_day_%s_pkey', rec.dt_year)
	  INTO insert_table_name_, select_table_name_, constraint_pkey_;
    query_ = FORMAT('WITH s AS (SELECT station, station_type,  descrname, dt::date, BIT_AND(value::integer)
		       FROM %s WHERE descrname = ANY(%L)
		       GROUP BY dt::date, station, station_type, descrname)
	           INSERT INTO %s SELECT * FROM s 
       	       ON CONFLICT ON CONSTRAINT %s 
	           DO UPDATE SET value = excluded.value WHERE excluded.value > %s.value',
					  select_table_name_, descrname_, insert_table_name_, constraint_pkey_, insert_table_name_);
	  EXECUTE query_;	
  END LOOP;

END;
$$;


ALTER FUNCTION meteo.update_surf_day_bitsum(descrname_ text[]) OWNER TO postgres;

--
-- Name: update_surf_day_max(text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_surf_day_max(descrname_ text[]) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  rec RECORD;
  insert_table_name_ TEXT;
  select_table_name_ TEXT;
  constraint_pkey_ TEXT;
  query_ TEXT;

BEGIN

-- max для ff и dd
  FOR rec IN SELECT date_part('year', dt) AS dt_year FROM meteo.surf_accumulation GROUP BY dt_year
  LOOP
    SELECT FORMAT('meteo.surf_day_%s', rec.dt_year),FORMAT('meteo.surf_accumulation_%s', rec.dt_year), FORMAT('surf_day_%s_pkey', rec.dt_year)
	  INTO insert_table_name_, select_table_name_, constraint_pkey_;
	query_ = FORMAT('WITH s AS (SELECT station, station_type,  descrname, dt::date, MAX(value)  
		   FROM %s WHERE descrname = ANY(%L)
		   GROUP BY dt::date, station, station_type, descrname)
	  INSERT INTO %s SELECT * FROM s 
	  ON CONFLICT ON CONSTRAINT %s 
	  DO UPDATE SET value = excluded.value WHERE excluded.value > %s.value',
					  select_table_name_, descrname_, insert_table_name_, constraint_pkey_, insert_table_name_);
    EXECUTE query_;	
  END LOOP;

END;

$$;


ALTER FUNCTION meteo.update_surf_day_max(descrname_ text[]) OWNER TO postgres;

--
-- Name: update_surf_decade_avg(text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_surf_decade_avg(descrname_ text[]) RETURNS void
    LANGUAGE plpgsql
    AS $$DECLARE

BEGIN

-- avg по декадам месяца для sss
		   
WITH s AS (SELECT station, station_type, 
		   date_part('year', dt), date_part('month', dt),
		   CASE WHEN date_part('day', dt) <= 10 THEN 1
				WHEN date_part('day', dt) <= 20 THEN 2
				WHEN date_part('day', dt) <  32 THEN 3
				END AS decade, 
		   descrname, AVG(value)
		   FROM meteo.surf_accumulation WHERE descrname = ANY(descrname_)
		   GROUP BY date_part('year', dt), date_part('month', dt), decade, station, station_type, descrname)
	INSERT INTO meteo.surf_decade SELECT * FROM s 
	ON CONFLICT ON CONSTRAINT surf_decade_pkey 
	DO UPDATE SET value = excluded.value WHERE excluded.value > meteo.surf_decade.value;		   

END;

$$;


ALTER FUNCTION meteo.update_surf_decade_avg(descrname_ text[]) OWNER TO postgres;

--
-- Name: update_surf_month_avg(text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_surf_month_avg(descrname_ text[]) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  rec RECORD;
  insert_table_name_ TEXT;
  select_table_name_ TEXT;
  constraint_pkey_ TEXT;
  query_ TEXT;

BEGIN

-- avg по месяцам
--'ff', 'U', 'P0'
  FOR rec IN SELECT date_part('year', dt) AS dt_year FROM meteo.surf_accumulation GROUP BY dt_year
  LOOP
    SELECT FORMAT('meteo.surf_month_%s', rec.dt_year),FORMAT('meteo.surf_accumulation_%s', rec.dt_year), FORMAT('surf_month_%s_pkey', rec.dt_year)
	  INTO insert_table_name_, select_table_name_, constraint_pkey_;

      query_ = FORMAT('WITH s AS (SELECT station, station_type, 
	     	date_part(''year'', dt), date_part(''month'', dt), date_part(''hour'', dt),	  
	    	descrname, AVG(value)
		    FROM %s WHERE descrname = ANY(%L)
		    GROUP BY date_part(''year'', dt), date_part(''month'', dt), date_part(''hour'', dt), station, station_type, descrname)
	        INSERT INTO %s SELECT * FROM s 
	        ON CONFLICT ON CONSTRAINT %s 
            DO UPDATE SET value = excluded.value WHERE excluded.value > %s.value',
					  select_table_name_, descrname_, insert_table_name_, constraint_pkey_, insert_table_name_);
	  EXECUTE query_;
  END LOOP;
	  

END;
$$;


ALTER FUNCTION meteo.update_surf_month_avg(descrname_ text[]) OWNER TO postgres;

--
-- Name: update_surf_month_daycnt(text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_surf_month_daycnt(descrname_ text[]) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  rec RECORD;
  insert_table_name_ TEXT;
  select_table_name_ TEXT;
  constraint_pkey_ TEXT;
  query_ TEXT;

BEGIN

-- число дней с осадками
  FOR rec IN SELECT date_part('year', dt) AS dt_year FROM meteo.surf_accumulation GROUP BY dt_year
  LOOP
    SELECT FORMAT('meteo.surf_month_%s', rec.dt_year),FORMAT('meteo.surf_accumulation_%s', rec.dt_year), FORMAT('surf_month_%s_pkey', rec.dt_year)
	  INTO insert_table_name_, select_table_name_, constraint_pkey_;

    query_ = FORMAT('WITH s AS (SELECT station, station_type, 
             date_part(''year'', dt), date_part(''month'', dt),	0,	  
             ''YR01'', count(*) as val
             FROM %s WHERE descrname = ANY(%L)
		     GROUP BY date_part(''year'', dt), date_part(''month'', dt), station, station_type)
	         INSERT INTO %s SELECT * FROM s 
	         ON CONFLICT ON CONSTRAINT %s 
	         DO UPDATE SET value = excluded.value WHERE excluded.value > %s.value',
					  select_table_name_, descrname_, insert_table_name_, constraint_pkey_, insert_table_name_);
	EXECUTE query_;
  END LOOP;

END;
$$;


ALTER FUNCTION meteo.update_surf_month_daycnt(descrname_ text[]) OWNER TO postgres;

--
-- Name: update_surf_month_sum(text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_surf_month_sum(descrname_ text[]) RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
  rec RECORD;
  insert_table_name_ TEXT;
  select_table_name_ TEXT;
  constraint_pkey_ TEXT;
  query_ TEXT;

BEGIN

-- сумма осадков
  FOR rec IN SELECT date_part('year', dt) AS dt_year FROM meteo.surf_accumulation GROUP BY dt_year
  LOOP
    SELECT FORMAT('meteo.surf_month_%s', rec.dt_year),FORMAT('meteo.surf_accumulation_%s', rec.dt_year), FORMAT('surf_month_%s_pkey', rec.dt_year)
	  INTO insert_table_name_, select_table_name_, constraint_pkey_;


    query_ = FORMAT('WITH s AS (SELECT station, station_type, 
		     date_part(''year'', dt), date_part(''month'', dt),	0,	  
		   	 descrname, SUM(value) as val
		     FROM %s WHERE descrname = ANY(%L)
		     GROUP BY date_part(''year'', dt), date_part(''month'', dt), station, station_type, descrname)
	         INSERT INTO %s SELECT * FROM s 
             ON CONFLICT ON CONSTRAINT %s 
	         DO UPDATE SET value = excluded.value WHERE excluded.value > %s.value',
					  select_table_name_, descrname_, insert_table_name_, constraint_pkey_, insert_table_name_);
	EXECUTE query_;
  END LOOP;

END;
$$;


ALTER FUNCTION meteo.update_surf_month_sum(descrname_ text[]) OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

