function(key,valuesin) {
  var descrsout = [];
  var values = [];
  var quality = [];
  var val = { descrs: []};
  for (var i = 0; i < valuesin.length; ++i) {
      var descrsbuf = valuesin[i].descrs;
      for ( var j = 0; j < descrsbuf.length; ++j) {
          val.descrs.push(descrsbuf[j]);
      }

  }
  return val;
}
