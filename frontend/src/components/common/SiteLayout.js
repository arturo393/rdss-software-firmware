import React, { useState, useEffect } from 'react';
import Schema from "../../pages/schema/schema"
import Alerts from '../schema/Alerts'
import Status from '../common/Status'
import axios from "axios"
import Register from '../../pages/auth/register';
import Login from '../../pages/auth/login'
import Roles from '../../pages/manage/roles/roles'
import NetworkParameters from '../../pages/manage/networkParameters';

const SiteLayout = (props, { children }) => {
    const loc = window.location.pathname;
    if (loc == '/auth/register'){
        return(
            <div className="container-fluid">
                <div className="row">
                    <div className="col-md-6">
                        <Schema config={props.config} devices={props.devices}/>
                    </div>
                    <div className="col-md-6">
                        <div className="row">
                            <div className="col-md-12">
                                <Status />
                            </div>
                        </div>
                        <div className="row">
                            <div className="col-md-12">
                                <Register />
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        )
    }
    if (loc == '/auth/login'){
        return(
            <div className="container-fluid">
                <div className="row">
                    <div className="col-md-6">
                        <Schema config={props.config} devices={props.devices}/>
                    </div>
                    <div className="col-md-6">
                        <div className="row">
                            <div className="col-md-12">
                                <Status />
                            </div>
                        </div>
                        <div className="row">
                            <div className="col-md-12">
                                <Login />
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        )
    }
    if (loc == '/manage/roles/roles'){
        return(
            <div className="container-fluid">
                <div className="row">
                    <div className="col-md-6">
                        <Schema config={props.config} devices={props.devices}/>
                    </div>
                    <div className="col-md-6">
                        <div className="row">
                            <div className="col-md-12">
                                <Status />
                            </div>
                        </div>
                        <div className="row">
                            <div className="col-md-12">
                                <Register />
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        )
    }
    if (loc == '/manage/networkParameters'){
        return(
            <div className="container-fluid">
                <div className="row">
                    <div className="col-md-6">
                        <Schema config={props.config} devices={props.devices}/>
                    </div>
                    <div className="col-md-6">
                        <div className="row">
                            <div className="col-md-12">
                                <Status />
                            </div>
                        </div>
                        <div className="row">
                            <div className="col-md-12">
                                <NetworkParameters />
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        )
    }
    
    return(
        <div className="container-fluid">
            <div className="row">
                <div className="col-md-6">
                    <Schema config={props.config} devices={props.devices}/>
                </div>
                <div className="col-md-6">
                    <div className="row">
                        <div className="col-md-12">
                            <Status />
                        </div>
                    </div>
                    <div className="row">
                        <div className="col-md-12">
                            { children }
                        </div>
                    </div>
                </div>
            </div>
        </div>
    )
}

export default SiteLayout;
