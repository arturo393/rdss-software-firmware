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
        a_agcdown = data.alerts.guwl ? red.src : green.src
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
    <div className="containers">
      <div className="text-center mt-2 mb-2">
        <h5>Vlad Status</h5>
      </div>
      <div className="card h-100">
        <div className="card-body table-responsive">
          {/* CONTENIDO */}
          <table className="table table-striped table-bordered table-light">
            <thead>
              <tr>
                <th>Vlad ID</th>
                <th>Connection</th>
                <th>Voltage</th>
                <th>Current</th>
                <th>AGC Up</th>
                <th>AGC Down</th>
                <th>Ptx</th>
              </tr>
            </thead>
            <tbody>
              {alerts?.map((data) => {
                return (
                  <tr>
                    <td>{data.id}</td>
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
          {/* FIN CONTENIDO */}
        </div>
      </div>
    </div>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Alerts)
