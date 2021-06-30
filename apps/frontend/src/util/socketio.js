import io from "socket.io-client"

const BACKEND_HOST = process.env.BACKEND_HOST
const BACKEND_PORT = process.env.BACKEND_PORT

// if (!BACKEND_HOST) {
//   throw new Error(
//     "Please define the BACKEND_HOST environment variable inside .env.local"
//   )
// }

// if (!BACKEND_PORT) {
//   throw new Error(
//     "Please define the BACKEND_PORT environment variable inside .env.local"
//   )
// }

export async function connectToBackend() {
  console.log("Client init")
  const socket = io({
    path: "/api/socketio",
    transports: ["websocket", "polling"],
  })
  return socket
}
