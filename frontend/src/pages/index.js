import axios from "axios"

import { useEffect } from "react"

import Schema from "../components/Schema"
// import Status from "../components/common/Status"
import Login from "../components/auth/Login"
import { connect } from "react-redux"

import { setConfig, setDevices, setMonitorDataEvent ,setdbDevicestype, setFields} from "../redux/actions/main"
import DynamicComponent from "../components/DynamicComponent"

const Home = (props) => {
  const { isLoggedIn, dbConfig, dbDevices,dbDevicestype,setdbDevicestype, setConfig, setDevices, setMonitorDataEvent, setFields, dbFields } = props

  useEffect(() => {
    setConfig(dbConfig)
    setDevices(dbDevices)
    setdbDevicestype(dbDevicestype)
    setMonitorDataEvent()
    setFields(dbFields)
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

  const url = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT

  const dbConfig = await axios.get(url + "/api/manage/config").then((res) => {
    return res.data[0]
  })
  const dbDevices = await axios.get(url + "/api/devices/devices").then((res) => {
    return res.data
  })
  const dbDevicestype = await axios.get(url + "/api/devices/devicestype").then((res) => {
    return res.data
  })

  const dbFields = await axios.get(url + "/api/fields").then((res) => {
    return res.data
  })

  console.log("dbFields", dbFields)

  return {
    props: { dbConfig, dbDevices , dbDevicestype, dbFields},
  }
}

const mapDispatchToProps = { setConfig, setDevices,setdbDevicestype, setMonitorDataEvent, setFields }

const mapStateToProps = (state) => {
  return {
    isLoggedIn: state.user.isLoggedIn,
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
