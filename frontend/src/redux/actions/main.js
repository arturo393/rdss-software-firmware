import * as t from "../types"

export const setName = (name) => (dispatch) => {
  dispatch({
    type: t.SET_NAME,
    payload: name,
  })
}

export const setNameEvent = () => (dispatch) => {
  dispatch({
    event: "set_name_event",
    handle: (data) =>
      dispatch({
        type: t.SET_NAME,
        payload: data.name,
      }),
  })
}

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

export const setCompany = (company) => (dispatch) => {
  dispatch({
    type: t.SET_COMPANY,
    payload: company,
  })
}
