import React, { useState, useEffect } from 'react';
import { Form, Button, Row } from 'react-bootstrap';
import axios from "axios"
import NextLink from 'next/link';

const NetworkUpload = () => {

    const [ state, setState ] = useState({
        file: [],
        base64: ''
    });

    const onSubmit = (e) => {
        e.preventDefault();
        const file = e.target[0].files[0]

        let reader = new FileReader();
        reader.readAsDataURL(file);
        reader.onloadend = () => {
            setState({
                file: file,
                base64: reader.result
            });
        }

        const photo = { photo: state.base64 };
    
        axios.post('http://localhost:3000/api/manage/editConfigPhoto', photo)
          .then((result) => {
            alert("Photo updated successfully")
          }, (error) => {
            console.log(error);
          }
        );
    }

    return (
        <Form onSubmit={onSubmit}>
            <Form.Group as={Row} controlId="formPhoto">
                <Form.Label column sm="2">
                    Photo
                </Form.Label>
                <Form.File id="image" name="image" />
            </Form.Group>
            <br></br>
            <NextLink href="/">
                <Button variant="danger" type="button">
                    Cancel
                </Button>
            </NextLink>
            {' '}
            <Button variant="primary" type="submit">
                Update
            </Button>
        </Form>
    )
}

export default NetworkUpload