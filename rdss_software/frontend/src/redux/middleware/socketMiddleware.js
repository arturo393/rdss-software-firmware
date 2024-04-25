import io from "socket.io-client"

export default function socketMiddleware() {
  // const socket = io(
  //   "http://" +
  //     process.env.NEXT_PUBLIC_SOCKETSHOST +
  //     ":" +
  //     process.env.NEXT_PUBLIC_SOCKETSPORT,
  //   {
  //     // transports: ["polling"],
  //     // transports: ["websocket"],
  //     // transports: ["websocket", "polling"],
  //   }
  // )
  // const socket = io("http://" + process.env.NEXT_PUBLIC_SOCKETSHOST + ":" + process.env.NEXT_PUBLIC_SOCKETSPORT, { forceBase64: true, preMessageDeflate: true })
  const socket = io("http://" + process.env.NEXT_PUBLIC_SOCKETSHOST + ":" + process.env.NEXT_PUBLIC_SOCKETSPORT, { forceBase64: true })

  console.log("=== INICIANDO SOCKETS MIDDLEWARE ===")

  return (store) => (next) => (action) => {
    if (typeof action === "function") {
      return next(action)
    }
    const { event, leave, handle, ...rest } = action
    if (!event) {
      return next(action)
    }

    if (leave) {
      socket.removeListener(event)
    }

    let handleEvent = handle
    if (typeof handleEvent === "string") {
      handleEvent = (result) => {
        store.dispatch({ type: handle, result, ...rest })
      }
    }

    return socket.on(event, handleEvent)
  }
}
