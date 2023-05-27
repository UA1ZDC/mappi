function() {
  var dec = NumberInt((this._id.dt.getUTCDate() - 1) / 10) + 1;
  dec = (dec > 3) ? 3 : dec;
  var date = this._id.dt;
  date.setUTCDate((dec - 1)*10 + 1);
  date.setUTCHours(0);
  date.setUTCMinutes(0); 
  date.setUTCSeconds(0);
  
  var key = {
    station : this._id.station, 
    station_type : NumberInt(this._id.station_type), 
    date: date,
    decade: NumberInt(dec),
    descrname : this._id.descrname
 };       
  
  emit(key, { value : this.value, count : 1}); 
}
