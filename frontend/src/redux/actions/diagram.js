import * as t from "../types"

export const setPath = (path) => (dispatch) => {
  dispatch({
    type: t.SET_DIAGRAM_PATH,
    payload: company,
  })
}
