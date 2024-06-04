import { exec } from 'child_process';
import fs from 'fs';
import path from 'path';

const LOG_FILE = '/var/log/change_ip.log';
const ENV_FILE = '/opt/rdss/frontend/.env.local';
const INTERFACES_FILE = '/etc/network/interfaces';

const logMessage = (message) => {
  const timestamp = new Date().toISOString();
  fs.appendFileSync(LOG_FILE, `${timestamp} - ${message}\n`);
};

const changeIpInEnvFile = (newIp) => {
  logMessage('Cambiando IP en el archivo .env.local');
  let envContent = fs.readFileSync(ENV_FILE, 'utf8');
  envContent = envContent.replace(/NEXT_PUBLIC_APIHOST=.*/, `NEXT_PUBLIC_APIHOST=${newIp}`);
  envContent = envContent.replace(/NEXT_PUBLIC_SOCKETSHOST=.*/, `NEXT_PUBLIC_SOCKETSHOST=${newIp}`);
  fs.writeFileSync(ENV_FILE, envContent, 'utf8');
  logMessage(fs.readFileSync(ENV_FILE, 'utf8'));
};

const changeIpInInterfacesFile = (newIp, newGateway, newNetmask) => {
  logMessage('Cambiando IP en el archivo /etc/network/interfaces');
  let interfacesContent = fs.readFileSync(INTERFACES_FILE, 'utf8');
  interfacesContent = interfacesContent.replace(/address .*/, `address ${newIp}`);
  interfacesContent = interfacesContent.replace(/netmask .*/, `netmask ${newNetmask}`);
  interfacesContent = interfacesContent.replace(/gateway .*/, `gateway ${newGateway}`);
  fs.writeFileSync(INTERFACES_FILE, interfacesContent, 'utf8');
  logMessage(fs.readFileSync(INTERFACES_FILE, 'utf8'));
};

export default async function handler(req, res) {
  if (req.method === 'POST') {
    const { ip, gateway, netmask } = req.body;

    if (!ip || !gateway || !netmask) {
      return res.status(400).json({ success: false, message: 'Missing required parameters.' });
    }

    try {
      // Cambiar IP en el archivo .env.local
      changeIpInEnvFile(ip);

      // Asegurarse de que el archivo se ha guardado correctamente
      fs.accessSync(ENV_FILE, fs.constants.R_OK | fs.constants.W_OK);
      logMessage('El archivo .env.local se ha guardado correctamente.');

      // Cambiar IP en el archivo /etc/network/interfaces
      changeIpInInterfacesFile(ip, gateway, netmask);

      logMessage(`La IP ha sido cambiada correctamente a ${ip}`);
      logMessage('El sistema se reiniciará en 5 segundos...');
      setTimeout(() => {
        exec('shutdown -r now', (error, stdout, stderr) => {
          if (error) {
            logMessage(`Error al reiniciar el sistema: ${stderr}`);
            return res.status(500).json({ success: false, message: 'Failed to restart the system.' });
          }
          logMessage(`Sistema reiniciado: ${stdout}`);
        });
      }, 5000);

      return res.json({
        success: true,
        message: 'IP, Gateway, and Netmask changed successfully. The system will restart in 5 seconds.',
      });
    } catch (error) {
      logMessage(`Error: ${error.message}`);
      return res.status(500).json({ success: false, message: 'Failed to change IP, Gateway, and Netmask.' });
    }
  } else {
    res.setHeader('Allow', ['POST']);
    res.status(405).end(`Method ${req.method} Not Allowed`);
  }
}
