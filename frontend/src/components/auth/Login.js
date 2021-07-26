import { useState, useEffect } from "react"
import { Form, Button, Container, Card, Row, Col } from "react-bootstrap"
import imgLoginForm from "../../images/Login_sigma.png"
import { connect } from "react-redux"
import { login } from "../../redux/actions/auth/user"

const Login = (props) => {
  const { user, isLoggedIn, login } = props

  useEffect(() => {
    // if (isLoggedIn) alert("Usuario Identificado")
  }, [user])

  const handleLoginSubmit = (e) => {
    e.preventDefault()
    login({
      email: e.target[0].value,
      password: e.target[1].value,
    })
  }

  return (
    <div className="containers">
      <div class="card h-100">
        <div className="card-body">
          <Row>
            <Col md={4}>
              <img src={imgLoginForm.src} alt="" width="100%" />
            </Col>
            <Col md={8}>
              <Form onSubmit={handleLoginSubmit}>
                <Form.Group controlId="email">
                  <Form.Label>Email address</Form.Label>
                  <Form.Control type="email" placeholder="Enter email" />
                  <Form.Text className="text-muted">
                    We'll never share your email with anyone else.
                  </Form.Text>
                </Form.Group>

                <Form.Group controlId="password">
                  <Form.Label>Password</Form.Label>
                  <Form.Control type="password" placeholder="Password" />
                </Form.Group>
                <Form.Group controlId="formBasicCheckbox">
                  <Form.Check type="checkbox" label="Check me out" />
                </Form.Group>
                <Button variant="primary" type="submit">
                  Login
                </Button>
              </Form>
            </Col>
          </Row>
        </div>
      </div>
    </div>
  )
}

const mapDispatchToProps = { login }

function mapStateToProps(state) {
  return {
    user: state.user.user,
    isLoggedIn: state.user.isLoggedIn,
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Login)
