# Must be built from the root directory of the repo
# BUILD: docker build -t singlecell -f container/SingleCell.Dockerfile .
# TEST LOCAL (optional): docker run -p 8888:8888 --name test1 -i -t singlecell
# TAG: docker tag singlecell JonahRileyHuggins/SingleCell:latest
# PUSH: docker push JonahRileyHuggins/SingleCell:latest

FROM ubuntu:24.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    SHELL=/bin/bash \
    BLAS_LIBS=-lopenblas \
    PATH=/root/.local/bin:$PATH \
    VENV_PATH=/SingleCell/.venv

# Copy SingleCell files (ensure build context matches path)
RUN mkdir -p /SingleCell
COPY . /SingleCell/

# Safety Check; verifies any files from Windows are sanitized
RUN apt-get update && apt-get install -y dos2unix \
    && find /SingleCell -type f -name "*.sh" -exec dos2unix {} \; \
    && find /SingleCell -type f -name "*.sh" -exec chmod +x {} \; \
    && find /SingleCell -type l -exec sh -c 'target=$(readlink "{}"); cp --remove-destination "$target" "{}"' \; \
    && apt-get remove -y dos2unix && apt-get autoremove -y



# Set working directory
WORKDIR /SingleCell


# Update and install basic dependencies
RUN apt-get install -y --no-install-recommends \
        build-essential \
        curl \
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
        swig \
        libhdf5-dev \
        && apt-get clean && rm -rf /var/lib/apt/lists/*

# Create persistent paths for pipx
# RUN mkdir -p $PIPX_HOME $PIPX_BIN_DIR

RUN cd /SingleCell
RUN python3 -m venv .venv \
    && . .venv/bin/activate \
    && pip install -r requirements.txt

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

    # Build models inside container
RUN cd /SingleCell/ \
    && . .venv/bin/activate \
    && cd python/ModelBuilding/ \
    && python3 createModels.py -p ../../data/config.yaml

    ## SingleCell:
RUN cd /SingleCell/ \
    && cmake -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    && cmake --build build

# Set default shell
SHELL ["/bin/bash", "-c"]

# on run: activates the virtual environment stored in SingleCell/.venv
CMD ["/bin/bash", "-c", "source $VENV_PATH/bin/activate && exec bash"]
# Test the installation
# RUN sparced --help