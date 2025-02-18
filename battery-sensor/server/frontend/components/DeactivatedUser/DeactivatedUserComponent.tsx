import React, {useEffect} from "react";
import {Container} from "@mui/material";
import Head from "next/head";
import {useCookies} from "react-cookie";
import {useAppDispatch} from "../../features/hooks";
import Image from "next/image";
import {Dispatch} from "@reduxjs/toolkit";
import {logIn} from "../../features/auth/authSlice";
import NavBar from "../Home/NavBar";

const actionDispatch = (dispatch: Dispatch<any>) => ({
	logIn: (query: any) => dispatch(logIn(query)),
});

const LoginComponent = () => {
	const {logIn} = actionDispatch(useAppDispatch());
	const [cookies, _] = useCookies();
	
	useEffect(() => {
		if (cookies["token"]) logIn(cookies["token"]);
	}, [cookies]);

	return (
		<>
		<NavBar handleOpenShow={() => null} showWeather={false}/>
			<Head>
				<title>Deaktivert bruker - Kystlaget</title>
				<meta
					name="description"
					content="Page for booking a trip."
				/>
			</Head>
			<Container className="flex flex-col m-2 items-center justify-center ml-auto mr-auto">
				<Image
					alt="Kystlaget Trondheim logo"
					src="/kystlaget_trondheim_logo.jpg"
					width={200}
					height={300}
				/>
				 <p>Brukeren din er deaktivert. Kontakt oss på elfryd@kystlaget-trh.no dersom du mener dette er feil.</p>
			</Container>
		</>
	);
};

export default LoginComponent;
