import { useEffect } from "react"
import { connect } from "react-redux"
import { setName, setNameEvent } from "../redux/actions/main"
import { setCompany } from '../redux/actions/user'

import styles from "../styles/Home.module.css"
import axios from "axios"

function Home(props) {
  const 
    { 
      name, 
      setName, 
      setNameEvent,
      company,
      setCompany
    } = props


  useEffect(() => {
    setNameEvent()
    //setCompanyEvent()
    setCompany(props.users[0].company)
    setName(props.devices[0].name)
  }, [])

  return (
    <div className={styles.container}>
      <main>
        {/* Página principal: Valor leído desde la base de datos:
        <h1 className={styles.title}>{company}</h1>
        <h1 className={styles.title}>{name}</h1> */}
        <h2>Home: RDSSv2</h2>
      </main>
    </div>
  )
}

const mapStateToProps = (state) => {
  return { 
    name: state.main.name,
    company: state.main.company 
  }
}

const mapDispatchToProps = {
  setName,
  setNameEvent,
  setCompany
}


export async function getServerSideProps() {
  const users = await axios
    .get("http://localhost:3000/api/auth/user")
    .then((res) => {
      return res.data
    })

  const devices = await axios
    .get("http://localhost:3000/api/devices/devices")
    .then((res) => {
      return res.data
    })

  return {
    props: {
      users,
      devices
    },
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
