import React from "react"
import { Button, Container, Card } from "react-bootstrap"
import Router from "next/router"
import { setName } from "../../../redux/actions/rol"
import { useEffect } from "react"
import { connect } from "react-redux"
import axios from "axios"
import Rol from "./rol"

function Roles(props) {
  const handleHome = () => {
    Router.push("/")
  }

  const { setName } = props

  useEffect(() => {
    setName(props.roles)
  }, [])

  return (
    <Container>
      <Card>
        <Card.Header>Roles List</Card.Header>
        <Card.Body>
          <blockquote className="blockquote mb-0">
            {props.roles.map((rol) => (
              <Rol rol={rol} key={rol._id} />
            ))}
          </blockquote>
        </Card.Body>
        <Card.Footer>
          <blockquote className="blockquote mb-0">
            <Button variant="success" type="button" onClick={handleHome}>
              Go Home
            </Button>
          </blockquote>
        </Card.Footer>
      </Card>
    </Container>
  )
}

const mapStateToProps = (state) => {
  return {
    name: state.rol.name,
  }
}

const mapDispatchToProps = {
  setName,
}

export async function getServerSideProps() {
  const roles = await axios
    .get(
      "http://" +
        process.env.NEXT_PUBLIC_APIHOST +
        ":" +
        process.env.NEXT_PUBLIC_APIPORT +
        "/api/manage/roles"
    )
    .then((res) => {
      return res.data
    })

  return {
    props: {
      roles,
    },
  }
}

export default connect(mapStateToProps, mapDispatchToProps)(Roles)
