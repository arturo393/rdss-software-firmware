import * as t from "../types"

export const setName = (name) => (dispatch) => {
  dispatch({
    type: t.SET_NAME,
    payload: name,
  })
}

export const setNameEvent = (name) => (dispatch) => {
  dispatch({
    event: "set_name_event",
    handle: (data) =>
      dispatch({
        type: t.SET_NAME,
        payload: data.name,
      }),
  })
}
