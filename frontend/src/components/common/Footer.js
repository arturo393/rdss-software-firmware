import React from "react"
import { Navbar, NavDropdown, Nav } from "react-bootstrap"
import ImgUrl from "../../images/Url.png"
import NextLink from "next/link"

const Footer = ({ url }) => {
  return (
    <Navbar
      collapseOnSelect
      expand="lg"
      className="navbar navbar-expand-lg  fixed-bottom  sigmaDarkBg custom-footer"
      align="center"
      variant="dark"
    >
      <Navbar.Toggle aria-controls="responsive-navbar-nav" />
      <Navbar.Collapse
        id="responsive-navbar-nav"
        className="collapse navbar-collapse"
      >
        <div className="container-fluid sigmaDarkBg float-sm-star">
          <div className="row">
            <div className="col-md-12">
              <div className="row mb-3 mt-3">
                <div className="col-md-3 text-center">
                  <label>Contact</label>
                  <br />
                  <a href="mailto:info@sigma-telecom.com">info@sigma-telecom.com</a>
                </div>
                <div className="col-md-3 text-center">
                  <img src="/images/Url.png" alt="" width="100px" />
                </div>
                <div className="col-md-3 text-center">
                  <label>Find Us</label>
                  <br />
                  <label>Los Quillayes 446 - Concón, Chile</label>
                  <br />
                </div>
                <div className="col-md-3 text-center">
                  <iframe
                    title="Google Maps"
                    className="responsive-iframe"
                    src="https://maps.google.com/maps?width=100%25&amp;height=600&amp;hl=es&amp;q=Los%20Romeros,%20Los%20Quillayes%20446,%20Conc%C3%B3n,%20Valpara%C3%ADso+(Sigma)&amp;t=&amp;z=17&amp;ie=UTF8&amp;iwloc=B&amp;output=embed"
                    width="270"
                    height="95"
                    frameborder="0"
                  ></iframe>
                </div>
              </div>
            </div>
          </div>
        </div>
        <Nav className="mr-auto"></Nav>
      </Navbar.Collapse>
    </Navbar>
  )
}

export default Footer
