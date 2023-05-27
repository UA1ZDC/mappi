function() {
  var date = this._id.dt;
  date.setUTCDate(1); 
  date.setUTCMinutes(0); 
  date.setUTCSeconds(0);
  var key = {
    station : this._id.station, 
    station_type : NumberInt(this._id.station_type),
    date : date,
    descrname : this._id.descrname
  };       

  emit(key, { value : this.value, count : 1}); 
}
