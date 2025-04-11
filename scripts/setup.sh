#!/bin/bash
# Get the script location and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Change to project root if not already there
if [[ "$(pwd)" != "$PROJECT_ROOT" ]]; then
    cd "$PROJECT_ROOT"
fi

if [ "$1" == "clean" ]; then
    echo "Clean setup..."
    # Remove virtual environment
    if [ -d ".venv" ]; then
        rm -rf .venv
    fi
    
    # Remove osm-server directory
    if [ -d "osrm-server" ]; then
        rm -rf osrm-server
    fi
fi

cd tools
if [ ! -d "FlameGraph" ]; then
    echo "FlameGraph not found. Cloning repository..."
    if ! command -v git &> /dev/null; then
        echo "Git could not be found. Installing Git..."
        sudo apt install git -y
    fi

    git clone https://github.com/brendangregg/FlameGraph.git
    echo "FlameGraph cloned successfully"
fi

cd ..
if ! command -v docker &> /dev/null; then
    echo "Docker could not be found. Installing Docker..."

    sudo apt update
    sudo apt install ca-certificates curl
    sudo install -m 0755 -d /etc/apt/keyrings
    sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
    sudo chmod a+r /etc/apt/keyrings/docker.asc

    echo \
    "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
    $(. /etc/os-release && echo "${UBUNTU_CODENAME:-$VERSION_CODENAME}") stable" | \
    sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
    sudo apt update

    sudo apt install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
fi

if [ ! -d "osrm-server" ]; then
    mkdir osrm-server
    echo "Created osrm-server directory"
fi

cd osrm-server

# Check if Denmark map file already exists
if [ ! -f "denmark-latest.osm.pbf" ]; then
    echo "Downloading map of Denmark..."
    wget https://download.geofabrik.de/europe/denmark-latest.osm.pbf
else
    echo "Map of Denmark already downloaded"
fi

echo "Extracting map data..."
# Only run extract if the .osrm file doesn't exist yet
if [ ! -f "denmark-latest.osrm.timestamp" ]; then
    sudo docker run -t -v "${PWD}:/data" ghcr.io/project-osrm/osrm-backend osrm-extract -p /opt/car.lua /data/denmark-latest.osm.pbf || echo "osrm-extract failed"
else
    echo "Map already extracted"
fi

# Only run partition if needed
if [ ! -f "denmark-latest.osrm.partition" ]; then
    sudo docker run -t -v "${PWD}:/data" ghcr.io/project-osrm/osrm-backend osrm-partition /data/denmark-latest.osrm || echo "osrm-partition failed"
else
    echo "Map already partitioned"
fi

# Only run customize if needed
if [ ! -f "denmark-latest.osrm.cell_metrics" ]; then
    sudo docker run -t -v "${PWD}:/data" ghcr.io/project-osrm/osrm-backend osrm-customize /data/denmark-latest.osrm || echo "osrm-customize failed"
else
    echo "Map already customized"
fi

cd ..
echo "Setting up Python environment..."
if ! command -v python3 &> /dev/null; then
    echo "Python 3 could not be found. Installing Python 3..."
    sudo apt update
    sudo apt install -y python3 python3-pip python3-venv
fi

python3 -m venv .venv
source .venv/bin/activate

echo "Installing Python dependencies..."
pip install -r requirements.txt > /dev/null

echo "Starting OSRM backend..."
cd osrm-server
PORT_IN_USE=$(sudo docker ps | grep -c "0.0.0.0:5000->5000")
if [ $PORT_IN_USE -gt 0 ]; then
    echo "Port 5000 is already in use by a Docker container"
    
    # Find the container ID using port 5000
    EXISTING_CONTAINER=$(sudo docker ps | grep "0.0.0.0:5000->5000" | awk '{print $1}')
    
    echo "Stopping existing container: $EXISTING_CONTAINER"
    sudo docker stop $EXISTING_CONTAINER
    sudo docker rm $EXISTING_CONTAINER
    
    echo "Container removed, continuing with setup..."
fi

CONTAINER_ID=$(sudo docker run -d -p 5000:5000 -v "${PWD}:/data" \
    ghcr.io/project-osrm/osrm-backend osrm-routed --algorithm mld --max-table-size 100000000 \
    /data/denmark-latest.osrm)

# Check if container started successfully
if [ $? -ne 0 ] || [ -z "$CONTAINER_ID" ]; then
    echo "Error: Failed to start OSRM backend server"
    exit 1
fi

echo -n "Waiting for OSRM backend to initialize: "
MAX_RETRIES=10
RETRY_COUNT=0

# Function to check if OSRM server is ready
check_server() {
    if curl -s "http://localhost:5000/route/v1/driving/8.681495,49.41461;8.686507,49.41943" &>/dev/null; then
        return 0
    fi

    return 1
}

# Wait for server to be ready with timeout
while [ $RETRY_COUNT -lt $MAX_RETRIES ]; do
    if check_server; then
        echo -e "\rOSRM backend is ready!                                        "
        break
    fi
    
    RETRY_COUNT=$((RETRY_COUNT + 1))
    if [ $RETRY_COUNT -eq $MAX_RETRIES ]; then
        echo "Error: OSRM backend failed to start after $MAX_RETRIES attempts"
        echo "Cleaning up failed container..."
        sudo docker stop $CONTAINER_ID > /dev/null
        sudo docker rm $CONTAINER_ID > /dev/null
        exit 1
    fi
    
    echo -ne "\rWaiting for OSRM backend to initialize: ${RETRY_COUNT}s "
    sleep 1
done

cd ../preprocessing
if [ ! -f "aalborg-map.osm" ]; then
    echo "Downloading detailed map of Aalborg..."
    python3 fetch_map.py
else
    echo "Aalborg map already downloaded"
fi

echo "Generating environment file..."
python3 generate_environment.py

echo "Generating test data file..."
cd ..
python3 generate_test_data.py

echo "Shutting down OSRM backend..."
sudo docker stop $CONTAINER_ID > /dev/null
sudo docker rm $CONTAINER_ID > /dev/null

# Check if cmake is installed and install if not
if ! command -v cmake &> /dev/null; then
    echo "cmake could not be found. Installing cmake..."
    sudo apt install cmake -y
fi

# Check if make is installed and install if not
if ! command -v make &> /dev/null; then
    echo "make could not be found. Installing make..."
    sudo apt install make -y
fi

# Check if g++-14 is installed and install if not
if ! command -v g++-14 &> /dev/null; then
    echo "g++-14 could not be found. Installing g++-14..."
    sudo apt install g++-14 -y
fi

echo "Setup complete."