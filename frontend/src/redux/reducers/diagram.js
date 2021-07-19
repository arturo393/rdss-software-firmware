import * as t from "../types"

const diagram = (
  state = {
    path: "none",
  },
  action
) => {
  switch (action.type) {
    case t.SET_DIAGRAM_PATH:
      return {
        ...state,
        path: action.payload,
      }
    default:
      return { ...state }
  }
}

export default diagram
