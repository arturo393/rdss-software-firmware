import React, {useEffect, useState} from 'react';
import { Container, Card, Form } from 'react-bootstrap';
import { setMonitorDataEvent } from "../../redux/actions/main"
import { connect } from "react-redux"

const Status = (props) => {
    console.log("props status: ", props)
    const { setMonitorDataEvent } = props

    const [state, setState] = useState({
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
        let status = [  {
            connected: 0,
            network: 0,
            alert: 0
        }]
        setState({status})

        var conectados = 0
        var network = 0
        var alert = 0

        props.monitorData?.map((monitor) => {
            const data = JSON.parse(monitor)


            if (state.status.some(s=> s.id == parseInt(data.id)) && data.alerts != undefined){
                
                setSquare({ squares })
            }
            else{
                
            }

            if(data.connected == false) {
                conectados = conectados + 1
                
            }
            if(data.connected == false && data.alerts == undefined) {
                alert = alert + 1
            }

            status.connected = conectados
            status.alert = alert

            setState({status})

        })
    }, [props.monitorData])

    return(
        <Container>
            <Card>
                <Card.Header>Status</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <Form.Label>Connect:{state.status.connected}</Form.Label>
                        <Form.Label>In Netork:{state.status.network}</Form.Label>
                        <Form.Label>With Alert:{state.status.alert}</Form.Label>
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
