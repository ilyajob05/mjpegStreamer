FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive
ENV TZ=US
RUN apt update && apt install -y tcl
RUN apt upgrade -y
reinstall certificates, otherwise git clone command might result in an error
RUN apt install --reinstall ca-certificates -y

# install developer dependencies
RUN apt install -y git build-essential cmake --no-install-recommends

RUN apt install -y curl zip
RUN apt install -y git libopencv-dev python3-opencv

RUN git clone https://github.com/ilyajob05/mjpegStreamer.git

# copy files from local directory to container
COPY . .

# define working directory from container
WORKDIR /build

# compile with CMake 
RUN bash -c "cmake ../mjpegStreamer && cmake --build ."

# run executable
CMD [ "./mjpegStreamer -src 0 -port 8080 -quality [1-10] -fps 20" ]
