function () {
  var key, value;
  if(typeof(this._id.dt) === 'undefined') {
    key = { dt: this.dt, station: this.station, data_type: this.data_type, level_type: this.level_type, level: this.level };
    value = { descr: this.descr };
  }
  else {
    key = { dt: this._id.dt, station: this._id.station, data_type: this._id.data_type, level_type: this._id.level_type, level: this._id.level };
    value = { descr: this.value.descr };
  }
  emit(key, value);
}
