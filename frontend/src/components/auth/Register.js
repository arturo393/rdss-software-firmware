import { useState } from "react"
import { Form, Button, Row, Col } from "react-bootstrap"
import axios from "axios"

function Register(props) {
  const [state, setState] = useState({
    email: "",
    company: "",
    password: "",
  })

  const onSubmit = (e) => {
    e.preventDefault()
    // get our form data out of state
    const user = {
      email: state.email,
      company: state.company,
      password: state.password,
    }

    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/auth/postUser", user).then(
      (result) => {
        alert("User created successfully")
      },
      (error) => {
        console.log(error)
      }
    )
  }

  const handleChange = (e) => {
    setState({ ...state, [e.target.id]: e.target.value })
  }

  return (
    <>
      <div className="containers">
        <div className="text-center mt-2 mb-2">
          <h5>Register</h5>
        </div>
        <div className="card h-100">
          <div className="card-body">
            <Form onSubmit={onSubmit}>
              <Form.Group as={Row} controlId="email">
                <Form.Label column sm="2">
                  Email
                </Form.Label>
                <Col sm="10">
                  <Form.Control type="text" placeholder="example@example.com" onChange={handleChange} />
                </Col>
              </Form.Group>

              <Form.Group as={Row} controlId="company">
                <Form.Label column sm="2">
                  Company
                </Form.Label>
                <Col sm="10">
                  <Form.Control type="text" placeholder="Company" onChange={handleChange} />
                </Col>
              </Form.Group>

              <Form.Group as={Row} controlId="password">
                <Form.Label column sm="2">
                  Password
                </Form.Label>
                <Col sm="10">
                  <Form.Control type="password" onChange={handleChange} />
                </Col>
              </Form.Group>
              <Button variant="primary" type="submit">
                Create Account
              </Button>
            </Form>
          </div>
        </div>
      </div>
    </>
  )
}

export default Register
