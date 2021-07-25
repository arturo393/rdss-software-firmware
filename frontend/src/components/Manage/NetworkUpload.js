import React, { useState, useEffect } from "react"
import { Form, Button, Row } from "react-bootstrap"
import axios from "axios"
import NextLink from "next/link"
import { setConfig } from "../../redux/actions/main"

const NetworkUpload = () => {
  const [state, setState] = useState({
    file: [],
    base64: "",
  })

  const onChange = (e) => {
    const file = e.target.files[0]

    let reader = new FileReader()
    reader.readAsDataURL(file)
    reader.onloadend = () => {
      setState({
        file: file,
        base64: reader.result,
      })
    }
  }

  const onSubmit = (e) => {
    e.preventDefault()
    const photo = { photo: state.base64 }

    axios.post("http://localhost:3000/api/manage/editConfigPhoto", photo).then(
      (result) => {
        alert("Photo updated successfully")
      },
      (error) => {
        console.log(error)
      }
    )
  }

  return (
    <Form onSubmit={onSubmit}>
      <Form.Group as={Row} controlId="formPhoto">
        <Form.Label column sm="2">
          Photo
        </Form.Label>
        <Form.File id="image" name="image" onChange={onChange} />
      </Form.Group>
      <br></br>
      <NextLink href="/">
        <Button variant="danger" type="button">
          Cancel
        </Button>
      </NextLink>{" "}
      <Button variant="primary" type="submit">
        Upload
      </Button>
    </Form>
  )
}

export default NetworkUpload
