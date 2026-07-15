#!/bin/bash

# Configuration (Change to your Pi's actual details)
PI_USER="username"
PI_IP="raspberrypi.local"
PI_DIR="/home/$PI_USER/apps"

# 1. Compile the code
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# 2. Ensure the target folder exists on the Pi and transfer the binary
ssh $PI_USER@$PI_IP "mkdir -p $PI_DIR"
rsync -avz my_pi_app $PI_USER@$PI_IP:$PI_DIR/

echo "🚀 Deployment finished! Run it on your Pi using: ssh $PI_USER@$PI_IP '$PI_DIR/my_pi_app'"
