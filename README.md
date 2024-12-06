# silcnitc
A simple EXPL compiler.

## Instructions
Each branch of this repository corresponds to a particular stage in the [roadmap](https://silcnitc.github.io/expl-docs/roadmap).
All required learning material can be found on the main resource website.

Any one of the below two methods can be used for initial setup:
+ Docker setup
+ Manual installation

## Docker setup
### Build container image (one-time)
```
docker build -t expl:ubuntu20.04 .
```

### Start container instance (one-time)
```
docker run -v ${PWD}/workdir:/home/expl/xsm_expl/workdir -d --name expl -i expl:ubuntu20.04
```

This will start an instance of the container and map the local folder `workdir` to `/home/expl/workdir` directory of the container. We now have a container named `expl` running in the background.

### Connect to container 
```
docker start expl # Run this command to start container if not already running
docker exec -it expl /bin/bash # Get bash shell inside container

# If root permissions are required use
# docker exec -u root -it expl /bin/bash 
```

### Initialize eXpOS (one-time)
All compiled programs are tested on [eXpOS](https://exposnitc.github.io/expos-docs). Run the following commands after connecting to container.
```
cd /home/expl/xsm_expl/xfs-interface
./init
```

## Manual installation
For local setup, follow [link](https://silcnitc.github.io/expl-docs/install).
