import * as t from "../types"

const diagram = (
  state = {
    path: "none",
    xy: { x: 0, y: 0 },
  },
  action
) => {
  switch (action.type) {
    case t.SET_DIAGRAM_PATH:
      return {
        ...state,
        path: action.payload,
      }
    case t.SET_DIAGRAM_XY:
      return {
        ...state,
        xy: action.payload,
      }
    default:
      return { ...state }
  }
}

export default diagram
