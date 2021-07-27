import React, { useState, useEffect } from "react"
import { Button, Card, Form, Container } from "react-bootstrap"
import NextLink from "next/link"
import { alert } from "react-bootstrap-confirmation"
import axios from "axios"
import { setEmail } from "../../../../redux/actions/user"
import { connect } from "react-redux"

const ManageUser = (props) => {
  const { setEmail } = props

  const [state, setState] = useState({
    rol: [],
    users: props.users,
    userRol: [],
  })

  useEffect(() => {
    const rolId = { rolId: window.location.href.split("/").reverse()[0] }
    axios
      .post(
        "http://" +
          process.env.NEXT_PUBLIC_APIHOST +
          ":" +
          process.env.NEXT_PUBLIC_APIPORT +
          "/api/manage/rolid",
        rolId
      )
      .then(
        (result) => {
          const objRol = result.data[0]

          setState((prevState) => ({
            ...prevState,
            rol: objRol,
          }))

          setState((prevState) => ({
            ...prevState,
            userRol: objRol.users,
          }))
        },
        (error) => {
          console.log(error)
        }
      )
  }, [])

  const handleChange = (e) => {
    if (e.target.checked) {
      const newUserRol = { name: e.target.name }
      setState((prevState) => ({
        ...prevState,
        userRol: state.userRol.concat(newUserRol),
      }))
    } else {
      const usersRol = state.userRol?.filter((i) => i.name !== e.target.name)
      setState((prevState) => ({
        ...prevState,
        userRol: usersRol,
      }))
    }
  }

  const onSubmit = (e) => {
    e.preventDefault()
    const rolUser = { id: state.rol._id, users: state.userRol }
    axios
      .post(
        "http://" +
          process.env.NEXT_PUBLIC_APIHOST +
          ":" +
          process.env.NEXT_PUBLIC_APIPORT +
          "/api/manage/editRolUser",
        rolUser
      )
      .then(
        (result) => {
          alert("Rol has been edited")
        },
        (error) => {
          console.log(error)
        }
      )
  }

  return (
    <Container>
      <Card>
        <Card.Header>Add or remove users from this role</Card.Header>
        <Form onSubmit={onSubmit}>
          <Card.Body>
            <blockquote className="blockquote mb-0">
              {state.users?.map((user) => {
                if (state.userRol?.some((i) => i.name === user.name)) {
                  return (
                    <Form.Check
                      type="checkbox"
                      name={user.name}
                      label={user.name}
                      checked={true}
                      onChange={handleChange}
                    />
                  )
                } else {
                  return (
                    <Form.Check
                      type="checkbox"
                      name={user.name}
                      label={user.name}
                      onChange={handleChange}
                    />
                  )
                }
              })}
            </blockquote>
          </Card.Body>
          <Card.Footer>
            <Button variant="primary" type="submit">
              Update
            </Button>{" "}
            <NextLink href={`/manage/roles/edit/${state.rol._id}`}>
              <Button variant="danger" type="button">
                Cancel
              </Button>
            </NextLink>
          </Card.Footer>
        </Form>
      </Card>
      <br></br>
    </Container>
  )
}

const mapStateToProps = (state) => {
  return {
    email: state.user.email,
  }
}

const mapDispatchToProps = {
  setEmail,
}

export async function getServerSideProps() {
  const users = await axios
    .get(
      "http://" +
        process.env.NEXT_PUBLIC_APIHOST +
        ":" +
        process.env.NEXT_PUBLIC_APIPORT +
        "/api/manage/users"
    )
    .then((res) => {
      return res.data
    })

  return {
    props: {
      users,
    },
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(ManageUser)
