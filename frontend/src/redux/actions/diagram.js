import * as t from "../types"

export const setPath = (path) => (dispatch) => {
  dispatch({
    type: t.SET_DIAGRAM_PATH,
    payload: company,
  })
}

export const setXY = (x, y) => (dispatch) => {
  dispatch({
    type: t.SET_DIAGRAM_XY,
    payload: { x: x, y: y },
  })
}
