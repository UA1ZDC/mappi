function(key, values) {
  var sum = 0;  
  var count = 0;
  
  values.forEach(function(val) {
    if (val['value'] > -9999) {
      sum += val['value'];
      count += val['count'];
    }
  });
  
  return {value : sum, count : NumberInt(count)};
}
