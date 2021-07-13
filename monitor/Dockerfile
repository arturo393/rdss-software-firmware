FROM python

COPY ./src/entrypoint.sh entrypoint.sh
RUN chmod a+x entrypoint.sh

WORKDIR /opt/rdss/monitor
RUN apt -y update && apt -y upgrade
RUN apt -y install curl socat
RUN pip install python-socketio pymongo crccheck pyserial sympy Flask flask_socketio eventlet

CMD /bin/sh /entrypoint.sh