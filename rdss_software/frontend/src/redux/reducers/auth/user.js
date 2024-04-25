import * as t from "../../types/auth/user"
import { getCookie, setCookie, removeCookie } from "../../../lib/cookie"

let initialState
if (typeof localStorage !== "undefined") {
  const authCookie = getCookie("auth")
  if (authCookie && authCookie.o != undefined) {
    initialState = JSON.parse(decodeURIComponent(authCookie))
  } else {
    initialState = {
      isLoggedIn: false,
      user: {},
    }
  }
} else {
  initialState = {
    isLoggedIn: false,
    user: {},
  }
}

const user = (state = initialState, action) => {
  switch (action.type) {
    case t.DEAUTHENTICATE:
      removeCookie("auth")
      return {
        isLoggedIn: false,
      }

    case t.AUTHENTICATE:
      const authObj = {
        isLoggedIn: true,
        user: action.payload,
      }
      setCookie("auth", authObj)
      return authObj
    case t.RESTORE_AUTH_STATE:
      return {
        isLoggedIn: true,
        user: action.payload.user,
      }
    default:
      return state
  }
}

export default user
