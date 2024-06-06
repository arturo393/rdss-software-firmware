const express = require('express');
const { exec } = require('child_process');
const app = express();

app.use(express.json());

app.post('/api/change-ip', (req, res) => {
  const { ip, gateway } = req.body;

  exec(`/opt/rdss/change_ip.sh ${ip} ${gateway}`, (error, stdout, stderr) => {
    if (error) {
      console.error(`exec error: ${error}`);
      return res.status(500).json({ success: false, message: 'Failed to change IP and Gateway.' });
    }

    console.log(`stdout: ${stdout}`);
    console.error(`stderr: ${stderr}`);
    res.json({ success: true });
  });
});

app.listen(3000, () => {
  console.log('Server is running on port 3000');
});
