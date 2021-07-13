import * as t from "../types"

const main = (
  state = {
    name: "none",
    company: "empty",
    rtdata: {},
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
        company: action.payload,
      }
    case t.SET_RTDATA:
      return {
        ...state,
        rtdata: action.payload,
      }
    default:
      return { ...state }
  }
}

export default main
