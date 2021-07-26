import React, { Component } from "react"
import LoginForm from "./LoginForm"
import { connect } from "react-redux"
import { login } from "../../redux/actions/user"

class SignIn extends Component {
  constructor(props) {
    super(props)
  }

  componentDidUpdate(prevProps, prevState, snapshot) {
    if (this.props.isLoggedIn === true) {
      Router.pushRoute("/")
    }
  }

  handleLoginSubmit = (value) => {
    const { dispatch } = this.props
    dispatch(login(value))
  }

  render() {
    return (
      <div>
        <LoginForm onChange={this.handleLoginSubmit} />
      </div>
    )
  }
}

function mapStateToProps(state) {
  return {
    user: state.user,
    isLoggedIn: state.user.isLoggedIn,
  }
}

export default connect(mapStateToProps)(SignIn)
