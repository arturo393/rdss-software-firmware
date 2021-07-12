import React, { useState } from 'react';
import { Form, Button, Container, Card, Row, Col } from 'react-bootstrap';
import axios from "axios"
import Router from 'next/router'

import imgLogin from '../../images/login_sigma.png'

function Login(props) {

    const [ state, setState ] = useState({
      email: '',
      password: ''
    });
  
    const onSubmit = (e) => {
        e.preventDefault();
        // get our form data out of state
        const login = { email: state.email, password: state.password };

        axios.post('http://localhost:3000/api/auth/postLogin', login)
          .then((result) => {
            if(result.data.length == 0){
                alert('Invalid Email and Password Combination')
            }
            else{
                Router.push('/')
            }
          }, (error) => {
            console.log(error);
          }
        );
    }
  
    const handleChange = (e) => {
      setState({ ...state, [e.target.id]: e.target.value })
    }
  
    return (
        <Container>
            <Card>
                <Card.Header>Login</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <Row>
                            <Col md={4}>
                                <img  src={imgLogin.src} alt="" width="100%"/>
                            </Col>
                            <Col md={8}>
                                <Form onSubmit={onSubmit}>
                                    <Form.Group controlId="email">
                                        <Form.Label>Email address</Form.Label>
                                        <Form.Control type="email" placeholder="Enter email" onChange={handleChange}/>
                                        <Form.Text className="text-muted">
                                        We'll never share your email with anyone else.
                                        </Form.Text>
                                    </Form.Group>

                                    <Form.Group controlId="password">
                                        <Form.Label>Password</Form.Label>
                                        <Form.Control type="password" placeholder="Password" onChange={handleChange}/>
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
                    </blockquote>
                </Card.Body>
            </Card>
        </Container>
    )
}

export default Login