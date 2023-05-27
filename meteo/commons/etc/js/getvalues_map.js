function() {
    emit(
        {
            id: this._id,
            location: this.location
        },
        {
           value: this.param.value,
           qual: this.param.qual,
        });

}
