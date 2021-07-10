import * as t from "../types"

const main = (
  state = {
    name: "none",
    company: "empty"
  },
  action
) => {
  switch (action.type) {
    case t.SET_NAME:
      return {
        ...state,
        name: action.payload,
      }
    case t.SET_COMPANY:
      return {
        ...state,
        company: action.payload
      }
    default:
      return { ...state }
  }
}

export default main
