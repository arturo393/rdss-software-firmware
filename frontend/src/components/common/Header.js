import React, { Component } from 'react';
import { Navbar, Nav, NavDropdown } from 'react-bootstrap'
import NextLink from 'next/link';

class Header extends Component {

    render() {
        return (
            <Navbar collapseOnSelect expand="lg" className="custom-navbar" variant="dark">
                <div className="offset-2">
                    <NextLink href="/"><a className="navbar-brand"><img src={this.props.logo.src} alt="" height="45px" width="170px"/></a></NextLink>
                </div>
                <Navbar.Toggle aria-controls="responsive-navbar-nav" />
                <Navbar.Collapse id="responsive-navbar-nav">
                    <Nav className="mr-auto">
                        <NextLink href="/schema/schema"><a className="nav-link">Schema</a></NextLink>
                        <NextLink href="/rtdata/rtdata"><a className="nav-link">RT-Data</a></NextLink>
                        <NavDropdown title="Manage" id="collasible-nav-dropdown">
                            <NextLink href="/manage/roles/roles"><a className="dropdown-item">Roles</a></NextLink>
                            <NextLink href="/manage/users/users"><a className="dropdown-item">Users</a></NextLink>
                            <NextLink href="/manage/diagram"><a className="dropdown-item">Diagram Edit</a></NextLink>
                            <NextLink href="/manage/networkParameters"><a className="dropdown-item">Network Parameters</a></NextLink>
                        </NavDropdown>
                    </Nav>
                </Navbar.Collapse>
                <Nav className="mr-auto">
                    <NextLink href="/auth/login"><a className="nav-link">Login</a></NextLink>
                    <NextLink href="/auth/register"><a className="nav-link">Register</a></NextLink>
                </Nav>
            </Navbar>
        )
    }
}

export default Header