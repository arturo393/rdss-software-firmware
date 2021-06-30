import * as t from "../types"

const main = (
  state = {
    name: "none",
  },
  action
) => {
  switch (action.type) {
    case t.SET_NAME:
      console.log("SET NAME REDUCER")
      return {
        ...state,
        name: action.payload,
      }
    case t.SET_NAME_EVENT:
      console.log("SET NAME EVENT REDUCER")
      return {
        ...state,
        name: action.payload,
      }
    default:
      return { ...state }
  }
}

export default main
