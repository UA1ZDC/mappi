function(key, values) {
  var max = -9999;   
  values.forEach(function(val) {
    if (max < val) {
      max = val;
    }
  });
  
  return max;
}
