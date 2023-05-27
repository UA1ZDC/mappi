function() {
emit(
    {
        level: this.level,
        level_type: this.level_type,
        location: this.location
    },
    {
      params: [ {descr: this.descr, value: this.param.value, qual: this.param.qual, code: this.code}]
    });
}
