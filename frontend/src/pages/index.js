import { useEffect } from "react"
import { connect } from "react-redux"
import { setName, setNameEvent } from "../redux/actions/main"
import { setCompany } from "../redux/actions/user"

import styles from "../styles/Home.module.css"
import axios from "axios"

function Home(props) {
  const { name, setName, setNameEvent, company, setCompany } = props

  useEffect(() => {
    setNameEvent()
    //setCompanyEvent()
    setCompany(props.users[0].company)
    setName(props.devices[0].name)
  }, [])

  return (
    <div className={styles.container}>
      <main>
        <div className="container-fluid">
          <div className="row">
            <div className="col-md-6">
              <h3>ACA VA EL COMPONENTE MAPÂ</h3>
            </div>
            <div className="col-md-6">
              <div className="row">
                <div className="col-md-12">
                  <h3>ACA VA EL RESUMEN DEL STATUS.</h3>
                </div>
              </div>
              <div className="row">
                <div className="col-md-12">ACA VAN LOS COMPONENTES DINÁMICOS</div>
              </div>
            </div>
          </div>
        </div>
      </main>
    </div>
  )
}

const mapStateToProps = (state) => {
  return {
    name: state.main.name,
    company: state.main.company,
  }
}

const mapDispatchToProps = {
  setName,
  setNameEvent,
  setCompany,
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
      devices,
    },
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
