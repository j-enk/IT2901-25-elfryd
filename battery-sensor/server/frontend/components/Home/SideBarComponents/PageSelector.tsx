import { Box, Typography } from "@mui/material";
import NavMenuLink from "./NavMenuLink";
import AdminPanelSettingsIcon from "@mui/icons-material/AdminPanelSettings";
import CalendarMonthIcon from "@mui/icons-material/CalendarMonth";
import EventAvailableIcon from '@mui/icons-material/EventAvailable';
import { useSelector } from "react-redux";
import { selectActiveUser } from "../../../features/user/userSelector";
import KeyIcon from "@mui/icons-material/Key";
import PeopleIcon from "@mui/icons-material/People";
import ScheduleIcon from "@mui/icons-material/Schedule";
import DirectionsBoatIcon from "@mui/icons-material/DirectionsBoat";
import SensorsIcon from "@mui/icons-material/Sensors";
import LockOpenIcon from "@mui/icons-material/LockOpen";

const SubMenu = () => {
  return (
    <Box width="90%" ml="auto">
      <NavMenuLink
        isSubmenu
        href="/admin/status"
        text="Status"
        icon={<SensorsIcon />}
      />
      <NavMenuLink
        isSubmenu
        href="/admin/open"
        text="Åpne skap"
        icon={<LockOpenIcon />}
      />
      <NavMenuLink
        isSubmenu
        href="/admin/users"
        text="Brukere"
        icon={<PeopleIcon />}
      />
      <NavMenuLink
        isSubmenu
        href="/admin/keys"
        text="Nøkler"
        icon={<KeyIcon />}
      />
      <NavMenuLink
        isSubmenu
        href="/admin/bookingPeriods"
        text="Bookingperioder"
        icon={<ScheduleIcon />}
      />
      <NavMenuLink
        isSubmenu
        href="/admin/boats"
        text="Båter"
        icon={<DirectionsBoatIcon />}
      />
    </Box>
  );
};

const PageSelector = () => {
  const profile = useSelector(selectActiveUser);
  const isAdmin = profile.roles.includes("Admin");

  return (
    <Box p={1}>
      <Typography variant="h6" className="mb-4">
        Naviger
      </Typography>

      <Box display="flex" flexDirection="column" gap="2px">
        <NavMenuLink
          href="/booking"
          text="Booking"
          icon={<CalendarMonthIcon />}
        />
        <NavMenuLink
          href="/myBookings"
          text="Mine bookinger"
          icon={<EventAvailableIcon />}
        />
        {isAdmin && (
          <NavMenuLink
            href="/admin/status"
            text="Admin"
            icon={<AdminPanelSettingsIcon />}
            hasSubmenu
            submenu={<SubMenu />}
          />
        )}
      </Box>
    </Box>
  );
};

export default PageSelector;
