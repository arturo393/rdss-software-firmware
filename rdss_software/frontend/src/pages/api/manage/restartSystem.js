import { exec } from 'child_process';

export default function handler(req, res) {
  if (req.method === 'POST') {
    exec('shutdown -r now', (error) => {
      if (error) {
        return res.status(500).json({ success: false, message: 'Failed to restart the system.' });
      }

      return res.json({ success: true, message: 'System is restarting.' });
    });
  } else {
    res.setHeader('Allow', ['POST']);
    res.status(405).end(`Method ${req.method} Not Allowed`);
  }
}
