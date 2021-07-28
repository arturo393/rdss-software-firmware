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
  const socket = io(
    "http://" +
      process.env.NEXT_PUBLIC_SOCKETSHOST +
      ":" +
      process.env.NEXT_PUBLIC_SOCKETSPORT
  )

  console.log("=== INICIANDO SOCKETS MIDDLEWARE ===")
  console.log(socket)

  return (store) => (next) => (action) => {
    if (typeof action === "function") {
      return next(action)
    }
    const { event, leave, handle, ...rest } = action
    console.log(action)
    console.log(event)
    console.log(handle)

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
