import React, { Component } from "react"
import { Navbar, Nav, NavDropdown } from "react-bootstrap"
import NextLink from "next/link"
import LogoSigma from "../../images/logoSigma.png"

import { connect } from "react-redux"
import { setActiveComponent } from "../../redux/actions/main"

const Header = (props) => {
  const { isLoggedIn, setActiveComponent } = props

  if (isLoggedIn) {
    return (
      <Navbar
        collapseOnSelect
        expand="lg"
        className="navbar navbar-expand-lg  sticky-top   custom-navbar"
        variant="dark"
      >
        <div className="container-fluid">
          <a className="navbar-brand">
            <img
              src={LogoSigma.src}
              alt="Sigma Telecom"
              height="37px"
              width="160px"
            />
          </a>
          <Navbar.Toggle aria-controls="responsive-navbar-nav" />

          <Navbar.Collapse
            id="responsive-navbar-nav"
            className="collapse navbar-collapse"
          >
            <Nav className="mr-auto">
              <a className="nav-link" onClick={() => setActiveComponent("alerts")}>
                Alerts
              </a>
              <a className="nav-link" onClick={() => setActiveComponent("rtdata")}>
                RT-Data
              </a>
              <NavDropdown title="Manage" id="collasible-nav-dropdown">
                <NextLink href="/manage/roles/roles">
                  <a className="dropdown-item">Roles</a>
                </NextLink>
                <NextLink href="/manage/users/users">
                  <a className="dropdown-item">Users</a>
                </NextLink>
                <a
                  className="dropdown-item"
                  onClick={() => setActiveComponent("diagramedit")}
                >
                  Diagram Edit
                </a>
                <a
                  className="dropdown-item"
                  onClick={() => setActiveComponent("networkParameters")}
                >
                  Network Parameters
                </a>
              </NavDropdown>
            </Nav>
          </Navbar.Collapse>

          <Nav>
            <a className="nav-link" onClick={() => setActiveComponent("login")}>
              Login
            </a>
          </Nav>
        </div>
      </Navbar>
    )
  } else {
    return (
      <Navbar
        collapseOnSelect
        expand="lg"
        className="navbar navbar-expand-lg  sticky-top   custom-navbar"
        variant="dark"
      >
        <div className="container-fluid">
          <a className="navbar-brand">
            <img
              src={LogoSigma.src}
              alt="Sigma Telecom"
              height="37px"
              width="160px"
            />
          </a>
          <Navbar.Toggle aria-controls="responsive-navbar-nav" />

          <Navbar.Collapse
            id="responsive-navbar-nav"
            className="collapse navbar-collapse"
          ></Navbar.Collapse>

          <Nav>
            <a className="nav-link" onClick={() => setActiveComponent("login")}>
              Login
            </a>
          </Nav>
        </div>
      </Navbar>
    )
  }
}

const mapStateToProps = (state) => {
  return { isLoggedIn: state.user.isLoggedIn }
}
const mapDispatchToProps = {
  setActiveComponent,
}

export default connect(mapStateToProps, mapDispatchToProps)(Header)
