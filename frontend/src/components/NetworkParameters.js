import React from "react"
import { Container, Card } from "react-bootstrap"

import NetworkUpload from "./manage/network/NetworkUpload"
//MEMO
// import NetworkForm from "./manage/network/NetworkForm"
// import NetworkForm from "./manage/network/NetworkFormType"
import NetworkForm from "./manage/DeviceFormulario"

const NetworkParameters = () => {
  return (
    <div className="containers">
      <div className="text-center mt-2 mb-2">
        <h5>Alerts Linits</h5>
      </div>
      <div className="card h-100">
        <div className="card-body">
          <blockquote className="blockquote mb-0">
            {/* <NetworkUpload /> */}
          </blockquote>
          <br></br>
          <blockquote className="blockquote mb-0">
            <NetworkForm />
          </blockquote>
        </div>
      </div>
    </div>
  )
}

export default NetworkParameters
