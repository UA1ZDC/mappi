NORAD_URL="http://celestrak.org/NORAD/elements/gp.php?GROUP=active&FORMAT=tle"
curl "$NORAD_URL" -o "$SRC/mappi/etc/weather.txt"
curl "$NORAD_URL" -o "$BUILD_DIR/var/mappi/weather.txt"