SELECT * from meteo.get_military_district( @id@,
                                  @name@, @query@ )
                                  as (
                                  id integer,
                                  name text,
                                  center_coord jsonb,
                                  region_coords jsonb,
                                  center_name text);
                                  
--CREATE OR REPLACE FUNCTION meteo.get_military_district_curs(id_ integer[], title_ text[], query_ text)
-- RETURNS refcursor
-- LANGUAGE plpgsql
--AS $function$
--DECLARE
--  ref refcursor := 'rescursor';
--  like_str_ TEXT = CONCAT('%', query_, '%');
--BEGIN
--  OPEN ref FOR SELECT a.id,
--                      a.name,
--					  jsonb_set(ST_AsGeoJson(a.center_coord)::jsonb,'{alt}', to_jsonb(0)) as center,
--					    jsonb_agg(ST_AsGeoJSON(a.region_coords)::jsonb) as region_coords,
--					  a.center_name
--                      FROM (select id, name, center_coord, (ST_DUMP(region_coords)).geom as region_coords, center_name from  meteo.okruga 
--                  WHERE  (id_   IS NULL OR id = ANY(id_) )
--				  	AND  (title_ IS NULL OR name = ANY(title_) )
--					AND  (query_ IS NULL OR name ILIKE like_str_ )) as a group by a.id, a.name, center,a.center_name  ;
--  RETURN ref;
--END;
--$function$
--;