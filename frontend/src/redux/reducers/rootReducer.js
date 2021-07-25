import { combineReducers } from "redux"
import main from "./main"
import user from "./user"
import rol from "./rol"
import diagram from "./diagram"

const rootReducer = combineReducers({
  main: main,
  user: user,
  rol: rol,
  diagram: diagram,
})

export default rootReducer
