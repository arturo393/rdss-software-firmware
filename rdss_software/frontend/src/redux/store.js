import { createStore, applyMiddleware, compose } from "redux"
import thunk from "redux-thunk"
import { createWrapper } from "next-redux-wrapper"
import rootReducer from "./reducers/rootReducer"
import socketMiddleware from "./middleware/socketMiddleware"
import { logger, crashReporter } from "./middleware/utilsMiddleware"

const middleware = [thunk, logger, crashReporter, socketMiddleware()]

const composeEnhancers =
  typeof window === "object" && window.__REDUX_DEVTOOLS_EXTENSION_COMPOSE__
    ? window.__REDUX_DEVTOOLS_EXTENSION_COMPOSE__({})
    : compose

const enhancer = composeEnhancers(applyMiddleware(...middleware))

const makeStore = () => createStore(rootReducer, enhancer)

export const wrapper = createWrapper(makeStore)
