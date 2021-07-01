import { useState, useEffect } from "react"
import { connect } from "react-redux"
import { setName, setNameEvent } from "../redux/actions/main"
import styles from "../styles/Home.module.css"
import axios from "axios"

import MyComponent from "../components/MyComponent"
import TablaNombres from "../components/TablaNombres"
function Home(props) {
  const { name, setName, setNameEvent } = props

  useEffect(() => {
    setNameEvent()
    setName(props.devices[0].name)
  }, [])

  return (
    <div className={styles.container}>
      <main>
        Página principal: Valor leído desde la base de datos:
        <h1 className={styles.title}>{name}</h1>
        <h2>RDSSv2</h2>
        <TablaNombres />
      </main>
      <footer>
        <MyComponent />
      </footer>
    </div>
  )
}

const mapStateToProps = (state) => {
  return { name: state.main.name }
}
const mapDispatchToProps = {
  setName,
  setNameEvent,
}

export async function getServerSideProps() {
  const devices = await axios
    .get("http://localhost:3000/api/devices")
    .then((res) => {
      return res.data
    })
  return {
    props: {
      devices,
    },
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
