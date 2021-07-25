import { useEffect, useState } from "react"
import { setMonitorDataEvent } from "../../redux/actions/main"
import { connect } from "react-redux"

const Status = (props) => {
  const { monitorData } = props

  const [status, setStatus] = useState({
    connected: 0,
    network: 0,
    alert: 0,
  })

  useEffect(() => {
    setMonitorDataEvent()
    var connected = 0
    var alerted = 0

    monitorData?.map((monitor) => {
      const data = JSON.parse(monitor)
      data.connected ? connected++ : alerted++
      if (data.connected && data.alerts != undefined && data.alerts.length > 0) {
        alerted++
      }
      setStatus({
        connected: connected,
        network: monitorData.length,
        alert: alerted,
      })
    })
  }, [monitorData])

  return (
    <>
      <div className="container-fluid sigmaDarkBg">
        <div className="row">
          <div className="col-md-12">
            <div className="row">
              <div className="col-md-4">
                <h3 className="mt-3 mb-3">
                  <span className="badge rounded-ltlb-pill bg-light text-dark">
                    In Network
                  </span>
                  <span className="badge rounded-rtrb-pill sigmaBlue">
                    {status.network ? status.network : 0}
                  </span>
                </h3>
              </div>
              <div className="col-md-4">
                <h3 className="mt-3 mb-3">
                  <span className="badge rounded-ltlb-pill bg-light text-dark ">
                    Connected
                  </span>
                  <span className="badge rounded-rtrb-pill sigmaBlue">
                    {status.connected ? status.connected : 0}
                  </span>
                </h3>
              </div>
              <div className="col-md-4">
                <h3 className="mt-3 mb-3">
                  <span className="badge rounded-ltlb-pill bg-light text-dark ">
                    With Alerts
                  </span>
                  <span className="badge rounded-rtrb-pill sigmaRed">
                    {status.alert ? status.alert : 0}
                  </span>
                </h3>
              </div>
            </div>
          </div>
        </div>
      </div>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Status)
