import React, {useEffect, useState} from 'react';
import { Container, Card, Form, Button, Col } from 'react-bootstrap';
import axios from "axios"

const Rtdata = (props) => {

    return (
        <Container>
            <Card>
                <Card.Header>Vlad Control</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <Form.Row className="d-flex">
                            <Form.Group as={Col} md="4">
                                <Form.Label>Vlad</Form.Label>
                                <Form.Control as="select">
                                    <option value={0}>Select an option...</option>
                                </Form.Control>
                                <br></br>
                                <Button variant="primary" type="button">
                                    Search
                                </Button>
                            </Form.Group>
                        </Form.Row>
                    </blockquote>
                </Card.Body>
            </Card>
            <br></br>
            <Card>
                <Card.Header>Data</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        
                    </blockquote>
                </Card.Body>
            </Card>
        </Container>
    )
}

export async function getServerSideProps() {
    const devices = await axios
      .get("http://localhost:3000/api/devices/devices")
      .then((res) => {
        return res.data
      })
  
    return {
      props: { devices },
    }
}

export default Rtdata