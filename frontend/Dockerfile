FROM node
WORKDIR /opt/rdss/frontend

ADD src .
RUN npm install -g npm@latest
RUN npm install

CMD "npm run dev"