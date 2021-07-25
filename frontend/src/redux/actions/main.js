import * as t from "../types"

export const setMonitorDataEvent = () => (dispatch) => {
  dispatch({
    event: "set_rtdata_event",
    handle: (data) => {
      dispatch({
        type: t.SET_MONITOR_DATA_EVENT,
        payload: data,
      })
    },
  })
}

export const setActiveComponent = (name) => (dispatch) => {
  dispatch({
    type: t.SET_ACTIVE_COMPONENT,
    payload: name,
  })
}

export const setConfig = (config) => (dispatch) => {
  dispatch({
    type: t.SET_CONFIG,
    payload: config,
  })
}

export const setDevices = (devices) => (dispatch) => {
  dispatch({
    type: t.SET_DEVICES,
    payload: devices,
  })
}
