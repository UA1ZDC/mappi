function(key, values) {
  var sum = 0;  
  values.forEach(function(val) {
    sum |= val;
  });
  return sum;
}
