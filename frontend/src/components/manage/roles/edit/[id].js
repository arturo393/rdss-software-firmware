import React, { useState, useEffect } from 'react';
import { Form, Button, Container, Card, Row, Col } from 'react-bootstrap';
import axios from "axios"
import NextLink from 'next/link';
import ListUsers from '../../../../components/manage/roles/ListUsers'


const Edit = () => {

    const [ state, setState ] = useState({
        rol: []
    });

    useEffect(() => {
        const rolId = {rolId: window.location.href.split('/').reverse()[0]}
        axios.post('http://localhost:3000/api/manage/rolid', rolId)
            .then((result) => {
                const rol = result.data[0]
                setState({ rol })
            }, (error) => {
                console.log(error);
            }
        );
    }, [])
    
    return (
        <Container>
            <Card>
                <Card.Header>Edit Role - Test</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <h5>Users in this Role:</h5>
                        <ListUsers users={state.rol.users} />
                    </blockquote>
                </Card.Body>
                <Card.Footer>
                    <NextLink href={`/manage/roles/manageuser/${state.rol._id}`}><Button variant="primary" type="button">Add User Or Remove User</Button></NextLink>
                    {' '}
                    <NextLink href="/manage/roles/roles">
                        <Button variant="danger" type="button">
                            Cancel
                        </Button>
                    </NextLink>
                </Card.Footer>
            </Card>
        </Container>
    )
}


export default Edit