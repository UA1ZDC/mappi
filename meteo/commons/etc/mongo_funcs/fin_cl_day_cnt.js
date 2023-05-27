function(key, reduced)
{
  var dates = [];
  
  reduced.date.forEach(function(val) {
    var one = val;
    if (!dates.includes(one)) {
      dates.push(one); 
    }
  });
  
  return dates.length;
}
