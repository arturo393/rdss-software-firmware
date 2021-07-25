import React, { Component } from "react"
import { Navbar, Nav, NavDropdown } from "react-bootstrap"
import NextLink from "next/link"
import LogoSigma from "../../images/logoSigma.png"

import { connect } from "react-redux"
import { setActiveComponent } from "../../redux/actions/main"

const Header = (props) => {
  const { setActiveComponent } = props

  return (
    <>
      <Navbar
        collapseOnSelect
        expand="lg"
        className="custom-navbar"
        variant="dark"
        // fixed="top"
      >
        <div className="offset-0">
          <a className="navbar-brand">
            <img
              src={LogoSigma.src}
              alt="Sigma Telecom"
              height="37px"
              width="160px"
            />
          </a>
        </div>
        <Navbar.Toggle aria-controls="responsive-navbar-nav" />
        <Navbar.Collapse id="responsive-navbar-nav">
          <Nav className="mr-auto">
            <a className="nav-link" onClick={() => setActiveComponent("alerts")}>
              Alerts
            </a>
            <a className="nav-link" onClick={() => setActiveComponent("rtdata")}>
              RT-Data
            </a>
            <NavDropdown title="Manage" id="collasible-nav-dropdown">
              <NextLink href="/manage/roles/roles">
                <a className="dropdown-item" >Roles</a>
              </NextLink>
              <NextLink href="/manage/users/users">
                <a className="dropdown-item">Users</a>
              </NextLink>
              <NextLink href="/manage/diagram">
                <a className="dropdown-item">Diagram Edit</a>
              </NextLink>
                <a className="dropdown-item" onClick={() => setActiveComponent("networkParameters")}>Network Parameters</a>
            </NavDropdown>
          </Nav>
        </Navbar.Collapse>
        <Nav className="mr-auto">
          <a className="nav-link" onClick={() => setActiveComponent("login")}>
            Login
          </a>
          <a className="nav-link" onClick={() => setActiveComponent("register")}>
            Register
          </a>
        </Nav>
      </Navbar>
    </>
  )
}

const mapStateToProps = (state) => {
  return {}
}
const mapDispatchToProps = {
  setActiveComponent,
}

export default connect(mapStateToProps, mapDispatchToProps)(Header)
