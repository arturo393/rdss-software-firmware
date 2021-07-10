import * as t from "../types"

const user = (
  state = {
    email: "none",
  },
  action
) => {
  switch (action.type) {
    case t.SET_NAME_USER:
      return {
        ...state,
        email: action.payload,
      }
    case t.SET_USER:
      return {
        ...state,
        user: action.payload,
      }
    default:
      return { ...state }
  }
}

export default user
