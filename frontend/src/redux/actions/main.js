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
  console.log("NAME:" + name)
  console.log(dispatch)
  dispatch({
    type: t.SET_ACTIVE_COMPONENT,
    payload: name,
  })
}
