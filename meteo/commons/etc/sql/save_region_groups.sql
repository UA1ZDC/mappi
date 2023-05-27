DO $$
DECLARE 
    regiongroup_id_seq bigint;
BEGIN
    -- начало транзакции
    -- BEGIN;

    -- добавляем в таблицу данные по группе регионов
    -- в случае, если такая группа уже есть - обновляем поля
    INSERT INTO meteo.forecast_regiongroup("id", "title", "description", "is_active")
                    VALUES( @id@, @title@, @description@, @is_active@ )
                    ON CONFLICT (id) DO
                    UPDATE SET title=@title@, description=@description@, is_active=@is_active@ 
                    RETURNING id INTO regiongroup_id_seq;


    -- удаляем все связанные записи из таблицы связей
    
    DELETE FROM meteo.forecast_region_to_regiongroup 
        WHERE regiongroup_id = regiongroup_id_seq;

    -- после вставки/обновления/удаления создаем новые связи с регионами/станциями
    IF @regions_id@ IS NOT NULL THEN
        INSERT INTO meteo.forecast_region_to_regiongroup("regiongroup_id", "region_id")
            SELECT regiongroup_id_seq,r FROM unnest( @regions_id@ ) r;
    END IF;

    IF @stations@ IS NOT NULL THEN
        INSERT INTO meteo.forecast_region_to_regiongroup("regiongroup_id", "station_id")
            SELECT regiongroup_id_seq, s.id FROM unnest(  @stations@, @stations_types@ ) x(code,sttype)
			left join meteo.stations as s on s.station = code and s.station_type=sttype;
    END IF;

    -- завершение транзакции
    -- COMMIT;

END$$;