import React from 'react';
import { Button, Container, Card, Form } from 'react-bootstrap';
import Router from 'next/router'
import { setEmail } from '../../../redux/actions/user'
import { useEffect } from "react"
import { connect } from "react-redux"
import axios from "axios"
import User from './user';

const Users = (props) => {

    const { setEmail } = props

    useEffect(() => {
        setEmail(props.users)
    }, [])

    const handleHome = () => {
        Router.push('/')
    }

    const handleCreate = () => {
        Router.push('/auth/register')
    }

    return (
        <Container>
            <Card>
                <Card.Header>Users List</Card.Header>
                <Card.Body>
                    <blockquote className="blockquote mb-0">
                        { props.users.map(user => <User user={user} key={user._id}/> )}
                    </blockquote>
                </Card.Body>
                <Card.Footer>
                    <blockquote className="blockquote mb-0">
                        <Button variant="primary" type="button" onClick={handleCreate}>
                            Create New User
                        </Button>
                        {' '}
                        <Button variant="success" type="button" onClick={handleHome}>
                            Go Home
                        </Button>
                    </blockquote>
                </Card.Footer>
            </Card>
        </Container>
    )
}

const mapStateToProps = (state) => {
    return { 
      email: state.user.email,
    }
}

const mapDispatchToProps = {
    setEmail
}

export async function getServerSideProps() {
    const users = await axios
      .get("http://localhost:3000/api/manage/users")
      .then((res) => {
        return res.data
      })
  
    return {
      props: {
        users
      },
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(Users)