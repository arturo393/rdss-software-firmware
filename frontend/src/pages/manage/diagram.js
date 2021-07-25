import React, {useEffect, useState} from 'react';
import { Container, Card, Form, Button, Col } from 'react-bootstrap';
import axios from "axios"
import dynamic from 'next/dynamic'
const Stage = dynamic(() => import('react-konva').then((module) => module.Stage), { ssr: false });
const Layer = dynamic(() => import('react-konva').then((module) => module.Layer), { ssr: false });
const Image = dynamic(() => import('react-konva').then((module) => module.Image), { ssr: false });
const Rect = dynamic(() => import('react-konva').then((module) => module.Rect), { ssr: false });
const Text = dynamic(() => import('react-konva').then((module) => module.Text), { ssr: false });
const Group = dynamic(() => import('react-konva').then((module) => module.Group), { ssr: false });
import useImage from 'use-image'
import {alert} from 'react-bootstrap-confirmation';


const Diagram = (props) => {

  const [state, setState] = useState({
    scale: 1,
    x: 0,
    y: 0,
    devices: props.devices
  });
  const [stateSquare, setSquare] = useState({
    squares: []
  });
  const [stateVlad, setVlad] = useState({
    id: 0
  });
  const [image] = useImage(props.config[0].image);
  
  useEffect(() => {
    state.devices?.map((device) => {
      if (device.status.provisioned){
        const fill = device.status.connected ? "green" : "red"
        setSquare( prevSquare => ({
          squares: [...prevSquare.squares, { x: device.status.x, y: device.status.y, fill: fill, name: "Vlad - " + device.id, id: device.id }]
        }));
      }
    })
  }, [])

  const handleWheel = (e) => {
    e.evt.preventDefault();

    const scaleBy = 1.02;
    const stage = e.target.getStage();
    const oldScale = stage.scaleX();
    const mousePointTo = {
      x: stage.getPointerPosition().x / oldScale - stage.x() / oldScale,
      y: stage.getPointerPosition().y / oldScale - stage.y() / oldScale
    };

    const newScale = e.evt.deltaY < 0 ? oldScale * scaleBy : oldScale / scaleBy;

    setState({
      ...state,
      scale: newScale,
      x: (stage.getPointerPosition().x / newScale - mousePointTo.x) * newScale,
      y: (stage.getPointerPosition().y / newScale - mousePointTo.y) * newScale
    });
  };
  
  const onDragEndSquare = (e) => {
    let squares = [...stateSquare.squares];
    let index = squares.findIndex(el => el.id == e.target.attrs.id)
    squares[index].x = e.target.x()
    squares[index].y = e.target.y()
    setSquare({ squares })

    const device = {id: parseInt(e.target.attrs.id), x: e.target.x(), y: e.target.y()}
    axios.post('http://localhost:3000/api/devices/editDevice', device)
        .then((result) => {
            
        }, (error) => {
            console.log(error);
        }
    );
  }

  const onChangeVlad = (e) => {
    setVlad({ ...stateVlad, id: e.target.value })
  }

  const onClickAdd = (e) => {
    if (stateVlad.id == 0){
      alert("You must select an option.")
    }
    else if (stateSquare.squares.some(s=> s.id == parseInt(stateVlad.id))){
      alert("Vlad - " + stateVlad.id + " is already on the map.")
    }
    else{
      setSquare( prevSquare => ({
        squares: [...prevSquare.squares, { x: 100, y: 100, fill: "red", name: "Vlad - " + stateVlad.id, id: stateVlad.id }]
      }));

      const device = {id: parseInt(stateVlad.id)}
      axios.post('http://localhost:3000/api/devices/editDeviceProvisioned', device)
          .then((result) => {

          }, (error) => {
              console.log(error);
          }
      );
    }
  }

  const onClickDel = (e) => {
    if (stateVlad.id == 0){
      alert("You must select an option.")
    }
    else if (!stateSquare.squares.some(s=> s.id == parseInt(stateVlad.id))){
      alert("Vlad - " + stateVlad.id + " is not on the map.")
    }
    else{

      let squares = [...stateSquare.squares]
      let square = squares.find(square => square.id == stateVlad.id)
      squares.pop(square)
      setSquare({ squares })

      const del = {id: parseInt(stateVlad.id)}
      axios.post('http://localhost:3000/api/devices/delDevice', del)
          .then((result) => {

          }, (error) => {
              console.log(error);
          }
      );
    }
  }

  console.log("stateSquare.squares: ", stateSquare.squares)
  return (
      <Container>
        <Card>
              <Card.Header>Vlad Control</Card.Header>
              <Card.Body>
                  <blockquote className="blockquote mb-0">
                  <Form.Row className="d-flex">
                    <Form.Group as={Col} md="4">
                      <Form.Label>Vlad</Form.Label>
                      <Form.Control as="select" onChange={onChangeVlad}>
                        <option value={0}>Select an option...</option>
                        { state.devices?.map(device => 
                          <option value={device.id}>Vlad - {device.id}</option>
                         ) }
                      </Form.Control>
                      <br></br>
                      <Button variant="primary" type="button" onClick={onClickAdd}>
                        Add
                      </Button>
                      {' '}
                      <Button variant="danger" type="button" onClick={onClickDel}>
                        Del
                      </Button>
                    </Form.Group>
                  </Form.Row>
                  </blockquote>
              </Card.Body>
          </Card>
          <br></br>
          <Card>
              <Card.Header>Leaky Feeder network status</Card.Header>
              <Card.Body>
                  <blockquote className="blockquote mb-0">
                      <Stage width={1250} height={1000} onWheel={handleWheel} scaleX={state.scale} scaleY={state.scale} x={state.x} y={state.y} draggable>
                        <Layer>
                          <Image image={image}/>
                          { stateSquare.squares?.map((square) => 
                            {
                              return(  
                                <Group>
                                  <Text text={square.name} x={square.x + 20} y={square.y + 5}/>
                                  <Rect
                                    x={square.x}
                                    y={square.y}
                                    width={20}
                                    height={20}
                                    fill={square.fill}
                                    onDragEnd={onDragEndSquare}
                                    draggable
                                    id={square.id}
                                  />
                                </Group>)
                            }
                          ) }
                          
                        </Layer>
                      </Stage>
                  </blockquote>
              </Card.Body>
          </Card>
      </Container>
  )
}

export async function getServerSideProps() {
    const config = await axios
      .get("http://localhost:3000/api/manage/config")
      .then((res) => {
        return res.data
      })

    const devices = await axios
      .get("http://localhost:3000/api/devices/devices")
      .then((res) => {
        return res.data
      })
  
    return {
      props: { config, devices },
    }
}

export default Diagram