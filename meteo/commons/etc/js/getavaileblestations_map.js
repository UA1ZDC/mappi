function() {
emit(
    {
       station: this.station,
       location: this.location.coordinates
    },
    {
        data_type: this.data_type
    });
}
