import React, { useEffect, useState } from "react"
import { Container, Card, Table } from "react-bootstrap"
import { setMonitorDataEvent } from "../redux/actions/main"
import axios from "axios"
import { connect } from "react-redux"
import green from "../images/green.svg"
import red from "../images/red.svg"
import gray from "../images/gray.svg"

let a_connected
let a_voltage
let a_current
let a_agcup
let a_agcdown
let a_ptx
let a_smartTune
let a_reverse
let a_attenuation

const Alerts = (props) => {
  const [alerts, setAlerts] = useState([])
  const [deviceData, setDeviceData] = useState({})
  const { monitorData } = props
  useEffect(() => {
    let currentAlerts = []
    monitorData?.map((monitor) => {
      const data = JSON.parse(monitor)

      console.log(data)
      if (data.connected) {
        a_connected = green.src
        a_voltage = data.alerts.voltage ? red.src : green.src
        a_current = data.alerts.current ? red.src : green.src
        a_agcup = data.alerts.gupl ? red.src : green.src
        a_agcdown = data.alerts.gdwl ? red.src : green.src
        a_ptx = data.alerts.power ? red.src : green.src
      } else {
        a_connected = red.src
        a_voltage = gray.src
        a_current = gray.src
        a_agcup = gray.src
        a_agcdown = gray.src
        a_ptx = gray.src
      }

      const device = {
        id: data.id,
        name: data.name,
        type: data.type,
        connected: a_connected,
        voltage: a_voltage,
        current: a_current,
        gupl: a_agcup,
        guwl: a_agcdown,
        power: a_ptx,
        smartTune: data.rtData.smartTune!=undefined?(data.rtData.smartTune == "ON"?green.src:gray.src):gray.src,
        reverse: data.rtData.reverse!=undefined?(data.rtData.reverse == "ON"?green.src:gray.src):gray.src,
        attenuation: data.rtData.attenuation
      }
      currentAlerts.push(device)
    })
    setAlerts(currentAlerts)
  }, [monitorData])

  const handleChange = (e) => {
    e.preventDefault()
    setDeviceData({
      [e.target.id]: e.target.value,
    })
  }
  const saveDevice = (e) => {
    e.preventDefault()
    let new_attenuation = e.target.attenuation.value || -1
    if (new_attenuation < 0 || new_attenuation > 32) {
      let resultado = document.getElementById(e.target.id.value+"status")
      resultado.style.display = "block"
      resultado.innerHTML = "Value should be between 0 and 32"
      return false
    }

    axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/" + e.target.id.value)
                      .then(
                        (result) => {
                          return result.data[0]
                        },
                        (error) => {
                          console.log(error)
                        }
                      )
                      .then((device) => {
                        device.attenuation = new_attenuation  * 1 // ensure integer
                        let status = document.getElementById(e.target.id.value+"status")
                        let message
                        axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/save", device).then(
                          (result) => {
                            message = "Device updated successfully"
                          },
                          (error) => {
                            message = "Error saving device data"
                            console.log(error)
                          }
                        ).then(() => {
                          status.style.display = "block"
                          status.innerHTML = message
                        })
                      })

  }
  return (
    <>
      <h5 className="text-center">Devices Status</h5>
      <div className="container table-responsive text-center ">
        {/* CONTENIDO */}
        <div className="row">
          <table className="table table-striped table-bordered table-light">
            <thead>
              <tr>
                <th>
                  <h6>Device</h6>
                </th>
                <th>
                  <h6>Connection</h6>
                </th>
                <th>
                  <h6>Voltage</h6>
                </th>
                <th>
                  <h6>Current</h6>
                </th>
                <th>
                  <h6>AGC Uplink</h6>
                </th>
                <th>
                  <h6>AGC Downlink</h6>
                </th>
                <th>
                  <h6>Downlink Power</h6>
                </th>
                <th>
                  <h6>Smart Tune</h6>
                </th>
                <th>
                  <h6>Reverse</h6>
                </th>
                <th>
                  <h6>Downlink Attenuation</h6>
                </th>
                
              </tr>
            </thead>
            <tbody>
              {alerts?.map((data) => {
                return (
                  <tr>
                    <td>{data.name + "(" + data.type + "-" + data.id + ")"}</td>
                    <td>
                      <img alt="" src={data.connected} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.voltage} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.current} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.gupl} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.guwl} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.power} width={20} height={20} />
                    </td>
                    <td>
                    <img alt="" src={data.smartTune} width={20} height={20} />
                    </td>
                    <td>
                    <img alt="" src={data.reverse} width={20} height={20} />
                    </td>
                    <td>
                      <form onSubmit={saveDevice}>
                        <div className="input-group mb-3">
                          <input type="number" className="form-control" id="attenuation" value={data?.attenuation} onChange={handleChange} />
                          <button className="btn btn-primary" type="submit">Save</button>
                          <input type="hidden" name="id" id="id" value={data.id}/>
                        </div>
                        <div className="row">
                          <div className="col-md-12 text-center">
                            <div className="alert alert-info hidden" role="alert" id={data.id+"status"}>OK
                            </div>
                          </div>
                        </div>
                      </form>
                    </td>
                  </tr>
                )
              })}
            </tbody>
          </table>
        </div>
        {/* FIN CONTENIDO */}
      </div>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Alerts)
