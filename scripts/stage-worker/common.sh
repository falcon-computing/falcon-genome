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

# log messge
log_msg() {
  local level=$1;
  local msg=$2;
  local v=$verbose;
  if [[ "$v" == "" ]]; then
    local v=1;
  fi;
  if [ "$level" -le "$v" ]; then
    (>&2 echo "[fcs-genome $stage_name] $msg");
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
  eval "echo "1" > ${filename}.tmp" &>/dev/null ;
  if [ ! -f ${filename}.tmp ]; then
    log_error "Cannot write to folder $(dirname $filename)"
    exit 1
  fi;
  rm ${filename}.tmp;

  # Check if output already exists
  if [ -f $filename ]; then
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
      log_warn "Overwriting output file $filename"
    fi
    rm $filename
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

check_ret() {
  local ret=$?;
  local msg=$1;
  if [ $ret -ne 0 ]; then
    log_error "$1 failed";
    exit -1
  fi;
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

# Setup interval lists for partitions
setup_intv() {
  if [ $# -lt 2 ]; then
    log_internal;
    return 1;
  fi
  local nparts=$1;
  local ref=$2;
  if [ ! -d "$PWD/.fcs-genome/intv_$nparts" ]; then
    $DIR/intvGen.sh -r $ref -n $nparts;
  fi;
}

get_sample_id() {
  local input_dir=$1;
  local sample_id=`ls $input_dir/*.gvcf | sed -e 'N;s/^\(.*\).*\n\1.*$/\1\n\1/;D'`;
  local sample_id=$(basename $sample_id);
  local sample_id=${sample_id%%.*}
  echo $sample_id;
}

# Start manager
start_manager() {
  if [ ! -z "$manager_pid" ]; then
    # remove debug information 
    log_debug "Manager already running"
    return 1
  fi;
  if [ -e "host_file" ]; then
    local host_file=host_file
  else
    local host_file=$DIR/../host_file
  fi;
  log_debug "$DIR/../manager/manager --v=0 -h $host_file";
  $DIR/../manager/manager --v=0 -h $host_file &
  manager_pid=$!;
  sleep 1;
  if [[ ! $(ps -p "$manager_pid" -o comm=) =~ "manager" ]]; then
    log_debug "Cannot start manager, exiting";
    log_internal
    exit -1;
  fi;
}

# Stop manager
stop_manager() {
  if [ -z "$manager_pid" ]; then
    log_debug "Manager is not running"
    return 1
  fi;
  ps -p $manager_pid > /dev/null;
  if [ "$?" == 0 ]; then 
    kill "$manager_pid "
  fi
  manager_pid=;
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

# Catch the interrupt option to stop manager
terminate() {
  log_info "Caught interruption, cleaning up";
  stop_manager;

  # Check run dir
  for file in ${output_table[@]}; do
    if [ -e ${file}.pid ]; then
      local node_name=$(sed "1q;d" ${file}.pid)
      local bash_pid=$(sed "2q;d" ${file}.pid)
      #echo "kill $bash_pid on $node_name for $file "
      log_debug "kill $bash_pid on $node_name for $file "
      # kill the remote process
      ssh $node_name "kill $bash_pid 2>/dev/null"
      rm ${file}.pid -f
      log_debug "Stopped remote printReads process $bash_pid for $file"
    fi;
  done;

  # Stop stray processes
  for pid in ${pid_table[@]}; do
    terminate_process "$pid"
  done;

  exit 1;
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
