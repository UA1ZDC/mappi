rm res_loc.txt

echo "\"NOAA 15\"" >> res_loc.txt; 
echo "Город Утро День Вечер Ночь Всего" >> res_loc.txt; 
while read line; do ./test "NOAA 15" $line >> res_loc.txt; done < file.txt; 
echo >> res_loc.txt

echo "\"NOAA 18\"" >> res_loc.txt; 
echo "Город Утро День Вечер Ночь Всего" >> res_loc.txt; 
while read line; do ./test "NOAA 18" $line >> res_loc.txt; done < file.txt; 
echo >> res_loc.txt

echo "\"NOAA 19\"" >> res_loc.txt; 
echo "Город Утро День Вечер Ночь Всего" >> res_loc.txt; 
while read line; do ./test "NOAA 19" $line >> res_loc.txt; done < file.txt; 
echo >> res_loc.txt

echo "\"METEOR-M 2\"" >> res_loc.txt; 
echo "Город Утро День Вечер Ночь Всего" >> res_loc.txt; 
while read line; do ./test "METEOR-M 2" $line >> res_loc.txt; done < file.txt; 
echo >> res_loc.txt

exit 0