import { useState, useEffect } from "react"
import { Form, Button, Container, Card, Row, Col } from "react-bootstrap"
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
    <>
      <div className="container">
        <div className="card h-100 ">
          <div className="card-body ">
            <Row>
              <Col md={4}>
                <img src="/images/login_sigma.png" alt="" width="140%" />
              </Col>
              <Col md={2}></Col>
              <Col md={5}>
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
                  {/* <Form.Group controlId="formBasicCheckbox">
                  <Form.Check type="checkbox" label="Check me out" />
                </Form.Group> */}
                  <div className="row mt-5">
                    <Button variant="primary" type="submit" classname="border-0">
                      Login
                    </Button>
                  </div>
                </Form>
              </Col>
            </Row>
          </div>
        </div>
      </div>
    </>
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
