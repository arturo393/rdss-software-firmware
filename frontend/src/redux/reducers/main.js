import * as t from "../types"

const main = (
  state = {
    name: "none",
    company: "empty",
    monitorData: [],
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

    case t.SET_MONITOR_DATA_EVENT:
      return {
        ...state,
        monitorData: action.payload.data,
      }
    default:
      return { ...state }
  }
}

export default main
