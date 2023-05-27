SELECT * FROM global.delete_data_expired(
	@msgTable@,
	@msgFsField@,
	@dtField@,
	@dt@,
	@dtWriteField@,
	@dtWrite@,
	@fsTable@,
	@fsField@
) AS (
  msg_part_count integer,
  msg_rec_count  bigint,
  fs_part_count  integer,
  fs_rec_count   bigint
);
