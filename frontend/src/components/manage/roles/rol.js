import React from "react"
import { Button, Card } from "react-bootstrap"
import NextLink from "next/link"

const Rol = ({ rol }) => {
  return (
    <div>
      <Card>
        <Card.Header>{rol?.name}</Card.Header>
        <Card.Body>
          <blockquote className="blockquote mb-0">
            <NextLink href={`/manage/roles/edit/${rol?._id}`}>
              <Button variant="primary" type="button">
                Add Users
              </Button>
            </NextLink>
          </blockquote>
        </Card.Body>
      </Card>
      <br></br>
    </div>
  )
}

export default Rol
