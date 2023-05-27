function (key, values) {
  var count = values[0];
  for(var i = 1; i < values.length; ++i) {
    count += values[i];
  }
  return count;
}
