# Reliably assign to function input argument
upvar() {
  if unset -v "$1"; then       # Unset & validate varname
    if (( $# == 2 )); then
      eval $1=\"\$2\"          # Return single value
    else
      eval $1=\(\"\${@:2}\"\)  # Return array
    fi
  fi;
}

start_manager() {
  local bin_name=$1;
  local pid_fname="/tmp/.${bin_name}.pid";
  local log_dir=$(pwd)/log;
  if [ -f $pid_fname ]; then
    echo "$bin_name is already started"
    return
  fi;
  mkdir -p $log_dir;
  $DIR/$bin_name \
    -i "$log_dir/${bin_name}.log" \
    -d $run_dir &

  local pid=$!;
  echo $! > $pid_fname;
  
  if [[ ! $(ps -p "$pid" -o cmd=) =~ "$bin_name" ]]; then                                        
     echo "failed to start $bin_name";
     rm -f $pid_fname;
  else
     echo "$bin_name is started";
  fi;
}

stop_manager() {
  local bin_name=$1;
  local pid_fname="/tmp/.${bin_name}.pid";

  if [ -f $pid_fname ]; then
    pid=`cat $pid_fname`;
    rm -f $pid_fname;
    if [ -z "$pid" ]; then
      echo "no $bin_name is running";
    else 
      kill $pid;
      echo "$bin_name is stopped";
    fi;
  else
    echo "no $bin_name is running";
  fi;
}

# log messge
log_msg() {
  local level=$1;
  local msg=$2;
  local v=$verbose;
  local handle=$(date +"%F %T")" [$(basename $0)]";
  if [[ "$v" == "" ]]; then
    local v=1;
  fi;
  if [ "$level" -le "$v" ]; then
    if [ -z "$log_file" ]; then
      (>&2 echo "$handle $msg");
    else
      echo "$handle $msg" >> $log_file
    fi; 
  fi;
}

log_info() {
  log_msg 2 "INFO: $1";
}

log_warn() {
  log_msg 1 "WARNING: $1";
}

log_error() {
  log_msg 0 "ERROR: $1";
}

log_debug() {
  if [ ! -z "$DEBUG" ]; then
    log_msg 0 "$1";
  fi;
}

log_internal() {
  log_error "Encountered an internal error"
  log_error "Please contact technical support at support@falcon-computing.com";
}

# Check if arguement is present, if a default value is specified
# then it will be assigned, otherwise error will be reported
check_arg() {
  local arg=$1;
  eval "local val=\$$2";
  if [ -z $val ]; then
    if [ $# -gt 2 ]; then
      local default=$3
      local "$2" && upvar $2 $default
      log_warn "Argument '$arg' is not specified, use $default by default";
    else
      log_error "Argument '$arg' is missing, please specify."
      args_error=1
    fi;
  fi;
}

# Validate if all the arguments are checked
check_args() {
  if [ ! -z $args_error ]; then
    log_error "Failed to parse input arguments"
    print_help
    exit -1
  fi;
}

check_input() {
  local filename=$1;
  if [ ! -f $filename ]; then
    log_error "Input file $filename does not exist"
    exit 1
  fi;
}

check_output() {
  local filename=$1;

  # Check if output folder writtable
  if [ -d $filename ]; then
    local tmp_fname=${filename}/$(basename $filename).tmp
  else
    local tmp_fname=${filename}.tmp
  fi
  eval "echo "1" > $tmp_fname" &>/dev/null;
  if [ ! -f $tmp_fname ]; then
    log_error "Cannot write to folder $(dirname $filename)"
    exit 1
  fi;
  rm $tmp_fname;

  # Check if output already exists
  if [ -f $filename ] || [ -d $filename ]; then
    # Here detect a global variable that will be handled in the caller
    if [ -z "$force_flag" ]; then
      >&2 echo -e -n "[fcs-genome $stage_name] Output file $filename already exists, overwrite? \e[31m[yes|no|all]\e[0m "
      while true; do
        read answer;
        if [[ "$answer" == "yes" ]]; then
          log_warn "Overwritting $filename, run with -f option to skip this prompt."
          break
        elif [[ "$answer" == "all" ]]; then
          log_warn "Overwritting $filename, run with -f option to skip this prompt."
          force_flag=1
          break
        elif [[ "$answer" == "no" ]]; then
          exit 2
        else 
          >&2 echo -e -n "[fcs-genome $stage_name] Please type 'yes' or 'no' or 'all': "
        fi
      done
    else
      log_warn "Overwriting output $filename"
    fi
    rm -rf $filename
  fi;
}

# Check if output dir exists and is writable
check_output_dir() {
  dir=$1;
  if [[ ! -d "$dir" ]]; then
    log_error "Output dir $dir does not exists";
    exit 1;
  fi;
  echo "1" > $dir/.test;
  if [ ! -f $dir/.test ]; then
    log_error "Cannot write to folder $dir";
    exit 1;
  fi;
  rm $dir/.test;
}

# Create output dir if it does not exist 
create_dir() {
  local dir=$1
  if [[ ! -d "$dir" ]]; then
    mkdir -p $dir &> /dev/null;
    if [ $? -ne 0 ]; then
      log_error "Cannot create dir $dir";
      exit 1;
    fi;
    return 0;
  else
    return 1; # did not create dir
  fi;
}

get_sample_id() {
  local input_dir=$1;
  local sample_id=`ls $input_dir/*.gvcf | sed -e 'N;s/^\(.*\).*\n\1.*$/\1\n\1/;D'`;
  local sample_id=$(basename $sample_id);
  local sample_id=${sample_id%%.*}
  echo $sample_id;
}

kill_process() {
  log_info "Caught interruption, cleaning up";
  kill -5 $(jobs -p) 2> /dev/null;
  exit 1;
}

kill_task_pid() {
  log_info "kill $task_pid"
  kill $task_pid 2> /dev/null
  exit 1
}

terminate_process() {
  local pid=$1;
  if [ -z "$pid" ]; then
    return 2;
  fi
  kill "${pid}" 2> /dev/null;
  if [ "$?" -eq 0 ]; then
    log_debug "killed $pid";
  else
    return 1;
  fi
}

readlink_check() {
  eval "local val=\$$1"
  abs_path=$(readlink -f $val)
  if [ -z $abs_path ];then
    log_error "The directory $(dirname $val) does not exist"
    exit 1
  else
    eval "$1=$abs_path"
  fi
}
