#!/bin/bash

# build the target example
./ns3 configure --disable-python --enable-examples
./ns3 build $NS3_EXAMPLE

./ns3 run $NS3_EXAMPLE -- --sionna-local-machine=false --sionna-server-ip=$SIONNA_IP

# Create a timestamped folder inside the "results" directory
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTS_DIR="results/$TIMESTAMP"
mkdir -p "$RESULTS_DIR"

# Copy all *Stats.txt files into the timestamped folder
find . -name "*Stats.txt" -exec cp {} "$RESULTS_DIR" \;