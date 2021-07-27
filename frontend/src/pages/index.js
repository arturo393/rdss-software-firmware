import axios from "axios"

import { useEffect } from "react"

import Schema from "../components/Schema"
import Status from "../components/common/Status"
import Login from "../components/auth/Login"
import { connect } from "react-redux"
import { setConfig, setDevices } from "../redux/actions/main"
import DynamicComponent from "../components/DynamicComponent"

const Home = (props) => {
  const { isLoggedIn, dbConfig, dbDevices, setConfig, setDevices } = props

  useEffect(() => {
    setConfig(dbConfig)
    setDevices(dbDevices)
  }, [])

  if (isLoggedIn) {
    return (
      <main className="container-fluid">
        s{" "}
        <div className="col-md-12">
          <div className="row">
            <div className="col-md-7" id="myMap">
              <div className="row">
                <Schema />
              </div>
            </div>
            <div className="col-md-5">
              <div className="row">
                <Status />
              </div>
              <div className="row">
                <div className="col-md-12">
                  <DynamicComponent activeComponent="alerts" />
                </div>
              </div>
            </div>
          </div>
        </div>
      </main>
    )
  } else return <Login />
}
export async function getServerSideProps(context) {
  const dbConfig = await axios
    .get("http://localhost:3000/api/manage/config")
    .then((res) => {
      return res.data[0]
    })
  const dbDevices = await axios
    .get("http://localhost:3000/api/devices/devices")
    .then((res) => {
      return res.data
    })

  return {
    props: { dbConfig, dbDevices },
  }
}

const mapDispatchToProps = { setConfig, setDevices }

const mapStateToProps = (state) => {
  return {
    isLoggedIn: state.user.isLoggedIn,
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
