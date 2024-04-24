import * as t from "../types"

const rol = (
  state = {
    name: "none"
  },
  action
) => {
  switch (action.type) {
    case t.SET_NAME_ROL:
      return {
        ...state,
        name: action.payload,
      }
    default:
      return { ...state }
  }
}

export default rol
