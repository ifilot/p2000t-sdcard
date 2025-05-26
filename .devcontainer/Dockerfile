# Use the z88dk/z88dk image as the base image
FROM --platform=linux/amd64 z88dk/z88dk:latest

# Set environment variables for Python installation
ENV PYTHONUNBUFFERED=1

# Install Python and other dependencies
RUN apk update && apk add --no-cache \
    python3 \
    # py3-pip \
    # py3-virtualenv \
    bash \
    git \
    && ln -sf python3 /usr/bin/python

# Set bash as the default shell for the container
SHELL ["/bin/bash", "-c"]

# Set bash as the entry point so the container opens with bash
ENTRYPOINT ["/bin/bash"]