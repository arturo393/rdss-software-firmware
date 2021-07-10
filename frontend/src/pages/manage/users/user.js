import React, { useState } from 'react'
import { Button, Card } from 'react-bootstrap';
import NextLink from 'next/link';
import {alert, confirm} from 'react-bootstrap-confirmation';
import axios from "axios"


const User = ({ user }) => {
    const [ state, setState ] = useState({
        userState: user
    });
    console.log(state.userState)
    const handleDelete = async () => {
        const result = await confirm('Are you sure you want to delete?');
        if (result){
            const idUser = { id: state.userState._id };
            delete state.userState
            setState(state)

            console.log(state.userState)
            axios.post('http://localhost:3000/api/manage/deleteuser', idUser)
                .then((result) => {
                    alert('User has been deleted');
                }, (error) => {
                    console.log(error);
                }
            );
        }
    };
    
    return (
        <div>
            <Card>
                <Card.Header>{ user.name }</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        <NextLink href={`/manage/users/edit/${user._id}`}><Button variant="primary" type="button">Edit</Button></NextLink>
                        {' '}
                        <Button variant="danger" type="button" onClick={handleDelete}>
                            Delete
                        </Button>
                    </blockquote>
                </Card.Body>
            </Card>
            <br></br>
        </div>
    )
  }
  
export default User;