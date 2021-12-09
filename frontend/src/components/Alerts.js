import React, { useEffect, useState } from "react"
import { Container, Card, Table } from "react-bootstrap"
import { setMonitorDataEvent } from "../redux/actions/main"
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

const Alerts = (props) => {
  const [alerts, setAlerts] = useState([])
  const { monitorData } = props
  useEffect(() => {
    let currentAlerts = []

    monitorData?.map((monitor) => {
      const data = JSON.parse(monitor)

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
      }
      currentAlerts.push(device)
    })
    setAlerts(currentAlerts)
  }, [monitorData])

  return (
    <>
      <h5 className="text-center">Vlad Status</h5>
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
                  <h6>AGC Up</h6>
                </th>
                <th>
                  <h6>AGC Down</h6>
                </th>
                <th>
                  <h6>Ptx</h6>
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
