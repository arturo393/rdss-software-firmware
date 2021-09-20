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
  const { activeDeviceId } = props
  const [devices, setDevices] = useState([])
  const [device, setDevice] = useState(0)
  const [deviceName, setDeviceName] = useState("=== Select a Device ===")

  const [dateFrom, handleDateFromChange] = useState(new Date(Date.now() - 3600 * 1000 * 6))
  const [dateTo, handleDateToChange] = useState(new Date())
  const [rtData, setRtData] = useState({})

  useEffect(() => {
    axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/devices").then((res) => {
      const devices = res.data
      setDevices(devices)
    })

    if (activeDeviceId) {
      document.getElementById("device").value = activeDeviceId
      setDevice(activeDeviceId)
      getDeviceRTData(activeDeviceId)
    }
  }, [])

  useEffect(() => {
    document.getElementById("device").value = activeDeviceId
    if (activeDeviceId) {
      setDeviceName("vlad" + activeDeviceId)
      setDevice(activeDeviceId)
    }
  }, [activeDeviceId])

  useEffect(() => {
    getDeviceRTData(device)
  }, [device, dateFrom, dateTo])

  const getDeviceRTData = () => {
    if (device) {
      console.log("OBTENIENDO DATOS DE:" + device)
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
      })
    }
  }

  const getDateTime = (obj) => {
    let dateString = obj.year

    const hour = obj.hour || "00"
    const minute = obj.minute || "00"
    const second = obj.second || "00"

    dateString += "-" + obj.month
    dateString += "-" + obj.day
    dateString += " " + hour
    dateString += ":" + minute
    dateString += ":" + second

    return dateString
  }

  const handleSearch = (e) => {
    e.preventDefault()
    const deviceSelector = document.getElementById("device")
    setDevice(deviceSelector.selectedIndex)
  }

  return (
    <div className="containers">
      <div className="text-center mt-2 mb-2">
        <h5>RT-Data</h5>
      </div>
      <div className="card h-100">
        <div className="card-body text-center">
          <ThemeProvider theme={theme}>
            <MuiPickersUtilsProvider utils={LuxonUtils}>
              <div class="input-group mb-3">
                <span class="input-group-text" id="datetime1">
                  From:
                </span>
                <DateTimePicker class="form-control" aria-label="dateFrom" aria-describedby="datetime1" variant="inline" value={dateFrom} onChange={handleDateFromChange} />

                <span class="input-group-text" id="datetime2">
                  To:
                </span>
                <DateTimePicker class="form-control" aria-label="dateTo" aria-describedby="datetime2" variant="inline" value={dateTo} onChange={handleDateToChange} />
              </div>
            </MuiPickersUtilsProvider>
          </ThemeProvider>
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

            <button id="searchDevice" onClick={handleSearch} className="btn btn-primary">
              Search
            </button>
          </div>
          <Chart deviceId={device} rtData={rtData} label={"Voltage"} filter="voltage" color="lightblue" />
          <Chart deviceId={device} rtData={rtData} label={"Current"} filter="current" color="green" />
          <Chart deviceId={device} rtData={rtData} label={"Power"} filter="power" color="orange" />

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
