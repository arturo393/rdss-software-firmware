import React, { useEffect, useState } from "react"
import axios from "axios"

import DeviceGraphs from "./DeviceGraphs"

const Rtdata = (props) => {
  const [devices, setDevices] = useState([])
  const [device, setDevice] = useState(0)

  useEffect(() => {
    axios.get("http://localhost:3000/api/devices/devices").then((res) => {
      const devices = res.data
      setDevices(devices)
    })
  }, [])

  const setSelectedDevice = (e) => {
    e.preventDefault()
    const deviceSelector = document.getElementById("device")
    setDevice(deviceSelector.selectedIndex)
  }

  useEffect(() => {
    console.log("OBTENIENDO LOS DATOS DEL DEVICE" + device)
  }, [devices])

  return (
    <div className="containers">
      <div className="text-center mt-2 mb-2">
        <h5>RT-Data</h5>
      </div>
      <div class="card h-100">
        <div className="card-body text-center">
          <div class="input-group mb-3">
            <span class="input-group-text" id="device-label">
              Device
            </span>
            <select className="form-select" id="device">
              <option value={0}>=== Select a device ===</option>
              {devices.map((device) => {
                return (
                  <option value={device.id}>
                    {device.type}
                    {device.id}
                  </option>
                )
              })}
            </select>
            <button onClick={setSelectedDevice} className="btn btn-primary">
              Search
            </button>
          </div>
          <DeviceGraphs deviceId={device} />
        </div>
      </div>
    </div>
  )
}

export default Rtdata
