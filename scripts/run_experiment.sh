#!/bin/bash
# Get the script location and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Change to project root if not already there
if [[ "$(pwd)" != "$PROJECT_ROOT" ]]; then
    cd "$PROJECT_ROOT"
fi

if [ ! -f "build/palloc" ]; then
    echo "Error: build/palloc executable not found."
    echo "Compiling the project..."
    ./scripts/compile.sh release
    exit 1
fi

if [ ! -f "data.json" ]; then
    echo "Error: data.json not found in project root."
    exit 1
fi

show_help() {
  echo "Usage: $0 [options]"
  echo "Options:"
  echo "  -h, --help        Show help message"
  echo "  -d, --duration    Max duration in minutes of requests (can be a range: MIN-MAX), default: 600"
  echo "  -a, --arrival     Max time till arrival in minutes of requests (can be a range: MIN-MAX), default: 60"
  echo "  -r, --requests    Request rate per timestep (can be a range: MIN-MAX), default: 10.0"
  echo "  -t, --timesteps   Number of timesteps to simulate, default: 1440"
  echo "  -j, --jobs        Number of parallel jobs to run (default: number of CPU cores)"
  echo ""
  echo "Examples:"
  echo "  $0 -d 600-1200          # Run simulations in the range 600-1200 max durration"
  echo "  $0 -r 5.0-15.0          # Run simulations in the range 5.0-15.0 request rate"
  echo "  $0 -r 10-60             # Run simulations in the range 10-60 max time till arrival"
  echo "  $0 -j 4                 # Run 4 simulations in parallel"
}

# Default values
MAX_DURATION=600
DURATION_END=0
MAX_ARRIVAL=60
ARRIVAL_END=0
REQUEST_RATE=10.0
REQUEST_RATE_END=0
AGGREGATIONS=3
TIMESTEPS=1440
PARALLEL_JOBS=$(nproc)

# Set fixed step sizes
DURATION_STEP=10
ARRIVAL_STEP=10
REQUEST_RATE_STEP=0.5

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -d|--duration)
            if [[ $# -lt 2 || $2 == -* ]]; then
            echo "Error: Missing value for option $1"
            show_help
            exit 1
            fi
            
            # Check if duration is a range
            if [[ "$2" == *-* ]]; then
            MAX_DURATION="${2%%-*}" 
            DURATION_END="${2##*-}"
            else
            MAX_DURATION="$2"
            DURATION_END=0  # No range
            fi
            shift 2
            ;;
        -a|--arrival)
            if [[ $# -lt 2 || $2 == -* ]]; then
            echo "Error: Missing value for option $1"
            show_help
            exit 1
            fi
            
            # Check if arrival is a range
            if [[ "$2" == *-* ]]; then
            MAX_ARRIVAL="${2%%-*}" 
            ARRIVAL_END="${2##*-}"
            else
            MAX_ARRIVAL="$2"
            ARRIVAL_END=0  # No range
            fi
            shift 2
            ;;
        -r|--requests)
            if [[ $# -lt 2 || $2 == -* ]]; then
            echo "Error: Missing value for option $1"
            show_help
            exit 1
            fi
            
            # Check if request rate is a range
            if [[ "$2" == *-* ]]; then
            REQUEST_RATE="${2%%-*}"
            REQUEST_RATE_END="${2##*-}"
            else
            REQUEST_RATE="$2"
            REQUEST_RATE_END=0  # No range
            fi
            shift 2
            ;;
        -t|--timesteps)
            if [[ $# -lt 2 || $2 == -* ]]; then
            echo "Error: Missing value for option $1"
            show_help
            exit 1
            fi
            TIMESTEPS="$2"
            shift 2
            ;;
        -j|--jobs)
            if [[ $# -lt 2 || $2 == -* ]]; then
            echo "Error: Missing value for option $1"
            show_help
            exit 1
            fi
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

mkdir -p experiments

# Find the highest experiment number that already exists
highest_num=0
for dir in experiments/experiment-*/; do
    if [[ -d "$dir" ]]; then
        dir_name=$(basename "$dir")
        num=${dir_name#experiment-}

        # Check if it's a number
        if [[ "$num" =~ ^[0-9]+$ ]]; then
            if (( num > highest_num )); then
                highest_num=$num
            fi
        fi
    fi
done

next_num=$((highest_num + 1))
exp_dir="experiments/experiment-$next_num"
mkdir -p "$exp_dir"

# Create experiment summary file header
echo "Experiment Summary" > "${exp_dir}/summary.txt"
echo "Date: $(date)" >> "${exp_dir}/summary.txt"
echo "" >> "${exp_dir}/summary.txt"

# Count configurations and runs
duration_count=1
if [ "$DURATION_END" -gt 0 ]; then
    # Count durations in range
    duration_count=0
    current_dur=$MAX_DURATION
    while [ "$current_dur" -le "$DURATION_END" ]; do
        ((duration_count++))
        current_dur=$((current_dur + DURATION_STEP))
    done
    echo "Duration range: ${MAX_DURATION}-${DURATION_END} (step: ${DURATION_STEP})" >> "${exp_dir}/summary.txt"
else
    echo "Duration: ${MAX_DURATION}" >> "${exp_dir}/summary.txt"
fi

arrival_count=1
if [ "$ARRIVAL_END" -gt 0 ]; then
    # Count durations in range
    arrival_count=0
    current_arv=$MAX_ARRIVAL
    while [ "$current_arv" -le "$ARRIVAL_END" ]; do
        ((arrival_count++))
        current_dur=$((current_arv + ARRIVAL_STEP))
    done
    echo "Arrival range: ${MAX_ARRIVAL}-${ARRIVAL_END} (step: ${ARRIVAL_STEP})" >> "${exp_dir}/summary.txt"
else
    echo "Arrival: ${MAX_ARRIVAL}" >> "${exp_dir}/summary.txt"
fi

rate_count=1
if [ "$REQUEST_RATE_END" != "0" ]; then
    # Count rates in range
    rate_count=0
    current_rate=$REQUEST_RATE
    while (( $(echo "$current_rate <= $REQUEST_RATE_END" | bc -l) )); do
        ((rate_count++))
        current_rate=$(echo "$current_rate + $REQUEST_RATE_STEP" | bc)
    done
    echo "Request rate range: ${REQUEST_RATE}-${REQUEST_RATE_END} (step: ${REQUEST_RATE_STEP})" >> "${exp_dir}/summary.txt"
else
    echo "Request rate: ${REQUEST_RATE}" >> "${exp_dir}/summary.txt"
fi

total_configs=$((duration_count * rate_count))

echo "Total configurations: ${total_configs}" >> "${exp_dir}/summary.txt"
echo "Number of runs per configuration: ${AGGREGATIONS}" >> "${exp_dir}/summary.txt"
echo "Parallel jobs: ${PARALLEL_JOBS}" >> "${exp_dir}/summary.txt"
echo "Timesteps: ${TIMESTEPS}" >> "${exp_dir}/summary.txt"
echo "----------------------------------------" >> "${exp_dir}/summary.txt"

echo "Running ${total_configs} simulations with ${PARALLEL_JOBS} parallel jobs..."
echo "Parameters:"
if [ "$DURATION_END" -gt 0 ]; then
    echo "  - Duration range: ${MAX_DURATION}-${DURATION_END} (step: ${DURATION_STEP})"
else
    echo "  - Duration: ${MAX_DURATION}"
fi

if [ "$ARRIVAL_END" -gt 0 ]; then
    echo "  - Arrival range: ${MAX_ARRIVAL}-${ARRIVAL_END} (step: ${ARRIVAL_STEP})"
else
    echo "  - Arrival: ${MAX_ARRIVAL}"
fi

if [ "$REQUEST_RATE_END" != "0" ]; then
    echo "  - Request rate range: ${REQUEST_RATE}-${REQUEST_RATE_END} (step: ${REQUEST_RATE_STEP})"
else
    echo "  - Request rate: ${REQUEST_RATE}"
fi

echo "  - Timesteps: ${TIMESTEPS}"
echo "  - Output directory: ${exp_dir}"
echo "----------------------------------------"

# Create a job list file
job_list_file="${exp_dir}/job_list.txt"
> "$job_list_file"  # Clear/create the job list file

# Build the job list
current_duration=$MAX_DURATION
while [ "$current_duration" -le "$DURATION_END" ] || [ "$DURATION_END" -eq 0 ]; do
    current_arrival=$MAX_ARRIVAL
    while [ "$current_arrival" -le "$ARRIVAL_END" ] || [ "$ARRIVAL_END" -eq 0 ]; do
        current_rate=$REQUEST_RATE
        while (( $(echo "$current_rate <= $REQUEST_RATE_END" | bc -l) )) || [ "$REQUEST_RATE_END" = "0" ]; do
            config_name="d${current_duration}-a${current_arrival}-r${current_rate}"
            SEED=$(date +%s)$RANDOM  # Add more randomness
            OUTPUT_FILE="${exp_dir}/${config_name}.json"
            
            echo "${current_duration}|${current_arrival}|${current_rate}|${SEED}|${OUTPUT_FILE}" >> "$job_list_file"
            
            # Break if not ranging through rates
            if [ "$REQUEST_RATE_END" = "0" ]; then
                break
            fi
            
            # Increment rate
            current_rate=$(echo "$current_rate + $REQUEST_RATE_STEP" | bc)
        done

        # Break if not ranging through durations
        if [ "$ARRIVAL_END" -eq 0 ]; then
            break
        fi
        
        # Increment duration
        current_arrival=$((current_arrival + ARRIVAL_STEP))
    done
    
    # Break if not ranging through durations
    if [ "$DURATION_END" -eq 0 ]; then
        break
    fi
    
    # Increment duration
    current_duration=$((current_duration + DURATION_STEP))
done

total_jobs=$(wc -l < "$job_list_file")
completed_jobs=0
start_time=$(date +%s)

# Create a counter file to track progress
progress_file="${exp_dir}/.progress"
echo "0" > "$progress_file"

# Function to update progress
update_progress() {
    local completed=$1
    local total=$2
    local width=40
    local percentage=$((100 * completed / total))
    local completed_bar=$((width * percentage / 100))
    
    local current_time=$(date +%s)
    local elapsed=$((current_time - start_time))
    
    local elapsed_min=$((elapsed / 60))
    local elapsed_sec=$((elapsed % 60))
    
    # Only estimate remaining time if we have completed at least one job
    if [ "$completed" -gt 0 ]; then
        local per_job=$((elapsed / completed))
        local remaining=$((per_job * (total - completed)))
        local remaining_min=$((remaining / 60))
        local remaining_sec=$((remaining % 60))
        local time_info=$(printf " | %02d:%02d elapsed | ~%02d:%02d remaining" $elapsed_min $elapsed_sec $remaining_min $remaining_sec)
    else
        local time_info=" | calculating time..."
    fi
    
    local bar=""
    for ((i=0; i<$completed_bar; i++)); do
        bar+="█"
    done
    
    for ((i=$completed_bar; i<$width; i++)); do
        bar+=" "
    done
    
    printf "\033[K\rProgress: [${bar}] ${percentage}%% ($completed/$total)${time_info}"
}

echo "Running simulations in parallel with $PARALLEL_JOBS concurrent jobs..."

# Create a semaphore mechanism
sem_file="${exp_dir}/.semaphore"
mkfifo "$sem_file"
exec 3<>"$sem_file"
rm "$sem_file"

# Initialize the semaphore
for ((i=0; i<$PARALLEL_JOBS; i++)); do
    echo >&3
done

increment_progress() {
    local lock_file="${exp_dir}/.progress_lock"
    (
        flock -x 200
        local current=$(cat "$progress_file")
        local next=$((current + 1))
        echo $next > "$progress_file"
        update_progress $next $total_jobs
    ) 200>"$lock_file"
}

# Process each job
while read job_info; do
    # Get a slot from the semaphore
    read -u3

    duration=$(echo $job_info | cut -d'|' -f1)
    arrival=$(echo $job_info | cut -d'|' -f2)
    rate=$(echo $job_info | cut -d'|' -f3)
    seed=$(echo $job_info | cut -d'|' -f4)
    output=$(echo $job_info | cut -d'|' -f5)
    
    (

        ./build/palloc -e data.json -o "$output" -d "$duration" -a "$arrival" -r "$rate" -s "$seed" -a "$AGGREGATIONS" -t "$TIMESTEPS" > /dev/null 2>&1
        
        # Log the run
        echo "Duration: ${duration}, Arrival: ${arrival}, Rate: ${rate}, Seed: ${seed}" >> "${exp_dir}/summary.txt"
        
        increment_progress
        
        # Return the semaphore slot
        echo >&3
    ) &
done < "$job_list_file"

wait

# Close the semaphore
exec 3>&-

echo -e "\n\nSimulations completed!"

rm -f "$progress_file" "$progress_lock" "$job_list_file"

echo "Created experiment directory: $exp_dir"

# Generate reports
echo "Generating reports..."
python analysis/generate_report.py data.json experiments/