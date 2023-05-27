function (key, value) {
  var has_descr = [];
  var descr = [];
  for(var i = 0; i < value.descr.length; ++i) {
    var index = has_descr.indexOf(value.descr[i]);
    if(index == -1) {
      has_descr.push(value.descr[i].descr);
      descr.push(value.descr[i]);
    }
    else {
      if(value[i].descr.qual >= descr[index].qual) {
        if(query_type === 0 && value.descr[i].value < descr[index].value) { /* maximum */
          descr[index] = value.descr;
        }
        else if(query_type === 1 && value.descr[i].value > descr[index].value) { /* minimum */
          descr[index] = value.descr;
        }
        else if(query_type === 2 && value.descr[i].dt > descr[index].dt) { /* last */
          descr[index] = value.descr;
        }
        else if(query_type == 3 && Math.abs(value.descr[i].dt - dt) < Math.abs(descr[index].dt)) { /* nearest */
          descr[index] = value.descr;
        }
        else if(query_type == 4) { /* average */
          descr[index] += value.descr;
          if(typeof(descr[index].count) !== 'undefined') {
            descr[index].count++;
          }
          else {
            descr[index] = 1;
          }
        }
      }
    }
  }
  if(query_type === 4) {
    for(var i = 0; i < descr.length; ++i) {
      if(typeof(descr[i].count) !== 'undefined') {
        descr[i].value /= descr[i].count;
        delete descr[i].count;
      }
    }
  }
  value.descr = descr;
  return value;
}
