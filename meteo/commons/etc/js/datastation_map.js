function() {
   var key, value;
   if (this.isComplex === 'true') {
   key = {
        level: this.level,
        level_type: this.level_type,
        location: this.location
    };
    value = {
      descrs: [ {descr: this.param[0].descr, value: this.param[0].value, qual: this.param[0].qual},
                {descr: this.param[1].descr, value: this.param[1].value, qual: this.param[1].qual}]
    };
   }
   else {
       key= {
            level: this.level,
            level_type: this.level_type,
            location: this.location
        };
        value = {
          descrs: [ {descr: this.descr, value: this.param.value, qual: this.param.qual}]
        };
   }

emit(key,value);
}
