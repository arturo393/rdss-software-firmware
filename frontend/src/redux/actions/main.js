import * as t from "../types"
import axios from "axios"

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

export const setActiveDeviceId = (id) => (dispatch) => {
  dispatch({
    type: t.SET_ACTIVE_DEVICE_ID,
    payload: id,
  })
}

export const saveConfig = (config) => {
  console.log("SAVE CONFIG")
  return async (dispatch) => {
    try {
      const result = await axios.post(
        "http://" +
          process.env.NEXT_PUBLIC_APIHOST +
          ":" +
          process.env.NEXT_PUBLIC_APIPORT +
          "/api/manage/saveConfig",
        config
      )
      if (result) dispatch(setConfig(config))
    } catch (e) {
      console.log(e)
    }
  }
}

export const setConfig = (config) => (dispatch) => {
  console.log("SET_CONFIG")
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
