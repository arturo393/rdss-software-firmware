import { connect } from "react-redux"
import { useState, useEffect } from "react"

import Alerts from "./Alerts"
import Rtdata from "./Rtdata"
import DiagramEdit from "./DiagramEdit"
import NetworkParameters from "./NetworkParameters"
import Schema from "./Schema"

import Login from "./auth/Login"
import UsersAdmin from "./manage/users/Users"

const DynamicComponent = (props) => {
  const { activeComponent } = props

  const components = {
    alerts: Alerts,
    rtdata: Rtdata,
    diagramedit: DiagramEdit,
    networkParameters: NetworkParameters,
    login: Login,
    usersadmin: UsersAdmin,
    map: Schema,
  }
  const SelectComponent = components[activeComponent ? activeComponent : "alerts"]
  return <SelectComponent />
}

const mapStateToProps = (state) => {
  return { activeComponent: state.main.activeComponent }
}

export default connect(mapStateToProps)(DynamicComponent)
