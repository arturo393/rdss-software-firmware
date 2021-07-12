import * as t from "../types"

export const setConfig = ({config}) => (dispatch) => {
  dispatch({
    type: t.SET_CONFIG,
    payload: {config},
  })
}