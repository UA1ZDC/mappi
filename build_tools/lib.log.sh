# ---------------------------------------------------------------------------------------------------------------------
# GLOBAL VARIABLES
# ---------------------------------------------------------------------------------------------------------------------
export var_LOG_FILE='/tmp/log.log'
export var_LOG_STAGE=''


# ---------------------------------------------------------------------------------------------------------------------
function f_init_log()
{
  var_LOG_FILE=$1
  echo > "$var_LOG_FILE"
  chmod a+rw "$var_LOG_FILE"
}


# ---------------------------------------------------------------------------------------------------------------------
function f_command_log()
{
  echo -e ${c_command}${@}${c_def}
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} ${@}" >> "$var_LOG_FILE"
}


# ---------------------------------------------------------------------------------------------------------------------
function f_begin_stage_log()
{
  var_LOG_STAGE=$1
  echo -e ${c_stage}${@}${c_def}
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} ${@}" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_end_stage_log()
{
  echo "" >> "$var_LOG_FILE"
  var_LOG_STAGE=''
  return $@
}

# ---------------------------------------------------------------------------------------------------------------------
function f_progress_log()
{
  echo "${@}"
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} ${@}" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_file_log()
{
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} ${@}" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_error_log()
{
  echo -e ${c_err}${@}${c_def}
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} ${@}" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_warning_log()
{
  echo -e ${c_wrn}${@}${c_def}
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} ${@}" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_ok_status_log()
{
  echo -e "[${c_ok}${1}${c_def}]  ${2}"
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} [${1}]  ${2}" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_fail_status_log()
{
  echo -e "[${c_err}${1}${c_def}]  ${2}"
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} [${1}]  ${2}" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_status_log()
{
  local text=$1
  local res=$2
  local ok_text=$3
  local fail_text=$4
  
  if [[ -z $ok_text ]]; then ok_text=' ok '; fi
  if [[ -z $fail_text ]]; then fail_text='fail'; fi
  
  if [[ "0" == "$res" ]]
  then
    echo -e "[${c_ok}${ok_text}${c_def}]  $text"
  else
    echo -e "[${c_err}${fail_text}${c_def}]  $text"
  fi    
  
  
  local time=$( date +'%Y-%m-%d %R:%S' )
  echo "${time} [${status}]  $text" >> "$var_LOG_FILE"
}

# ---------------------------------------------------------------------------------------------------------------------
function f_print_var()
{
  local name=$1
  local value="${!1}"
  if [[ "x" == "x$value" ]]; then value="< не задано >"; fi
  
  echo "$name: $value"
}







