function(key, values) {
  var dates = [];
  
  values.forEach(function(val) {
    var one = val['date'][0];
    if (!dates.includes(one)) {
      dates.push(one); 
    }
  });
  
  return { date : dates };
}
