import React, { useState, useEffect } from 'react'
import { Button, Card, Form, Container } from 'react-bootstrap';
import NextLink from 'next/link';
import {alert} from 'react-bootstrap-confirmation';
import axios from "axios"


const ManageRol = () => {
    const [ state, setState ] = useState({
        id: '',
        namerol: ''
    });

    useEffect(() => {
        const userId = {userId: window.location.href.split('/').reverse()[0]}
        axios.post('http://localhost:3000/api/manage/userid', userId)
            .then((result) => {
                const rolname = result.data[0].rolname
                const userid = result.data[0]._id
                setState({ namerol: rolname, id: userid })
            }, (error) => {
                console.log(error);
            }
        );
    }, [])

    const handleRol = (e) => {
        setState({ ...state, namerol: e.target.value })
    }

    const onSubmit = (e) => {
        e.preventDefault();
        // get our form data out of state
        const user = { id: state.id, rolname: state.namerol };
    
        axios.post('http://localhost:3000/api/manage/editUserRol', user)
          .then((result) => {
            alert("User updated successfully")
          }, (error) => {
            console.log(error);
          }
        );
    }
    
    return (
        <Container>
            <Card>
                <Card.Header>Manage User Rol</Card.Header>
                <Form onSubmit={onSubmit}>
                    <Card.Body>
                        <blockquote className="blockquote mb-0">
                            <Form.Label>Select Rol</Form.Label>
                            <Form.Control as="select" value={state.namerol} onChange={handleRol}>
                                <option value="SuperAdmin">SuperAdmin</option>
                                <option value="Admin">Admin</option>
                                <option value="User">User</option>
                            </Form.Control>
                        </blockquote>
                    </Card.Body>
                    <Card.Footer>
                        <Button variant="primary" type="submit">
                            Update
                        </Button>
                        {' '}
                        <NextLink href={`/manage/users/edit/${state.id}`}>
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
  
export default ManageRol;