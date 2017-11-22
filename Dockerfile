FROM ubuntu:16.04
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y build-essential libmysqlclient-dev libwebsockets-dev

# Build the ASCL
COPY modules/ascl /opt/ASCL
RUN /opt/ASCL/build.sh

# Build manager
COPY modules/remote_attestation /tmp/remote_attestation
RUN /tmp/remote_attestation/build_server.sh /opt/remote_attestation /opt/ASCL

# Clean up
RUN rm -rf /tmp/*

ENTRYPOINT /opt/remote_attestation/ra_manager $(nproc) >> /opt/online_backends/remote_attestation/ra_manager.log 2>&1
