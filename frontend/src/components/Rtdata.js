import React, { useEffect, useState, Fragment } from "react"
import axios from "axios"
import { connect } from "react-redux"
// import DeviceGraphs from "./DeviceGraphs"
import { DateTime } from "luxon"

import LuxonUtils from "@date-io/luxon"
import { DateTimePicker, MuiPickersUtilsProvider } from "@material-ui/pickers"
import { createTheme } from "@material-ui/core/styles"
import { ThemeProvider } from "@material-ui/styles"
import { alpha } from "@material-ui/core/styles"

const theme = createTheme({
  palette: {
    primary: { 500: "#006486" },
  },
})

// import Chart from "./Chart"
import dynamic from "next/dynamic"
const Chart = dynamic(import("./Chart"), {
  ssr: false,
})

const Rtdata = (props) => {
  const defaultNullValue = 0
  const { activeDeviceId, devices } = props
  // const [devices, setDevices] = useState([])
  const [device, setDevice] = useState(0)
  const [deviceData, setDeviceData] = useState({})
  const [deviceName, setDeviceName] = useState("=== Select a Device ===")

  const [dateFrom, handleDateFromChange] = useState(new Date(Date.now() - 3600 * 1000 * 6))
  const [dateTo, handleDateToChange] = useState(new Date())
  const [rtData, setRtData] = useState({})

  useEffect(() => {
    // axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/devices").then((res) => {
    //   const devices = res.data
    //   setDevices(devices)
    // })
    if (activeDeviceId) {
      document.getElementById("device").value = activeDeviceId
      setDevice(activeDeviceId)
      getDeviceRTData(activeDeviceId)
    }
    hiddeSpinner()
  }, [])

  useEffect(() => {
    document.getElementById("device").value = activeDeviceId
    if (activeDeviceId) {
      const dev = devices.find((d) => Number(d.id) === Number(activeDeviceId))
      setDeviceName(dev.name ? dev.name + "(" + dev.type + "-" + dev.id + ")" : dev.type + "-" + dev.id)
    }
    handleDateFromChange(new Date(Date.now() - 3600 * 1000 * 6))
    handleDateToChange(new Date())
    // console.log("HOLA")
    // hiddeSpinner()
  }, [activeDeviceId])

  useEffect(() => {
    if (device > 0) {
      showSpinner()
      getDeviceRTData(device)
    }
  }, [device, dateFrom, dateTo])

  const getDeviceRTData = () => {
    if (device) {
      console.log("Getting device data, ID=" + device)
      let x = []
      let rtd = {}
      let voltage = []
      let current = []
      let power = []

      let text = []
      let marker = {}
      marker.color = []

      let tempData = []
      const deviceReq = { id: parseInt(device), dateFrom: dateFrom, dateTo: dateTo }

      tempData = axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/deviceId", deviceReq).then((res) => {
        return res.data
      })

      tempData.then((data) => {
        let lastValues = { voltage: 0, current: 0, power: 0 }

        data.map((d) => {
          const datetime = getDateTime(d._id)
          x.push(datetime)

          voltage.push(d.voltage?.toFixed(2) || lastValues.voltage)
          current.push(d.current?.toFixed(2) || lastValues.current)
          //Fixes POWER tolerance
          //if (d.power?.toFixed(2) <= -5) d.power = -5
          power.push(d.power?.toFixed(2) || lastValues.power)
          lastValues.voltage = voltage.at(-1)
          lastValues.current = current.at(-1)
          lastValues.power = power.at(-1)

          let connectionAlert = false
          let voltageAlert = false
          let currentAlert = false
          let powerAlert = false
          let guplAlert = false
          let gdwlAlert = false
          d.alerts?.map((alert) => {
            connectionAlert |= alert.connection
            voltageAlert |= alert.voltage
            currentAlert |= alert.current
            powerAlert |= alert.power
            guplAlert |= alert.gupl
            gdwlAlert |= alert.gdwl
          })
          let t = "<b>Device: " + device + "</b><br>\n"
          if (connectionAlert) t += " * Connection Alert<br>\n"
          if (voltageAlert) t += " * Voltage Alert<br>\n"
          if (currentAlert) t += " * Current Alert<br>\n"
          if (powerAlert) t += " * Power Alert<br>\n"
          if (guplAlert) t += " * Gupl Alert<br>\n"
          if (gdwlAlert) t += " * Gdwl Alert<br>\n"
          if (connectionAlert || voltageAlert || currentAlert || powerAlert || guplAlert || gdwlAlert) marker.color.push("red")
          else marker.color.push("lightblue") //default color

          text.push(t)
        })

        rtd.voltage = voltage || []
        rtd.current = current || []
        rtd.power = power || []

        setRtData({ x, rtd, marker, text })
        hiddeSpinner()
      })
    }
  }

  const getDateTime = (obj) => {
    let dateString = obj.year

    if (obj.hour === undefined) obj.hour = 0
    if (obj.minute === undefined) obj.minute = 0
    if (obj.second === undefined) obj.second = 0

    obj.hour = ("0" + obj.hour).slice(-2) || "00"
    obj.minute = ("0" + obj.minute).slice(-2) || "00"
    obj.second = ("0" + obj.second).slice(-2) || "00"

    dateString += "-" + obj.month
    dateString += "-" + obj.day
    dateString += " " + obj.hour
    dateString += ":" + obj.minute
    dateString += ":" + obj.second

    return dateString
  }

  const handleSearch = (e) => {
    e.preventDefault()
    const deviceSelector = document.getElementById("device")
    if (deviceSelector.value > 0) {
      setDevice(deviceSelector.selectedIndex)
      const dev = devices.find((d) => Number(d.id) === Number(deviceSelector.value))
      setDeviceName(dev.name ? dev.name + "(" + dev.type + "-" + dev.id + ")" : dev.type + "-" + dev.id)
      setDeviceData(devices.find((d) => d.id === deviceSelector.selectedIndex))
    }
  }

  const hiddeSpinner = () => {
    let spinner = document.getElementById("spinnerContainer")
    if (spinner) {
      spinner.style.visibility = "hidden"
      spinner.style.opacity = 0
    }
  }

  const showSpinner = () => {
    let spinner = document.getElementById("spinnerContainer")
    if (spinner) {
      spinner.style.visibility = "visible"
      spinner.style.opacity = 1
    }
  }

  return (
    <div className="containers text-center">
      <span className="spinnerContainer" id="spinnerContainer">
        <span>Loading...</span>
        <div className="spinner-border text-light" role="status" name="spinner" id="spinner" style={{ maxWidth: "50px", maxHeight: "50px" }}></div>
      </span>

      <div className="card h-100">
        <div className="card-body text-center">
          <div className="input-group mb-3">
            <ThemeProvider theme={theme}>
              <MuiPickersUtilsProvider utils={LuxonUtils}>
                {/* <div class="input-group mb-3"> */}
                <span class="input-group-text" id="datetime1">
                  From:
                </span>
                <DateTimePicker class="form-control" aria-label="dateFrom" aria-describedby="datetime1" variant="inline" value={dateFrom} onChange={handleDateFromChange} />

                <span class="input-group-text" id="datetime2">
                  To:
                </span>
                <DateTimePicker class="form-control" aria-label="dateTo" aria-describedby="datetime2" variant="inline" value={dateTo} onChange={handleDateToChange} />
                {/* </div> */}
              </MuiPickersUtilsProvider>
            </ThemeProvider>

            <span className="input-group-text" id="device-label">
              Device
            </span>
            <select className="form-select" id="device">
              <option value={0}>{deviceName}</option>
              {devices.map((device) => {
                return (
                  <option value={device.id}>
                    {device.name} ({device.type}-{device.id})
                  </option>
                )
              })}
            </select>

            <button id="searchDevice" onClick={handleSearch} className="btn btn-primary">
              Search
            </button>
          </div>
          {hiddeSpinner()}
          {device > 0 && (
            <>
              <div className="text-center mt-2 mb-2">
                <h5>RT-Data: {deviceName}</h5>
              </div>
              <Chart deviceId={device} rtData={rtData} label={"Voltage"} filter="voltage" color="lightblue" />
              <br />
              <Chart deviceId={device} rtData={rtData} label={"Current"} filter="current" color="green" />
              <br />
              <Chart deviceId={device} rtData={rtData} label={"Power"} filter="power" color="orange" />
            </>
          )}
        </div>
      </div>
    </div>
  )
}

const mapStateToProps = (state) => {
  return {
    activeDeviceId: state.main.activeDeviceId,
    devices: state.main.devices,
  }
}

export default connect(mapStateToProps)(Rtdata)
