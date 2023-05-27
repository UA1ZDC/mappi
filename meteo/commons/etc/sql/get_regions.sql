SELECT * from meteo.get_regions( @id@,
                                  @name@,
                                  @skip@,
                                  @limit@,
                                  @query@ ) as 
(
    id                    integer,
    title                 text,
    region_center_point   jsonb,
    region_coords         jsonb
);
