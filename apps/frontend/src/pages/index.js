import { useState, useEffect } from "react"
import { connect } from "react-redux"
import { setName, setNameEvent } from "../redux/actions/main"
import styles from "../styles/Home.module.css"
import axios from "axios"

import Title from "../components/Title"
function Home(props) {
  console.log(props)
  const { name, setName, setNameEvent } = props
  const { message, setMessage } = useState()

  useEffect(() => {
    console.log("INIT PAGE")
    setNameEvent()
    setName(props.devices[0].name)
  }, [])

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
  setNameEvent,
}

export async function getServerSideProps() {
  console.log("GetProps")
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
