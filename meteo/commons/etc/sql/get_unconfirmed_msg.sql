SELECT * FROM meteo.msg WHERE
        (confirm IS NULL OR confirm = FALSE)
        AND external = FALSE
        AND dddd IS NOT NULL
			  AND cccc = @cccc@
        AND msg_dt >= @dt@
			  AND t1 = 'A'
			  AND t2 = 'A';
