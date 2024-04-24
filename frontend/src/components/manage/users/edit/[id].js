import React, { useState, useEffect } from "react"
import { Form, Button, Container, Card, Row, Col } from "react-bootstrap"
import axios from "axios"
import NextLink from "next/link"

const Edit = () => {
  const [state, setState] = useState({
    user: [],
  })

  useEffect(() => {
    const userId = { userId: window.location.href.split("/").reverse()[0] }
    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/userid", userId).then(
      (result) => {
        const user = result.data[0]
        setState({ user })
      },
      (error) => {
        console.log(error)
      }
    )
  }, [])

  const handleChange = (e) => {
    if (e.target.id == "name") {
      setState((prevState) => {
        let user = Object.assign({}, prevState.user)
        user.name = e.target.value
        return { user }
      })
    }
    if (e.target.id == "company") {
      setState((prevState) => {
        let user = Object.assign({}, prevState.user)
        user.company = e.target.value
        return { user }
      })
    }
  }

  const onSubmit = (e) => {
    e.preventDefault()
    // get our form data out of state
    const user = {
      id: state.user._id,
      email: state.user.name,
      company: state.user.company,
    }

    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/editUser", user).then(
      (result) => {
        alert("User updated successfully")
      },
      (error) => {
        console.log(error)
      }
    )
  }

  return (
    <Container>
      <Card>
        <Card.Header>Edit User</Card.Header>
        <Card.Body>
          <blockquote className="blockquote mb-0">
            <Form onSubmit={onSubmit}>
              <Form.Group as={Row} controlId="name">
                <Form.Label column sm="2">
                  Email
                </Form.Label>
                <Col sm="10">
                  <Form.Control type="email" placeholder="example@example.com" value={state.user.name} onChange={handleChange} />
                </Col>
              </Form.Group>
              <Form.Group as={Row} controlId="company">
                <Form.Label column sm="2">
                  Company
                </Form.Label>
                <Col sm="10">
                  <Form.Control type="text" placeholder="Company" value={state.user.company} onChange={handleChange} />
                </Col>
              </Form.Group>
              <Button variant="primary" type="submit">
                Update
              </Button>{" "}
              <NextLink href="/manage/users/users">
                <Button variant="danger" type="button">
                  Cancel
                </Button>
              </NextLink>
            </Form>
          </blockquote>
          <br></br>
          <Card>
            <Card.Header>User Rol</Card.Header>
            <Card.Body>
              <blockquote className="blockquote mb-0">
                <li>{state.user.rolname}</li>
              </blockquote>
            </Card.Body>
            <Card.Footer>
              <NextLink href={`/manage/users/managerol/${state.user._id}`}>
                <Button variant="primary" type="button">
                  Manage Rol
                </Button>
              </NextLink>
            </Card.Footer>
          </Card>
        </Card.Body>
      </Card>
    </Container>
  )
}

export default Edit
