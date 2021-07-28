import { useEffect } from "react"
import { Navbar, Nav, NavDropdown } from "react-bootstrap"
import NextLink from "next/link"
// import LogoSigma from "../../images/logoSigma.png"

import { connect } from "react-redux"
import { setActiveComponent } from "../../redux/actions/main"
import { logout } from "../../redux/actions/auth/user"

const Header = (props) => {
  const { user, isLoggedIn, setActiveComponent, logout } = props

  useEffect(() => {
    if (isLoggedIn) setActiveComponent("alerts")
  }, [isLoggedIn])

  return (
    <Navbar
      collapseOnSelect
      expand="lg"
      className="navbar navbar-expand-lg  sticky-top   custom-navbar"
      variant="dark"
    >
      <a className="navbar-brand">
        <img
          src="/images/logoSigma.png"
          alt="Sigma Telecom"
          height="37px"
          width="160px"
          onClick={() => setActiveComponent("alerts")}
        />
      </a>
      <Navbar.Toggle aria-controls="responsive-navbar-nav" />

      <Navbar.Collapse
        id="responsive-navbar-nav"
        className="collapse navbar-collapse"
      >
        <Nav className="mr-auto">
          {isLoggedIn && (
            <>
              <a
                className="nav-link"
                id="mapMenuItem"
                onClick={() => setActiveComponent("map")}
              >
                Map
              </a>
              <a className="nav-link" onClick={() => setActiveComponent("alerts")}>
                Alerts
              </a>
              <a className="nav-link" onClick={() => setActiveComponent("rtdata")}>
                RT-Data
              </a>
            </>
          )}

          {user && (user.data?.rol === "admin" || user.data?.rol === "sadmin") && (
            <NavDropdown title="Manage" id="collasible-nav-dropdown">
              {user && user.data?.rol === "sadmin" && (
                <a
                  className="dropdown-item"
                  onClick={() => setActiveComponent("usersadmin")}
                >
                  Users
                </a>
              )}

              <>
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
              </>
            </NavDropdown>
          )}
        </Nav>
      </Navbar.Collapse>
      {isLoggedIn && (
        <Nav className="ml-auto">
          <a className="nav-link">
            {user.data?.name} ({user.data?.rol})
          </a>

          <a className="nav-link btn sigmaDarkBg" onClick={() => logout()}>
            Logout
          </a>
        </Nav>
      )}
      {!isLoggedIn && (
        <Nav>
          <a className="nav-link" onClick={() => setActiveComponent("login")}>
            Login
          </a>
        </Nav>
      )}
    </Navbar>
  )
}

const mapStateToProps = (state) => {
  return { user: state.user.user, isLoggedIn: state.user.isLoggedIn }
}
const mapDispatchToProps = {
  setActiveComponent,
  logout,
}

export default connect(mapStateToProps, mapDispatchToProps)(Header)
