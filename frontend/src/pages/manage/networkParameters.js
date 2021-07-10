import React from 'react';
import { Container, Card } from 'react-bootstrap';

import NetworkUpload from '../../components/manage/NetworkUpload';
import NetworkForm from '../../components/manage/NetworkForm'

const NetworkParameters = () => {
    return (
        <Container>
            <Card>
                <Card.Header>Edit Leaky Feeder Network Parameters</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <NetworkUpload />
                    </blockquote>
                </Card.Body>
            </Card>
            <br></br>
            <Card>
                <Card.Header>Alarm Threshold</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <NetworkForm />
                    </blockquote>
                </Card.Body>
            </Card>
        </Container>
    )
}

export default NetworkParameters