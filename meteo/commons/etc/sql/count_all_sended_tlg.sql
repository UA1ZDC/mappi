SELECT 
  COUNT(*) as n
FROM
  meteo.msg WHERE confirm_tlg = false AND cccc = @param@;
