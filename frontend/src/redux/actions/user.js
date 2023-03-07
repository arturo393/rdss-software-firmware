import * as t from "../types"

export const setCompany = (company) => (dispatch) => {
  dispatch({
    type: t.SET_COMPANY,
    payload: company,
  })
}

export const setEmail = (email) => (dispatch) => {
  dispatch({
    type: t.SET_NAME_USER,
    payload: email,
  })
}

export const setCompanyEvent = () => (dispatch) => {
  dispatch({
    event: "set_company_event",
    handle: (data) =>
      dispatch({
        type: t.SET_COMPANY,
        payload: data.company,
      }),
  })
}
