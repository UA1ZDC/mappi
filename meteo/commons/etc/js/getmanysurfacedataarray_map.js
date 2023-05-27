function () {
  emit(
      { station: this.station, location: this.location, level_type: this.level_type, level: this.level, data_type: this.data_type },
      {
        idPtkpp: [ this.idPtkpp ],
        descr: [ { dt: this.dt, descr: this.descr, value: [ this.param.value ], qual: this.param.qual } ]
      });
}
