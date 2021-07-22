import React, {useEffect, useState} from 'react';
import { Container, Card, Form } from 'react-bootstrap';
import { setMonitorDataEvent } from "../../redux/actions/main"
import { connect } from "react-redux"

const Status = (props) => {
    console.log("props status: ", props)
    const { setMonitorDataEvent } = props

    const [state] = useState({
        status: [
            {
                connected: 0,
                network: 0,
                alert: 0
            }
        ]
    })
    

    useEffect(() => {
        setMonitorDataEvent()

        props.monitorData?.map((monitor) => {
            const data = JSON.parse(monitor)
            if (state.status.some(s=> s.id == parseInt(data.id)) && data.alerts != undefined){
                
                setSquare({ squares })
            }
            else{
                
            }
        })
    }, [props.monitorData])

    return(
        <Container>
            <Card>
                <Card.Header>Status</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <Form.Label>In Network: 0</Form.Label>
                        {'     /     '}
                        <Form.Label>Connect: 0</Form.Label>
                        {'     /     '}
                        <Form.Label>Alerted: 0</Form.Label>
                    </blockquote>
                </Card.Body>
            </Card>
        </Container>
    )
}

const mapStateToProps = (state) => {
    return {
        monitorData: state.main.monitorData
    }
}

const mapDispatchToProps = {
    setMonitorDataEvent,
}

export default connect(mapStateToProps, mapDispatchToProps)(Status)
