import React, { useEffect, useState } from "react"
import { connect } from "react-redux"

import { Container, Card, Button } from "react-bootstrap"
import axios from "axios"

import { setDevices } from "../../redux/actions/main"

const DevicesEdit = (props) => {
  const { devices, setDevices } = props

  const [device, setDevice] = useState({})
  const [deviceData, setDeviceData] = useState({})
  const [newDevices, setNewDevices] = useState([])
  const [status, setStatus] = useState()

  useEffect(() => {
    document.getElementById("status").style.display = "none"
  }, [])

  useEffect(() => {
    if (devices.length > 0) setNewDevices(devices)
  }, [devices])

  useEffect(() => {
    setDevices(newDevices)
  }, [newDevices])

  const handleDeviceSelected = (e) => {
    e.preventDefault()
    document.getElementById("status").style.display = "none"
    document.getElementById("name").value = ""
    document.getElementById("type").value = ""
    const deviceId = e.target.value
    if (deviceId) {
      const uri = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/" + deviceId
      axios
        .get(uri)
        .then(
          (result) => {
            return result.data[0]
          },
          (error) => {
            console.log(error)
          }
        )
        .then((res) => {
          setDeviceData(res)
        })
      // setDeviceData(devices.find((device) => device.id === Number(deviceId)))
    }
  }

  const saveDevice = (e) => {
    e.preventDefault()

    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/save", deviceData).then(
      (result) => {
        document.getElementById("status").style.display = "block"
        result ? setStatus("Device attributes updated successfully") : setStatus("Error when try to save device data")
        const objIndex = newDevices.findIndex((d) => d.id == parseInt(deviceData.id))
        let dev = devices.find((d) => d.id == parseInt(deviceData.id))
        dev = { ...dev, ...deviceData }
        let newDevicesList = devices
        newDevicesList[objIndex] = dev
        setNewDevices(newDevicesList)
      },
      (error) => {
        console.log(error)
      }
    )
  }

  const handleChange = (e) => {
    setDeviceData({
      ...deviceData,
      [e.target.id]: e.target.value,
    })
  }

  return (
    <>
      <h5 className="text-center">Device Editor</h5>
      <div class="container-fuid">
        <div class="row text-center">
          <div class="col-2"></div>
          <div class="col-7">
            <select className="form-control" id="device" onChange={handleDeviceSelected}>
              <option value={0}>=== Select a Device ===</option>
              {devices.map((device) => {
                return (
                  <option value={device.id}>
                    {device.name} ({device.type}-{device.id})
                  </option>
                )
              })}
            </select>
            <p></p>
            <div class="input-group mb-3">
              <div className="input-group mb-3">
                <span className="input-group-text">Name</span>
                <input type="text" className="form-control" id="name" value={deviceData?.name} onChange={handleChange} />
              </div>
              <div className="input-group mb-3">
                <span className="input-group-text">Type</span>
                <input type="text" className="form-control" id="type" value={deviceData?.type} onChange={handleChange} />
              </div>

              <button className="btn btn-primary" type="button" onClick={saveDevice}>
                Save
              </button>
            </div>
          </div>
          <div class="col-2"></div>
        </div>
      </div>
      <div className="row">
        <div className="col-md-12 text-center">
          <div className="alert alert-success" role="alert" id="status">
            {status}
          </div>
        </div>
      </div>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    devices: state.main.devices,
  }
}

const mapDispatchToProps = {
  setDevices,
}

export default connect(mapStateToProps, mapDispatchToProps)(DevicesEdit)
