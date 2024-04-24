import React from "react"
import { Navbar, NavDropdown, Nav } from "react-bootstrap"
// import ImgUrl from "../../images/Url.png"
import NextLink from "next/link"

const Contact = ({ url }) => {
  return (
    <>
      <div className="row text-center position-absolute top-30 start-50 translate-middle-x">
        <label>
          <b>Email</b>
        </label>
        <a href="mailto:info@sigma-telecom.com">info@sigma-telecom.com</a>
        <img src="/images/Url.png" alt="Sigma Telecom" style={{ width: 400, margin: "auto" }} />
        <label>
          <b>Find Us</b>
        </label>
        <span>Los Quillayes 446 - Concón, Chile</span>

        <iframe
          title="Google Maps"
          className="responsive-iframe "
          src="https://maps.google.com/maps?width=100%25&amp;height=600&amp;hl=es&amp;q=Los%20Romeros,%20Los%20Quillayes%20446,%20Conc%C3%B3n,%20Valpara%C3%ADso+(Sigma)&amp;t=&amp;z=17&amp;ie=UTF8&amp;iwloc=B&amp;output=embed"
          width="270"
          height="400"
          frameBorder="0"
        ></iframe>
      </div>
    </>
  )
}

export default Contact
