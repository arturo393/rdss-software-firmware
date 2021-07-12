import { combineReducers } from "redux"
import main from "./main"
import user from "./user"
import rol from "./rol"
import config from "./config"

const rootReducer = combineReducers({
  main: main,
  user: user,
  rol: rol,
  config: config
})

export default rootReducer
