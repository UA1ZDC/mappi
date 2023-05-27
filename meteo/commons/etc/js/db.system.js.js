db.system.js.save({
        "_id" : "existsDescrInObject",
        "value" : function (obj, descr) { if(typeof(obj[descr]) != 'undefined') { return true; }  return false; }
});
db.system.js.save({
        "_id" : "existsDescrInArray",
        "value" : function (arr, descr) { for(var i = 0; i < arr.length; ++i) { if(typeof(arr[i][descr]) != 'undefined') { return true; } else if(typeof(arr[i].data) != 'undefined') { if(existsDescrInArray(arr[i].data, descr)) { return true; } } } return false; }
});
db.system.js.save({
        "_id" : "getDescrFromArray",
        "value" : function (arr, descr, dest) { for(var i = 0; i < arr.length; ++i) { if(typeof(arr[i][descr]) != 'undefined') { dest.push(arr[i][descr]); } if(typeof(arr[i].data) != 'undefined') { getDescrFromArray(arr[i].data, descr, dest); } } }
});
db.system.js.save({
        "_id" : "getDescrFromObject",
        "value" : function (obj, descr, dest) { if(typeof(obj[descr]) != 'undefined') { dest.push(obj[descr]); } if(typeof(obj.data) != 'undefined') { getDescrFromArray(obj.data, descr, dest); } }
});
