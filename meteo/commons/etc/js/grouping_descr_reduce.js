function (key, values) {
  var value = { descr: [] };
  for(var i = 0; i < values.length; ++i) {
    if(typeof(values[i].descr) === typeof(1)) {
      if(value.descr.indexOf(values[i].descr) === -1) {
        value.descr.push(values[i].descr);
      }
    }
    else {
      for(var j = 0; j < values[i].descr.length; ++j) {
        if(value.descr.indexOf(values[i].descr[j]) === -1) {
          value.descr.push(values[i].descr[j]);
        }
      }
    }
  }
  return value;
}
