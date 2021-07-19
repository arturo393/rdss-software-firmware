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
