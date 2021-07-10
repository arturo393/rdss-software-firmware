import * as t from "../../types/auth/user"

export const setUserName = (userName) => (dispatch) => {
  dispatch({
    type: t.GET_USERNAME,
    payload: userName,
  })
}
