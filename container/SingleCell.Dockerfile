# Must be built from the root directory of the repo
# BUILD: docker build -t singlecell -f container/SingleCell.Dockerfile .
# TEST LOCAL (optional): docker run -p 8888:8888 --name test1 -i -t singlecell
# TAG: docker tag singlecell JonahRileyHuggins/SingleCell:latest
# PUSH: docker push JonahRileyHuggins/SingleCell:latest

# Dockerfile for SingleCell - optimized for bind mounts and venv isolation
FROM python:3.12-slim

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    SHELL=/bin/bash \
    BLAS_LIBS=-lopenblas \
    PATH=/root/.local/bin:$PATH \
    VENV_PATH=/opt/.venv 
    # BUILD_PATH=/opt/SingleCell-build \
    # THIRD_PARTY_BUILD=/opt/SingleCell-build/ThirdParty \
    # # PYTHONPATH=/opt/SingleCell-build:$PYTHONPATH \
    # LD_LIBRARY_PATH=/opt/SingleCell-build/lib:/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# Install basic dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        curl \
        wget \
        libboost-all-dev \
        libopenblas-dev \
        gcc \
        g++ \
        git \
        gfortran \
        cmake \
        swig \
        pipx \
        libhdf5-dev \
        vim nano curl wget dos2unix \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Create directories for builds
RUN mkdir -p $VENV_PATH $BUILD_PATH $THIRD_PARTY_BUILD

# Copy only essential project files
WORKDIR /SingleCell
COPY sbml_files ./sbml_files
COPY benchmarks ./benchmarks
COPY dist ./dist
COPY data ./data
COPY src ./src
COPY include ./include
COPY python ./python
COPY amici_models ./amici_models
COPY CMakeLists.txt ./CMakeLists.txt
COPY requirements.txt .

# Sanitize shell scripts from Windows line endings
RUN find . -type f -name "*.sh" -exec dos2unix {} \; \
 && find . -type f -name "*.sh" -exec chmod +x {} \;

# Create virtual environment and install Python deps
RUN pipx ensurepath
RUN pipx install dist/singlecell-0.0.1-py3-none-any.whl --verbose --force

# # Build pySingleCell C++ extension into BUILD_PATH and install in venv
# WORKDIR $BUILD_PATH
# RUN cd /SingleCell/python/pySingleCell \
#  && . $VENV_PATH/bin/activate \
#  && python setup.py build_ext --inplace --build-lib $BUILD_PATH \
#  && cp $BUILD_PATH/pySingleCell*.so $VENV_PATH/lib/python3.12/site-packages/

# # Build models inside container
# RUN . $VENV_PATH/bin/activate \
#  && cd /SingleCell/python/ModelBuilding \
#  && python createModels.py -p ../../data/config.yaml

# # Build SingleCell core C++ (optional, if needed)
# RUN cd /SingleCell \
#  && cmake -B $BUILD_PATH/build -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
#  && cmake --build $BUILD_PATH/build

# Set default shell
SHELL ["/bin/bash", "-c"]

# Default entrypoint: activate venv and drop into bash
# CMD ["bash", "-c", "source $VENV_PATH/bin/activate && exec bash"]
