import React from "react"
import { Navbar, Nav } from "react-bootstrap"
import ImgUrl from "../../images/Url.png"

const Footer = ({ url }) => {
  return (
    <>
      <div className="container-fluid sigmaDarkBg custom-footer">
        <div className="row">
          <div className="col-md-12">
            <div className="row mb-3 mt-3">
              <div className="col-md-4 text-center">
                <label>Contact</label>
                <br />
                <a href="mailto:info@sigma-telecom.com">info@sigma-telecom.com</a>
              </div>
              <div className="col-md-4 text-center">
                <img src={ImgUrl.src} alt="" width="150px" />
              </div>
              <div className="col-md-4 text-center">
                <label>Find Us</label>
                <br />
                <label>Los Quillayes 446 - Concón, Chile</label>
                <br />
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
    </>
  )
}

export default Footer
