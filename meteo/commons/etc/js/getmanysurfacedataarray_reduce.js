function (key, values) {
  var value = { idPtkpp: [], descr: [] };
  for(var i = 0; i < values.length; ++i) {
    value.idPtkpp.concat(values[i].idPtkpp);
    value.descr.concat(values[i].descr);
  }
  return result;
}
