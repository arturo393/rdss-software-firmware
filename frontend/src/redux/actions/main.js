import * as t from "../types"

export const setRTDataEvent = () => (dispatch) => {
  dispatch({
    event: "set_rtdata_event",
    handle: (data) =>
      dispatch({
        type: t.SET_RTDATA,
        payload: data.rtdata,
      }),
  })
}