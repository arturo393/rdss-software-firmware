#!/bin/bash

# Variables
NEW_IP=$1
NEW_GATEWAY=$2
NEW_NETMASK=$3
ENV_FILE="/opt/rdss/frontend/.env.local"
INTERFACES_FILE="/etc/network/interfaces"
SERVICE_NAME="frontendRDSS"
LOG_FILE="/var/log/change_ip.log"


# Paso 3: Reconstruir el frontend
log_message "Reconstruyendo el frontend"
cd /opt/rdss/frontend || { log_message "Error: No se pudo cambiar al directorio /opt/rdss/frontend"; exit 1; }
npm run build >> $LOG_FILE 2>&1 || { log_message "Error: La construcción del frontend falló"; exit 1; }

cat $ENV_FILE >> $LOG_FILE 2>&1

# Confirmar cambios de IP
log_message "La IP ha sido cambiada correctamente a $NEW_IP"

# Mensaje de reinicio
log_message "El sistema se reiniciará en 5 segundos..."

systemctl restart frontendRDSS.service

