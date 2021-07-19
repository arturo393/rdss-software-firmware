import React, { useEffect, useState } from 'react';
import { Form, Col, Button } from 'react-bootstrap';
import { connect } from "react-redux"
import { setConfig } from '../../redux/actions/config'
import axios from "axios"


const NetworkForm = (props) => {

    const [ state, setState ] = useState({
        config: []
    });

    useEffect(() => {
        axios.post('http://localhost:3000/api/manage/config')
            .then((result) => {
                setState({config: result.data[0]})
            }, (error) => {
                console.log(error);
            }
        );
        
    }, [])

    const handleChange = (e) => {
        setState(prevState => ({
            config: { 
                ...prevState.config, [e.target.id]: e.target.value
            }
        }))
    }

    const onSubmit = (e) => {
        e.preventDefault();
        const config =  { 
            minVoltage: state.config.minVoltage,
            maxVoltage: state.config.maxVoltage,
            minCurrent: state.config.minCurrent,
            maxCurrent: state.config.maxCurrent,
            minUplink: state.config.minUplink,
            maxUplink: state.config.maxUplink,
            minDownlink: state.config.minDownlink,
            maxDownlink: state.config.maxDownlink,
            minDownlinkOut: state.config.minDownlinkOut, 
            maxDownlinkOut: state.config.maxDownlinkOut
        };
    
        axios.post('http://localhost:3000/api/manage/editConfig', config)
          .then((result) => {
            alert("Networks Parameters updated successfully")
          }, (error) => {
            console.log(error);
          }
        );
    }

    return (
        <Form onSubmit={onSubmit}>
            <Form.Row className="d-flex">
                <Form.Group as={Col} md="4" controlId="minVoltage">
                    <Form.Label>Min Voltage [Volts]</Form.Label>
                    <Form.Control type="text" value={state.config.minVoltage} onChange={handleChange}/>
                </Form.Group>
                {' '}
                <Form.Group as={Col} className="col-md-4 offset-2" controlId="maxVoltage">
                    <Form.Label>Max Voltage [Volts]</Form.Label>
                    <Form.Control type="text" value={state.config.maxVoltage} onChange={handleChange}/>
                </Form.Group>
            </Form.Row>

            <Form.Row className="d-flex">
                <Form.Group as={Col}  md="4" controlId="minCurrent">
                    <Form.Label>Min Current [A]</Form.Label>
                    <Form.Control type="text" value={state.config.minCurrent} onChange={handleChange}/>
                </Form.Group>

                <Form.Group as={Col} className="col-md-4 offset-2" controlId="maxCurrent">
                    <Form.Label>Max Current [A]</Form.Label>
                    <Form.Control type="text" value={state.config.maxCurrent} onChange={handleChange}/>
                </Form.Group>
            </Form.Row>

            <Form.Row className="d-flex">
                <Form.Group as={Col}  md="4" controlId="minUplink">
                    <Form.Label>Min Uplink [dB]</Form.Label>
                    <Form.Control type="text" value={state.config.minUplink} onChange={handleChange}/>
                </Form.Group>

                <Form.Group as={Col} className="col-md-4 offset-2" controlId="maxUplink">
                    <Form.Label>Max Uplink AGC [dB]</Form.Label>
                    <Form.Control type="text" value={state.config.maxUplink} onChange={handleChange}/>
                </Form.Group>
            </Form.Row>

            <Form.Row className="d-flex">
                <Form.Group as={Col}  md="4" controlId="minDownlink">
                    <Form.Label>Min Downlink [dB]</Form.Label>
                    <Form.Control type="text" value={state.config.minDownlink} onChange={handleChange}/>
                </Form.Group>

                <Form.Group as={Col} className="col-md-4 offset-2" controlId="maxDownlink">
                    <Form.Label>
                        Max Downlink AGC [dB]</Form.Label>
                    <Form.Control type="text" value={state.config.maxDownlink} onChange={handleChange}/>
                </Form.Group>
            </Form.Row>

            <Form.Row className="d-flex">
                <Form.Group as={Col}  md="4" controlId="minDownlinkOut">
                    <Form.Label>Min Downlink Output Power [dBm]</Form.Label>
                    <Form.Control type="text" value={state.config.minDownlinkOut} onChange={handleChange}/>
                </Form.Group>

                <Form.Group as={Col} className="col-md-4 offset-2" controlId="maxDownlinkOut">
                    <Form.Label>Max Downlink Output Power [dBm]</Form.Label>
                    <Form.Control type="text" value={state.config.maxDownlinkOut} onChange={handleChange}/>
                </Form.Group>
            </Form.Row>
            <br></br>
            <Button variant="danger" type="button">
                Cancel
            </Button>
            {' '}
            <Button variant="primary" type="submit">
                Update Bounds Parameters
            </Button>
        </Form>
    )
}

const mapStateToProps = (state) => {
    return { 
      config: state.config,
    }
}

const mapDispatchToProps = {
    setConfig
}

export async function getServerSideProps() {
    const config = await axios
      .get("http://localhost:3000/api/manage/config")
      .then((res) => {
        return res.data
      })
  
    return {
      props: {
        config
      },
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(NetworkForm)