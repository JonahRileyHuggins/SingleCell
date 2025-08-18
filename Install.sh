#!/bin/bash

# copy project files into local app directory
mkdir ~/.local/share/SingleCell
cp -r ./* ~/.local/share/SingleCell

# persistent environmental variable
echo "export SINGLECELL_PATH=~/'.local/share/SingleCell'" >> ~/.bashrc

# Install pipx tool
pipx ensurepath#!/bin/bash
set -e  # exit on first error
set -x  # print each command before executing

# copy project files into local app directory
mkdir -p ~/.local/share/SingleCell
cp -r ./* ~/.local/share/SingleCell

# persistent environment variable
echo 'export SINGLECELL_PATH="$HOME/.local/share/SingleCell"' >> ~/.bashrc

# Install pipx tool
pipx ensurepath
pipx install ./python/dist/singlecell-0.0.1-py3-none-any.whl --verbose --force

# reset and exit 
echo -e "exiting...\n"