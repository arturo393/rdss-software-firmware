FROM node
# RUN apt -y update & apt -y upgrade
# RUN apt -y install xsel
WORKDIR /opt/rdss/frontend

ADD src .
RUN npm install -g npm@latest
RUN npm install
RUN npm install --save-dev eslint eslint-config-next
# RUN npx next build /opt/rdss/frontend
# RUN npm run build
RUN npm -g install serve

# CMD "npm run dev"
CMD serve -n out