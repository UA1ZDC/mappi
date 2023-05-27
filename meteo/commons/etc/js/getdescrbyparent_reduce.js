function(key,valuesin) {
  var val = { types: []};
  for (var i = 0; i < valuesin.length; ++i) {
      var typebuf = valuesin[i].types;
      for ( var j = 0; j < typebuf.length; ++j) {
          val.types.push(typebuf[j]);
      }
  }
  return val;
}
