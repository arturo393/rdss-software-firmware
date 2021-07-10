import React from 'react';
import { Navbar, Nav } from 'react-bootstrap'

const Footer = ({url}) => {
    return (
        <Navbar collapseOnSelect expand="lg" className="custom-footer" variant="dark" fixed="bottom">
                <div className="col-md-2 offset-2">
                    <div className="row justify-footer">
                        <label>Contac</label>
                    </div>
                    <div className="row justify-footer">
                        <Nav.Link href="mailto:info@sigma-telecom.com"><label>info@sigma-telecom.com</label></Nav.Link>
                    </div>
                </div>
                <Navbar.Brand href="/"><img src={url.src} alt="" width="100px"/></Navbar.Brand>
                <div className="col-md-2">
                    <div className="row justify-footer">
                        <label>Find Us</label>
                    </div>
                    <div className="row justify-footer">
                        <label>Los Quillayes 446 - Concón, Chile</label>
                    </div>
                </div>
                <iframe title="Google Maps" className="responsive-iframe" src="https://maps.google.com/maps?width=100%25&amp;height=600&amp;hl=es&amp;q=Los%20Romeros,%20Los%20Quillayes%20446,%20Conc%C3%B3n,%20Valpara%C3%ADso+(Sigma)&amp;t=&amp;z=17&amp;ie=UTF8&amp;iwloc=B&amp;output=embed" width="270" height="95" frameborder="0"></iframe>
            </Navbar>
    )
}

export default Footer