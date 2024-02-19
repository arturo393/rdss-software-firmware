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

  const [fields,setFields] = useState([])

  const [dateFrom, handleDateFromChange] = useState(new Date(Date.now() - 3600 * 1000 * 6))
  const [dateTo, handleDateToChange] = useState(new Date())
  const [rtData, setRtData] = useState({})

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`;


  useEffect(() => {
    // axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/devices").then((res) => {
    //   const devices = res.data
    //   setDevices(devices)
    // })

    const getFieldsData = async () => {
      const res = await axios.get(`${url}/api/fields`);
      setFields(res.data.filter(field => field.plottable ))
    }

    if (activeDeviceId) {
      document.getElementById("device").value = activeDeviceId
      setDevice(activeDeviceId)
    }
    getFieldsData()
    hiddeSpinner()
  }, [])


  useEffect(() => {
    document.getElementById("device").value = activeDeviceId
    if (activeDeviceId) {
      setDevice(activeDeviceId)
      // getDeviceRTData(activeDeviceId)
      const dev = devices.find((d) => Number(d.id) === Number(activeDeviceId))
      setDeviceName(dev.name ? dev.name + "(" + dev.type + "-" + dev.id + ")" : dev.type + "-" + dev.id)
    }
    handleDateFromChange(new Date(Date.now() - 3600 * 1000 * 6))
    handleDateToChange(new Date())
  }, [activeDeviceId])

  useEffect(() => {
    if (device > 0) {
      showSpinner()
      getDeviceRTData(device)
    }
  }, [device, dateFrom, dateTo])


  const getDeviceRTData = async (device = 0) => {
    if (device) {
      console.log("Getting device data, ID=" + device)
      let x = []
      let rtd = {}
      let text = []
      let marker = {}
      marker.color = []

      const deviceReq = { id: parseInt(device), dateFrom: dateFrom, dateTo: dateTo }

      const res = await axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/deviceId", deviceReq)
      /**
       * 
       [
    {
        "_id": {
            "device": 8,
            "year": 2024,
            "month": 2,
            "day": 15,
            "hour": 16,
            "minute": null,
            "second": null
        },
        "connected": false,
        "field_values": {
            "65d1474507a3be0013ede784": {
                "value": 6.666666666666667,
                "alert": true
            }
        }
    },
    {
        "_id": {
            "device": 8,
            "year": 2024,
            "month": 2,
            "day": 18,
            "hour": 21,
            "minute": null,
            "second": null
        },
        "connected": false,
        "field_values": {
            "65d1474507a3be0013ede784": {
                "value": 7.333333333333333,
                "alert": false
            }
        }
    }
]
       */
      
    console.log("RTDATA RES DATA", res.data)
    console.log("RTDATA FIELDS", fields)

    fields.map((field) => {
      rtd[field._id] = []
    })

    res.data.map((data) => {
      const datetime = getDateTime(data?._id)
      x.push(datetime)
      Object.keys(data?.field_values).map((key) => {
        rtd[key].push(data?.field_values[key].value)
        marker.color.push(data?.field_values[key].alert?"red":"lightblue")
        let text_content = `<b>Device: ${data._id.device}</b><br>\n`
        text_content = data?.connected?"":"Disconnect"
        text_content += data?.field_values[key].alert?fields.find(f => key === f._id).name+" Out of Limits":"no alerts"
        text.push(text_content)
      })
  
      

        // data.map((d) => {
        //   console.log("D",d)
          
        //   const datetime = getDateTime(d._id)
        //   x.push(datetime)
          
          
        //   let t = "<b>Device: " + device + "</b><br>\n"
        //   if (connectionAlert) t += " * Disconnect<br>\n"
        //   if (voltageAlert) t += " * Voltage Out of Limits<br>\n"
        //   if (currentAlert) t += " * Current Out of Limits<br>\n"
        //   if (powerAlert) t += " * Downlink Power Out of Limits<br>\n"
        //   if (guplAlert) t += " * AGC Uplink Out of Limits<br>\n"
        //   if (gdwlAlert) t += " * AGC Downlink Out of Limits<br>\n"
        //   if (connectionAlert || voltageAlert || currentAlert || powerAlert || guplAlert || gdwlAlert) marker.color.push("red")
        //   else marker.color.push("lightblue") //default color

        //   text.push(t)
        // })

        // rtd.voltage = voltage || []
        // rtd.current = current || []
        // rtd.power = power || []

        
      })

      setRtData({ x, rtd, marker, text })
      hiddeSpinner()
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

  console.log("RTDATA rtData",rtData)

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
              {fields && fields.filter(field => field.plottable).map(field => (
                // rtData = { x, rtd, marker, text }
                <div>
                <Chart deviceId={device}  rtData={rtData} label={field.name} filter={field._id} color={field.color || "lightblue"} />
                </div>
              ))}
              {/* <Chart deviceId={device} rtData={rtData} label={"Voltage"} filter="voltage" color="lightblue" />
              <br />
              <Chart deviceId={device} rtData={rtData} label={"Current"} filter="current" color="green" />
              <br />
              <Chart deviceId={device} rtData={rtData} label={"Downlink Power"} filter="power" color="orange" /> */}
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
