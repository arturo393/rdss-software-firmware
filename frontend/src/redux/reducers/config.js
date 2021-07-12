import * as t from "../types"

const config = (
  state = {
    config: []
  },
  action
) => {
  switch (action.type) {
    case t.SET_CONFIG:
      return {
        ...state,
        config: action.payload,
      }
    default:
      return { ...state }
  }
}

export default config
