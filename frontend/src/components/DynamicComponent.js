import { connect } from "react-redux"
import { useState, useEffect } from "react"

import Alerts from "./Alerts"
import Rtdata from "./Rtdata"

const DynamicComponent = (props) => {
  const { activeComponent } = props

  console.log(activeComponent)

  const components = {
    alerts: Alerts,
    rtdata: Rtdata,
  }
  const SelectComponent = components[activeComponent ? activeComponent : "alerts"]
  console.log(SelectComponent)
  return <SelectComponent />
}

const mapDispatchToProps = {}

const mapStateToProps = (state) => {
  return { activeComponent: state.main.activeComponent }
}

export default connect(mapStateToProps, mapDispatchToProps)(DynamicComponent)
