function(key,valuesin) {
    var val = { params: []};
    var flag = 1;
    for (var i = 0; i < valuesin.length; ++i) {
        var paramsbuf = valuesin[i].params;
        flag = 1;
        for (var k = 0; k < i; ++k ) {
            var ifbuf = valuesin[k].params;
            if (paramsbuf[0].descr === ifbuf[0].descr ) {
                flag = 0;
            }
        }
        if (flag == 1) {
            for ( var j = 0; j < paramsbuf.length; ++j) {
                val.params.push(paramsbuf[j]);
            }
        }
    }
    return val;
}
