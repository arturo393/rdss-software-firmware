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
  const { activeDeviceId, devices, fields } = props
  // const [devices, setDevices] = useState([])
  const [device, setDevice] = useState(activeDeviceId || 0)
  const [deviceData, setDeviceData] = useState({})
  const [deviceName, setDeviceName] = useState("=== Select a Device ===")


  const [dateFrom, handleDateFromChange] = useState(new Date(Date.now() - 3600 * 1000 * 6))
  const [dateTo, handleDateToChange] = useState(new Date())
  const [rtData, setRtData] = useState({})


  useEffect(() => {
    activeDeviceId && setDevice(activeDeviceId)
    hiddeSpinner()
  }, [])
  
  useEffect(() => {
    activeDeviceId && setDevice(activeDeviceId)
    handleDateFromChange(new Date(Date.now() - 3600 * 1000 * 6))
    handleDateToChange(new Date())

  }, [activeDeviceId])
  
  useEffect(() => {
    console.log("fields.length",fields.length)
    console.log("device",device)
    if (fields.length && device) {
      getDeviceRTData(device)
      document.getElementById("device").value = device
      const dev = devices.find((d) => Number(d.id) === Number(device))
      setDeviceName(dev.name ? dev.name + "(" + dev.type + "-" + dev.id + ")" : dev.type + "-" + dev.id) 
    }
         
  }, [device])
  

  const getDeviceRTData = async (device = 0) => {
    if (device > 0) { 
      showSpinner()
      console.log("Getting device data, ID=" + device)
      let x = []
      let rtd = {}
      let text = []
      let marker = {}
      marker.color = []

      const deviceReq = { id: parseInt(device), dateFrom: dateFrom, dateTo: dateTo }

      const res = await axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/deviceId", deviceReq)
      // console.log("RTDATA RES",res.data)
      fields?.map((field) => {
        rtd[field._id] = []
      })
      res.data?.map((data) => {
        const datetime = getDateTime(data?._id)
        x.push(datetime)
        Object.keys(data?.field_values).map((key) => {
          
          rtd[key]?.push(data?.field_values[key].value)
          marker.color.push(data?.field_values[key].alert?"red":"lightblue")
          let text_content = `<b>Device: ${data._id.device}</b><br>\n`
          text_content = data?.connected?"":"Disconnect"
          text_content += data?.field_values[key].alert?fields.find(f => key === f._id).name+" Out of Limits":"no alerts"
          text.push(text_content)
        })
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
    showSpinner()
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
  // console.log("RTDATA", rtData)

  return (
    <>
    <h5 className="text-center w-100 sigmaRed text-light">RT-Data {device > 0 && deviceName}</h5>
    
   
    <div className="containers text-center">
      <span className="spinnerContainer" id="spinnerContainer">
        <span>Loading...</span>
        <div className="spinner-border text-light" role="status" name="spinner" id="spinner" style={{ maxWidth: "50px", maxHeight: "50px" }}></div>
      </span>

      <div className="card h-100">
        <div className="card-body text-center">
          <div className="input-group mb-3">

          <span className="input-group-text" id="device-label">
              Device
            </span>
            <select className="form-select" id="device" onChange={(e) => setDevice(e.target.value)} value={device}>
              <option value={0}>{deviceName}</option>
              {devices.map((d) => {
                return (
                  <option value={d.id}>
                    {d.name} ({d.type}-{d.id})
                  </option>
                )
              })}
            </select>

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

            

            <button id="searchDevice" onClick={handleSearch} className="btn btn-primary">
              Search
            </button>
          </div>
          <div className="text-center mt-2 mb-2">
                
              </div>
          {(device > 0 && fields &&  rtData.x?.length > 0)? (
            <>
              
              {/* {console.log("RTDATA", rtData)} */}
              {
                  fields.filter(field => field.plottable).map(field => (
                    // rtData = { x, rtd, marker, text }
                    <div>
                    <Chart deviceId={device}  rtData={rtData} label={field.name} filter={field._id} color={field.color || "lightblue"} />
                    </div>
                  ))
              }
              
            </>
          ):(
            <div>No Data Found</div>
          )}
        </div>
      </div>
    </div>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    activeDeviceId: state.main.activeDeviceId,
    devices: state.main.devices,
    fields: state.main.fields
  }
}

export default connect(mapStateToProps)(Rtdata)
