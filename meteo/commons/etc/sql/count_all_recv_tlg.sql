SELECT 
  COUNT(*) as n
FROM 
  meteo.msg WHERE confirm_tlg = false AND dddd = @param@;
