# User Guide

_Written by Jonah R. Huggins_

## Overview
⚠️ Work In Progress ⚠️ 

- Code & Commands are hard-coded for the SingleCell root directory for the moment. Future updates will focus on flexible path specifications.

### Simple Operation
A container can be pulled from DockerHub that has the command-line interface pre-installed. The container is intended to be used from the command line, therefore running with an interactive (`-i`) teletypewriter (`-t`) flag is required. The below command will simultaneously pull the container from DockerHub and launch a session: 
```docker run -it --name demo jonahrileyhuggins/singlecell:latest```

To restart the container from your last session, call the container by the assigned name, an example is provided below:
`docker start -ai demo`

#### SingleCell CLI Commands
The container has 3 primary commands, each with their own help profile. While the container should launch the command list on startup, to view the primary commands again, type
```SingleCell --help #or -h```

Further, to view the list of arguments and an example use case, you can use the `--help` flag with these as well
```SingleCell <Primary Command> --help```

### Note! 
This is a work in progress, if you encounter errors during any process, following these steps helps in development:

1. Ensure you're working with the latest container version on DockerHub
2. If the error persists; navigate to the GitHub page and leave an issue describing the problem you've encountered
