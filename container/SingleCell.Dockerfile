# Must be built from the root directory of the repo
# BUILD: docker build -t singlecell -f container/SingleCell.docker .
# TEST LOCAL (optional): docker run -p 8888:8888 --name test1 -i -t singlecell
# TAG: docker tag singlecell JonahRileyHuggins/SingleCell:latest
# PUSH: docker push JonahRileyHuggins/SingleCell:latest

FROM ubuntu:24.04

# Copy SingleCell files (ensure build context matches path)
RUN mkdir -p /SingleCell
COPY . /SingleCell/

# Set working directory
WORKDIR /SingleCell

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    SHELL=/bin/bash \
    BLAS_LIBS=-lopenblas \
    PATH=/root/.local/bin:$PATH \
    LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH \
    PIPX_HOME=/usr/local/pipx \
    PIPX_BIN_DIR=/usr/local/bin 

# Update and install basic dependencies
RUN     apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        apt-utils \
        curl \
        libssl-dev \
        libmunge-dev \
        libmunge2 \
        munge \
        bash \
        wget \
        python3-pip \
        python3-venv \
        python3-dev \
        libboost-all-dev \
        libopenblas-dev \
        gcc \
        g++ \
        git \
        gfortran \
        cmake \
        make \
        file \
        libgfortran5 \
        libatomic1 \
        swig \
        libhdf5-dev \
        && apt-get clean && rm -rf /var/lib/apt/lists/*

# Create persistent paths for pipx
RUN mkdir -p $PIPX_HOME $PIPX_BIN_DIR

    # Installing ThirdParty dependencies
    ## muParser
RUN cd /SingleCell/ThirdParty/muparser/ \
    && cmake -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build \
    && cmake --install build

    ## libXML2
RUN cd /SingleCell/ThirdParty/libxml2-2.14.3/ \
    && cmake -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    && cmake --build build \
    && cmake --install build

    ## libSBML
RUN cd /SingleCell/ThirdParty/libsbml-5.20.2/ \
    && cmake -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    && cmake --build build \
    && cmake --install build

    ## AMICI
RUN cd /SingleCell/ThirdParty/AMICI/scripts/ \
    && ./buildSuiteSparse.sh \
    && ./buildSundials.sh \
    && ./buildAmici.sh

    # Build links to models inside container
RUN cd /SingleCell/python/ModelBuilding/ \
    && python3 createModels.py -p ../../data/config.yaml

    ## SingleCell:
RUN cd /SingleCell/ \
    && cmake -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    && cmake --build build

# Install Python dependencies using pipx
RUN pipx ensurepath
RUN pipx install dist/singlecell-0.1-py3-none-any.whl --verbose --force

# Set default shell
SHELL ["/bin/bash", "-c"]

# Test the installation
# RUN sparced --help