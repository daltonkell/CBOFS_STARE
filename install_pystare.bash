#/bin/bash

# NOTE: this expects you have already created a Python
# virtual environment and that the requirements.txt
# are already installed

export PY_VENV_DIR=./env
export STARE_INCLUDE_DIR=/home/dalton/SpatioTemporal/STARE/build/include/
export STARE_LIB_DIR=/home/dalton/SpatioTemporal/STARE/build/lib/
export PYSTARE=/home/dalton/SpatioTemporal/pystare

source $PY_VENV_DIR/bin/activate
pip install $PYSTARE
