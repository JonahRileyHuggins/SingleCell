# Must be built from the root directory of the repo
# BUILD: docker build -t singlecell -f container/SingleCell.Dockerfile .
# TEST LOCAL (optional): docker run -it --rm jonahrileyhuggins/singlecell:latest
# TAG: docker tag singlecell jonahrileyhuggins/singlecell:latest
# PUSH: docker push jonahrileyhuggins/singlecell:latest

# Dockerfile for SingleCell - optimized for bind mounts and venv isolation
FROM python:3.12-slim

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    SHELL=/bin/bash \
    BLAS_LIBS=-lopenblas \
    SINGLECELL_PATH=/SingleCell \
    PATH=/root/.local/bin:$PATH

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
        vim dos2unix \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Copy only essential project files
WORKDIR /SingleCell
COPY . /SingleCell/

# Sanitize shell scripts from Windows line endings
RUN find . -type f -name "*.sh" -exec dos2unix {} \; \
 && find . -type f -name "*.sh" -exec chmod +x {} \;

# Install dependencies
RUN ./Install.sh

# For HPC:
# RUN /root/.local/bin/SingleCell Build -p ./data/config.yaml 

# install python dependencies
RUN pip3 install -r requirements.txt
RUN pip3 install jupyter

# open directory as jupyter notebook
ENV TINI_VERSION=v0.6.0
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /usr/bin/tini
RUN chmod +x /usr/bin/tini
ENTRYPOINT ["/usr/bin/tini", "--"]
CMD ["jupyter", "notebook", "--port=8888", "--no-browser", "--ip=0.0.0.0", "--allow-root"]
