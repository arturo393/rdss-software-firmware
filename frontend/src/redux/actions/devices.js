import * as t from "../types"

export const setDevices = () => (dispatch) => {
  dispatch({
    event: "set_devices",
    handle: (data) =>
      dispatch({
        type: t.SET_DEVICES,
        payload: data.devices,
      }),
  })
}