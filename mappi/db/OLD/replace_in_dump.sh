#!/bin/bash

STR="OWNER TO maria"
TO="OWNER TO postgres"
find . -name "*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g"

STR="OWNER TO maxim"
TO="OWNER TO postgres"
find . -name "*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g" 

STR="OWNER TO gotur"
TO="OWNER TO postgres"
find . -name "*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g"

STR="OWNER TO marina"
TO="OWNER TO postgres"
find . -name "*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g"

STR="SET row_security = off;"
TO=" "
find . -name "*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g"

STR="SET idle_in_transaction_session_timeout = 0;"
TO=" "
find . -name "*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g"

STR='\$function\$'
TO='\$function\$;'
find . -name "update*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g"

STR='AS \$function\$;'
TO='AS \$function\$'
find . -name "update*.sql" -exec grep "$STR" --with-filename  {} \; | awk -F: '{ print $1}' | uniq | xargs -n1 sed -i "s/$STR/$TO/g"


