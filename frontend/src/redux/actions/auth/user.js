import * as t from "../../types/auth/user"
import axios from "axios"

export const setUserName = (userName) => (dispatch) => {
  dispatch({
    type: t.GET_USERNAME,
    payload: userName,
  })
}

export const authenticateAction = (user) => {
  return {
    type: t.AUTHENTICATE,
    payload: user,
  }
}

export const deAuthenticateAction = () => {
  return {
    type: t.DEAUTHENTICATE,
  }
}

export const restoreState = (authState) => {
  return {
    type: t.RESTORE_AUTH_STATE,
    payload: authState,
  }
}

export const login = (loginDetails) => {
  return async (dispatch) => {
    try {
      dispatch(deAuthenticateAction())
      const result = await axios.post(
        "http://localhost:3000/api/auth/postLogin",
        loginDetails
      )
      if (result.data.name === loginDetails.email)
        dispatch(authenticateAction(result))
    } catch (e) {
      dispatch(deAuthenticateAction())
    }
  }
}

export const signUp = (signUpDetails) => {
  return async (dispatch) => {
    try {
      dispatch(deAuthenticateAction())
      // Signup code. And storing data in result variable
      dispatch(authenticateAction(result))
    } catch (e) {
      dispatch(deAuthenticateAction())
    }
  }
}

export const logout = () => {
  return async (dispatch) => {
    dispatch(deAuthenticateAction())
  }
}

export const restore = (savedState) => {
  return (dispatch) => {
    dispatch(restoreState(savedState))
  }
}
