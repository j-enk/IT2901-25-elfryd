import React from "react";
import {
  AppBar,
  Box,
  Toolbar,
  Typography,
  Button,
  Tooltip,
} from "@mui/material";
import WaterIcon from "@mui/icons-material/Water";
import SideBar from "./SideBar";
import { DirectionsBoat } from "@mui/icons-material";
import Link from "next/link";
import Weather from "../Weather/WeatherComponent";
import useUserHasActiveBooking from "../../hooks/bookings/useUserHasActiveBooking";
import useRenderDesktopView from "../../hooks/shouldRenderDesktopView";

interface NavBarProps {
  handleOpenShow: () => void;
  showWeather: boolean;
}

const NavBar: React.FC<NavBarProps> = ({ handleOpenShow, showWeather }) => {
  const { data: userHasActiveBooking } = useUserHasActiveBooking();
  const shouldRenderDesktop = useRenderDesktopView();

  return (
    <Box sx={{ flexGrow: 1 }} width="100vw">
      <AppBar position="static">
        <Toolbar sx={{ display: "flex", justifyContent: "space-between" }}>
          <Box
            sx={{
              display: "flex",
              alignItems: "center",
              justifyContent: "start",
              gap: "0px",
            }}
          >
            {!shouldRenderDesktop && (
              <Box width="10%">
                <SideBar />
              </Box>
            )}
            <Typography
              variant="h6"
              noWrap
              component="div"
              sx={{ display: { xs: "none", sm: "block" }, mr: "auto" }}
            >
              <Link href="/">Kystlaget Trondheim</Link>
            </Typography>
          </Box>

          <Box sx={{ display: "flex", alignItems: "center" }}>
            {showWeather && (
              <Tooltip title="Lenke til været i Trondheim" placement="top">
                <a
                  href="https://www.yr.no/nb/v%C3%A6rvarsel/daglig-tabell/1-211102/Norge/Tr%C3%B8ndelag/Trondheim/Trondheim"
                  target="_blank"
                >
                  <Weather />
                </a>
              </Tooltip>
            )}

            {showWeather && (
              <Box>
                <Tooltip title="Lenke til marinogram" placement="top">
                  <a
                    href="https://nais.kystverket.no/marinogram/10.379257113342504_63.43688823966815"
                    target="_blank"
                  >
                    <WaterIcon
                      fontSize="large"
                      cursor="pointer"
                      sx={{ m: 2 }}
                    />
                  </a>
                </Tooltip>
              </Box>
            )}

            {userHasActiveBooking && (
              <Button
                onClick={handleOpenShow}
                sx={{
                  color: "white",
                  "&:hover": {
                    bgcolor: "#334454",
                  }
                }}
                endIcon={<DirectionsBoat />}
              >
                LÅS OPP
              </Button>
            )}
          </Box>
        </Toolbar>
      </AppBar>
    </Box>
  );
};

export default NavBar;
