FROM node:16.12.0
# RUN apt -y update & apt -y upgrade
# RUN apt -y install xsel
WORKDIR /opt/rdss/frontend

ADD src .
# RUN npm install -g npm@latest
RUN npm install --force
RUN npm install next@11.0.1 --force
RUN npm install --save --legacy-peer-deps && npm install @material-ui/core --save --legacy-peer-deps
RUN npm install --save-dev eslint eslint-config-next --force
# RUN npx next build

CMD /usr/bin/env npx next start
# CMD node server.js
# CMD serve -n out