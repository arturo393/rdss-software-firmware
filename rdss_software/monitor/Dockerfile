FROM python:alpine

COPY ./src/entrypoint.sh entrypoint.sh
RUN chmod a+x entrypoint.sh

WORKDIR /opt/rdss/monitor
# RUN apt -y update && apt -y upgrade
# RUN apt -y install curl socat
RUN apk add curl socat gcc musl-dev g++
RUN /usr/local/bin/python -m pip install --upgrade pip
RUN pip install python-socketio pymongo crccheck pyserial sympy Flask flask_socketio eventlet numpy

CMD /bin/sh /entrypoint.sh