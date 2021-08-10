import React, { useEffect, useState } from "react"
import axios from "axios"
import { connect } from "react-redux"
import DeviceGraphs from "./DeviceGraphs"

import Chart from "./Chart"

const Rtdata = (props) => {
  const { activeDeviceId } = props
  const [devices, setDevices] = useState([])
  const [device, setDevice] = useState(0)
  const [deviceName, setDeviceName] = useState("=== Select a Device ===")

  useEffect(() => {
    axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/devices").then((res) => {
      const devices = res.data
      setDevices(devices)
    })

    if (activeDeviceId != undefined) {
      document.getElementById("device").value = activeDeviceId
      setDevice(activeDeviceId)
    }
  }, [])

  const setSelectedDevice = (e) => {
    e.preventDefault()
    const deviceSelector = document.getElementById("device")
    setDevice(deviceSelector.selectedIndex)
  }

  useEffect(() => {
    // console.log("GETTING DEVICES LIST: " + device)
  }, [devices])

  useEffect(() => {
    document.getElementById("device").value = activeDeviceId
    setDevice(activeDeviceId)
    if (activeDeviceId) setDeviceName("vlad" + activeDeviceId)
  }, [activeDeviceId])

  return (
    <div className="containers">
      <div className="text-center mt-2 mb-2">
        <h5>RT-Data</h5>
      </div>
      <div className="card h-100">
        <div className="card-body text-center">
          <div className="input-group mb-3">
            <span className="input-group-text" id="device-label">
              Device
            </span>
            <select className="form-select" id="device">
              <option value={0}>{deviceName}</option>
              {devices.map((device) => {
                return (
                  <option value={device.id}>
                    {device.type}
                    {device.id}
                  </option>
                )
              })}
            </select>
            <button id="searchDevice" onClick={setSelectedDevice} className="btn btn-primary">
              Search
            </button>
          </div>
          <Chart deviceId={device}  />
          {/* <DeviceGraphs deviceId={device} /> */}
        </div>
      </div>
    </div>
  )
}

const mapStateToProps = (state) => {
  return {
    activeDeviceId: state.main.activeDeviceId,
  }
}

export default connect(mapStateToProps)(Rtdata)
