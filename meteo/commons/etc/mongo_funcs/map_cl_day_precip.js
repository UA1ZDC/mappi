function() {
  if (this.value != 0) {
    var day = this._id.dt.getUTCDate();
    var date = this._id.dt; 
    date.setUTCDate(1); 
    date.setUTCHours(0); 
    date.setUTCMinutes(0); 
    date.setUTCSeconds(0);  
    
    var key = { 
      station : this._id.station, 
      station_type : NumberInt(this._id.station_type), 
      date : date, 
      descrname : 'YR01' };      
    
    emit(key, { date : [ day ]});
  }
}
