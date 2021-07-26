import axios from "axios"

import { useEffect } from "react"

import Schema from "../components/Schema"
import Status from "../components/common/Status"

import { connect } from "react-redux"
import { setConfig, setDevices } from "../redux/actions/main"
import DynamicComponent from "../components/DynamicComponent"

const Home = (props) => {
  const { dbConfig, dbDevices, setConfig, setDevices } = props

  useEffect(() => {
    setConfig(dbConfig)
    setDevices(dbDevices)
  }, [])

  return (
    <main>
      <div className="container-fluid">
        <div className="row">
          <div className="col-md-7" id="myMap">
            <Schema />
          </div>
          <div className="col-md-5">
            <div className="row">
              <Status />
            </div>
            <div className="row">
              <div className="col-md-12 lightBgColor hundredHeight">
                <DynamicComponent activeComponent="alerts" />
              </div>
            </div>
          </div>
        </div>
      </div>
    </main>
  )
}

export async function getServerSideProps() {
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
    // config: state.main.config,
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
