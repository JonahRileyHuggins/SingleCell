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

# Install basic dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ninja-build \
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
COPY . /SingleCell/

# Sanitize shell scripts from Windows line endings
RUN find . -type f -name "*.sh" -exec dos2unix {} \; \
 && find . -type f -name "*.sh" -exec chmod +x {} \;

# Install dependencies
RUN ./Install.sh

# Set default shell
SHELL ["/bin/bash", "-c"]

# Default entrypoint: activate venv and drop into bash
CMD ["bash", "-c", "SingleCell --help; exec bash"]
