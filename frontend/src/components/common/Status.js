import { useEffect, useState } from "react"
import { connect } from "react-redux"

const Status = (props) => {
  const { monitorData, devices } = props
  const [status, setStatus] = useState({
    connected: "",
    network: "",
    alert: "",
  })

  useEffect(() => {
    var spinners = document.getElementsByName("spinner")
    for (let i = 0; i < spinners.length; i++) {
      spinners[i].style.visibility = "hidden"
    }
  }, [status])

  useEffect(() => {
    var spinners = document.getElementsByName("spinner")
    for (let i = 0; i < spinners.length; i++) {
      spinners[i].style.visibility = "visible"
    }
  }, [])

  useEffect(() => {
    var connected = 0
    var alerted = 0

    monitorData && monitorData?.map((monitor) => {
      const data = JSON.parse(monitor)
      data.connected ? connected++ : alerted++
      
      if (data.connected && Object.values(data.field_values || {}).some(fieldValue => fieldValue.alert)) {
        alerted++
      }

      // if (data.connected && Object.entries(data.alerts).length != 0) {
      //   alerted++
      // }

      setStatus({
        connected: connected,
        network: monitorData.length,
        alert: alerted,
      })
    })
  }, [monitorData])

  return (
    <div className="row sigmaDarkBg sigmaStatusBar justify-content-md-center text-center">
      <div className="col-md-auto">
        <h5 className="text-nowrap">
          <span className="badge rounded-ltlb-pill bg-light text-dark">Devices</span>
          <span className="badge rounded-rtrb-pill sigmaBlue">
            <div className="spinner-border text-light" role="status" name="spinner" style={{ maxWidth: "10px", maxHeight: "10px" }}>
              <span className="visually-hidden">Loading...</span>
            </div>
            {status.network}
          </span>
        </h5>
      </div>
      <div className="col-md-auto">
        <h5 className="text-nowrap">
          <span className="badge rounded-ltlb-pill bg-light text-dark ">Connected</span>
          <span className="badge rounded-rtrb-pill sigmaBlue">
            <div className="spinner-border text-light" role="status" name="spinner" style={{ maxWidth: "10px", maxHeight: "10px" }}>
              <span className="visually-hidden">Loading...</span>
            </div>
            {status.connected}
          </span>
        </h5>
      </div>
      <div className="col-md-auto">
        <h5 className="text-nowrap">
          <span className="badge rounded-ltlb-pill bg-light text-dark ">Alerts</span>
          <span className="badge rounded-rtrb-pill sigmaRed">
            <div className="spinner-border text-light" role="status" name="spinner" style={{ maxWidth: "10px", maxHeight: "10px" }}>
              <span className="visually-hidden">Loading...</span>
            </div>
            {status.alert}
          </span>
        </h5>
      </div>
    </div>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
    devices: state.main.devices,
  }
}

export default connect(mapStateToProps)(Status)
