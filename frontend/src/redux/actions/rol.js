import * as t from "../types"

export const setName = (name) => (dispatch) => {
  dispatch({
    type: t.SET_NAME_ROL,
    payload: name,
  })
}