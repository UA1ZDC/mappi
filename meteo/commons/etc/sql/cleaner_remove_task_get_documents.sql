SELECT * FROM global.cleaner_remove_task_get_documents(
  @collection@,
  @dtField@,
  @enddt@,  
  @dtWriteField@,
  @storeDt@,
  @gridField@
) AS
(
  id bigint,
  fileid text
);
