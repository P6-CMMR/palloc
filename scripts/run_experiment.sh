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
  echo "  -r, --requests    Request rate per timestep (can be a range: MIN-MAX), default: 10.0"
  echo "  -t, --timesteps   Number of timesteps to simulate, default: 1440"
  echo ""
  echo "Examples:"
  echo "  $0 -d 600-1200          # Run 1 simulations per step in the range 600-1200"
  echo "  $0 -r 5.0-15.0          # Run 1 simulation per step in the range 5.0-15.0"
}

# Default values
MAX_DURATION=600
DURATION_END=0
REQUEST_RATE=10.0
REQUEST_RATE_END=0
AGGREGATIONS=3
TIMESTEPS=1440

# Set fixed step sizes
DURATION_STEP=10
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

progress_bar() {
    local progress=$1
    local total=$2
    local width=40
    local percentage=$((100 * progress / total))
    local completed=$((width * percentage / 100))
    
    local current_time=$(date +%s)
    if [ "$progress" -gt 1 ]; then
        # Calculate elapsed and estimate remaining time
        local elapsed=$((current_time - start_time))
        local per_iteration=$((elapsed / (progress - 1)))
        local remaining=$((per_iteration * (total - progress)))

        local elapsed_min=$((elapsed / 60))
        local elapsed_sec=$((elapsed % 60))
        local remaining_min=$((remaining / 60))
        local remaining_sec=$((remaining % 60))
        
        local time_info=$(printf " | %02d:%02d elapsed | ~%02d:%02d remaining" $elapsed_min $elapsed_sec $remaining_min $remaining_sec)
    else
        local time_info=" | calculating time..."
        start_time=$(date +%s)
    fi

    local bar=""
    for ((i=0; i<$completed; i++)); do
        bar+="█"
    done

    for ((i=$completed; i<$width; i++)); do
        bar+=" "
    done

    printf "\033[K\rProgress: [${bar}] ${percentage}%% ($progress/$total)${time_info}"
}

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
echo "Number of runs: ${AGGREGATIONS}" >> "${exp_dir}/summary.txt"
echo "----------------------------------------" >> "${exp_dir}/summary.txt"

echo "Running ${total_configs} simulations..."
echo "Parameters:"
if [ "$DURATION_END" -gt 0 ]; then
    echo "  - Duration range: ${MAX_DURATION}-${DURATION_END} (step: ${DURATION_STEP})"
else
    echo "  - Duration: ${MAX_DURATION}"
fi

if [ "$REQUEST_RATE_END" != "0" ]; then
    echo "  - Request rate range: ${REQUEST_RATE}-${REQUEST_RATE_END} (step: ${REQUEST_RATE_STEP})"
else
    echo "  - Request rate: ${REQUEST_RATE}"
fi

echo "  - Timesteps: ${TIMESTEPS}"
echo "  - Output directory: ${exp_dir}"
echo "----------------------------------------"

current_run=0
start_time=0

# Iterate over durations
current_duration=$MAX_DURATION
while [ "$current_duration" -le "$DURATION_END" ] || [ "$DURATION_END" -eq 0 ]; do
    # Iterate over request rates
    current_rate=$REQUEST_RATE
    while (( $(echo "$current_rate <= $REQUEST_RATE_END" | bc -l) )) || [ "$REQUEST_RATE_END" = "0" ]; do
        ((++current_run))
        config_name="d${current_duration}-r${current_rate}"
        SEED=$(date +%s)
        OUTPUT_FILE="${exp_dir}/${config_name}.json"
        
        progress_bar $current_run $total_configs
        
        # Call engine
        ./build/palloc -e data.json -o "$OUTPUT_FILE" -d "$current_duration" -r "$current_rate" -s "$SEED" -a "$AGGREGATIONS" -t "$TIMESTEPS"> /dev/null 2>&1
        
        echo "Duration: ${current_duration}, Rate: ${current_rate}, Seed: ${SEED}" >> "${exp_dir}/summary.txt"
        
        # Break if not ranging through rates
        if [ "$REQUEST_RATE_END" = "0" ]; then
            break
        fi
        
        # Increment rate
        current_rate=$(echo "$current_rate + $REQUEST_RATE_STEP" | bc)
    done
    
    # Break if not ranging through durations
    if [ "$DURATION_END" -eq 0 ]; then
        break
    fi
    
    # Increment duration
    current_duration=$((current_duration + DURATION_STEP))
done

echo -e "\n\nSimulations completed!"
echo "Created experiment directory: $exp_dir"

# Generate reports
echo "Generating reports..."
python analysis/generate_report.py data.json experiments/