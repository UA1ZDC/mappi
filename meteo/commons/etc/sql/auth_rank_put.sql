INSERT INTO meteo.ranks(id, name) VALUES (@id@, @name@) ON CONFLICT (id) DO UPDATE SET name = EXCLUDED.name
