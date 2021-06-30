import * as t from "../types"

export const setName = (name) => (dispatch) => {
  console.log("ACTION: SET NAME")

  dispatch({
    type: t.SET_NAME,
    payload: name,
  })
}

export const setNameEvent = (name) => (dispatch) => {
  console.log("ACTION: SET NAME EVENT")
  dispatch({
    event: "set_name_event",
    handle: (data) =>
      dispatch({
        type: t.SET_NAME,
        payload: data.name,
      }),
  })
}
