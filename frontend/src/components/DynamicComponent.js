import { connect } from "react-redux"
import { useState, useEffect } from "react"

import Alerts from "./Alerts"
import Rtdata from "./Rtdata"
import DiagramEdit from "./DiagramEdit"
import NetworkParameters from "./NetworkParameters"

import Login from "./auth/Login"
import Register from "./auth/Register"

const DynamicComponent = (props) => {
  const { activeComponent } = props

  const components = {
    alerts: Alerts,
    rtdata: Rtdata,
    diagramedit: DiagramEdit,
    networkParameters: NetworkParameters,
    login: Login,
    register: Register,
  }
  const SelectComponent = components[activeComponent ? activeComponent : "alerts"]
  return <SelectComponent />
}

const mapStateToProps = (state) => {
  return { activeComponent: state.main.activeComponent }
}

export default connect(mapStateToProps)(DynamicComponent)
