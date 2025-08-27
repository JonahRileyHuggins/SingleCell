#!/bin/bash

# copy project files into local app directory
mkdir -p $HOME/.local/share/SingleCell
tar cf - . | (cd $HOME/.local/share/SingleCell && tar xvf -)

# Install pipx tool
pipx ensurepath
pipx install --python python3.12 ./python/dist/singlecell-0.0.1-py3-none-any.whl --verbose --force

# reset and exit 
echo -e "exiting...\n"