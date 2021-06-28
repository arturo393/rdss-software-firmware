import { useState, useEffect } from "react"
import { connect } from "react-redux"
import { setName } from "../redux/actions/main"
import styles from "../styles/Home.module.css"

import io from "socket.io-client"

import Title from "../components/Title"

function Home(props) {
  const { name, setName } = props

  useEffect(() => {
    console.log("Client init")
    const socket = io("http://localhost:4200", {
      transports: ["websocket", "polling"],
    })
    socket.on("change_name", (data) => {
      setName(data)
    })
  }, [])

  useEffect(() => {
    console.log("NAME CHANGED TO: " + name)
  }, [name])

  return (
    <div className={styles.container}>
      <main>
        <h1 className={styles.title}>{name}</h1>
      </main>
      <footer>
        <Title />
      </footer>
    </div>
  )
}

const mapStateToProps = (state) => {
  return { name: state.main.name }
}
const mapDispatchToProps = {
  setName,
}

export async function getInitialProps() {
  console.log("GetInitialProps")
  const reqDevices = await fetch(`http://localhost:3000/api/devices`)
  const resDevices = await reqDevices.json()
  const devices = await Object.entries(resDevices)
  return {
    props: {
      devices,
    },
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
