import axios from "axios"

import { useEffect } from "react"

import Schema from "../components/Schema"
// import Status from "../components/common/Status"
import Login from "../components/auth/Login"
import { connect } from "react-redux"
import { setConfig, setDevices, setMonitorDataEvent } from "../redux/actions/main"
import DynamicComponent from "../components/DynamicComponent"

const Home = (props) => {
  const { isLoggedIn, dbConfig, dbDevices, setConfig, setDevices, setMonitorDataEvent } = props

  useEffect(() => {
    setConfig(dbConfig)
    setDevices(dbDevices)
    setMonitorDataEvent()
  }, [])

  if (isLoggedIn) {
    return (
      <main className="container-fluid">
        <div className="row">
          <DynamicComponent activeComponent="map" />
        </div>
      </main>
    )
  } else return <Login />
}
export async function getServerSideProps(context) {
  const dbConfig = await axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/config").then((res) => {
    return res.data[0]
  })
  const dbDevices = await axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/devices").then((res) => {
    return res.data
  })

  return {
    props: { dbConfig, dbDevices },
  }
}

const mapDispatchToProps = { setConfig, setDevices, setMonitorDataEvent }

const mapStateToProps = (state) => {
  return {
    isLoggedIn: state.user.isLoggedIn,
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
