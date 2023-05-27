SELECT insert_journal as "ok" 
  from meteo.insert_journal (
    @priority@,
    @sender@,
    @username@,
    @filename@,
    @linenumber@,
    @ip@,
    @host@,
    @dt@,
    @message@
);

