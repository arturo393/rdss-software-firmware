const express = require('express');
const bodyParser = require('body-parser');
const { exec } = require('child_process');

const app = express();
const PORT = 3001;

app.use(bodyParser.json());

app.post('/change-ip', (req, res) => {
  const { ip, gateway } = req.body;

  exec(`/opt/rdss/change_ip.sh ${ip} ${gateway}`, (error, stdout, stderr) => {
    if (error) {
      console.error(`exec error: ${error}`);
      return res.status(500).json({ success: false, message: 'Failed to change IP and Gateway.' });
    }

    console.log(`stdout: ${stdout}`);
    console.error(`stderr: ${stderr}`);
    res.json({ success: true, message: 'IP and Gateway changed successfully.' });
  });
});

app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
