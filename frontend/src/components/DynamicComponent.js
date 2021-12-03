import { connect } from "react-redux"
import { useState, useEffect } from "react"

import Alerts from "./Alerts"
import Rtdata from "./Rtdata"
import Diagram from "./Diagram"
import NetworkParameters from "./NetworkParameters"
import Schema from "./Schema"
import Contact from "./common/Contact"

import Login from "./auth/Login"
import UsersAdmin from "./manage/users/Users"
import DevicesEdit from "./manage/DevicesEdit"

const DynamicComponent = (props) => {
  const { activeComponent } = props

  const components = {
    alerts: Alerts,
    rtdata: Rtdata,
    diagramedit: Diagram,
    networkParameters: NetworkParameters,
    login: Login,
    usersadmin: UsersAdmin,
    map: Schema,
    contact: Contact,
    devicesEdit: DevicesEdit,
  }
  const SelectComponent = components[activeComponent ? activeComponent : "map"]
  return <SelectComponent />
}

const mapStateToProps = (state) => {
  return { activeComponent: state.main.activeComponent }
}

export default connect(mapStateToProps)(DynamicComponent)
