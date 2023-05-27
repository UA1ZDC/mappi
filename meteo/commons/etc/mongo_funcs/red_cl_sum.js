function(key, values) {
  var sum = 0;  

  values.forEach(function(val) {
    if (val > -9999) {
      sum += val;
    }
  });
  
  return sum;
}
