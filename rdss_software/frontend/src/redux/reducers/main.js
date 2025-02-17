import * as t from "../types"

const main = (
  state = {
    name: "none",
    company: "empty",
    monitorData: [],
    activeComponent: "",
    config: {},
    devices: [],
    activeDeviceId: 0,
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
    case t.SET_ACTIVE_COMPONENT:
      return {
        ...state,
        activeComponent: action.payload,
      }
    case t.SET_CONFIG:
      return {
        ...state,
        config: action.payload,
      }
    case t.SET_DEVICES:
      return {
        ...state,
        devices: action.payload,
      }
    case t.SET_ACTIVE_DEVICE_ID:
      return {
        ...state,
        activeDeviceId: action.payload,
      }
    default:
      return { ...state }
  }
}

export default main
