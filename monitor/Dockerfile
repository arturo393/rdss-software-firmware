FROM python

COPY ./src/entrypoint.sh entrypoint.sh

RUN chmod a+x entrypoint.sh

WORKDIR /opt/rdss/monitor
RUN apt -y update && apt -y upgrade
RUN apt -y install curl socat

COPY ./src/requirements.txt .
RUN pip install -r requirements.txt


CMD /bin/sh /entrypoint.sh