#!/bin/bash

# copy project files into local app directory
mkdir ~/.local/share/SingleCell
cp -r ./* ~/.local/share/SingleCell

# persistent environmental variable
echo "export SINGLECELL_PATH=~/'.local/share/SingleCell" >> ~/.bashrc

# Install pipx tool
pipx ensurepath
pipx ./python/dist/singlecell-0.0.1-py3-none-any.whl --verbose --force

# reset and exit 
echo -e "exiting... \n"
source ~/.bashrc