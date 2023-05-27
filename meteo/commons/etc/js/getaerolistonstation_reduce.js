function(key,valuesin) {
  var val = { dt: []};
  for (var i = 0; i < valuesin.length; ++i) {
      var dtbuf = valuesin[i].dt;
      for ( var j = 0; j < dtbuf.length; ++j) {
          val.dt.push(dtbuf[j]);
      }
  }
  return val;
}
