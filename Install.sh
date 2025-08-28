#!/bin/bash

# persistent environment variable
echo "export SINGLECELL_PATH="/SingleCell" >> ~/.bashrc

# Install pipx tool
pipx ensurepath
pipx install --python python3.12 ./python/dist/singlecell-0.0.1-py3-none-any.whl --verbose --force

# reset and exit 
echo -e "exiting...\n"