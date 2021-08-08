import App, { Container } from "next/app"
import { Provider } from "react-redux"
import { wrapper } from "../redux/store"

import Header from "../components/common/Header"
import Footer from "../components/common/Footer"

import "bootstrap/dist/css/bootstrap.css"
import "../styles/global.css"
import "../styles/common/Header.css"
import "../styles/common/Footer.css"

class MyApp extends App {
  render() {
    const { Component, pageProps, store } = this.props

    return (
      <>
        <Header />
        <Component {...pageProps} />
        <Footer />
      </>
    )
  }
}

export default wrapper.withRedux(MyApp)
